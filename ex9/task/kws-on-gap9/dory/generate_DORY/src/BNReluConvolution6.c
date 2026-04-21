/*
 * layer_template.c
 * Alessio Burrello <alessio.burrello@unibo.it>
 * Francesco Conti <f.conti@unibo.it>
 *
 * Copyright (C) 2018-2020 University of Bologna
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

#include "BNReluConvolution6.h"
#include "pmsis.h"
#include "dory_get_tile.h"
#include "dory_dma.h"
#include "pulp_nn_kernels.h"
#include "tile_index.h"
#include "layer.h"
#include "double_buffer.h"
#include "net_utils.h"



static const TileIndex index_end = {
  .height = 1,
  .width = 1,
  .input_channel = 1,
  .output_channel = 1,
};

static const Layer body = {
  .input = {
    .height = 21,
    .width = 5,
    .channel = 64,
    .channel_size = 64
  },
  .output = {
    .height = 21,
    .width = 5,
    .channel = 64,
    .channel_size = 64
  },
  .weights = {
    .output_channel = 64,
    .input_channel = 64,
    .input_channel_size = 64
  }
};

static const Layer border = {
  .input = {
    .height = 21,
    .width = 5,
    .channel = 64,
    .channel_size = 64
  },
  .output = {
    .height = 21,
    .width = 5,
    .channel = 64,
    .channel_size = 64
  },
  .weights = {
    .output_channel = 64,
    .input_channel = 64,
    .input_channel_size = 64
  }
};


static void load_input_async(Layer tile, Layer body, Layer layer, TileIndex index) {
  // additionally overlap by padding for the first tile after a border one
  // this because in the first tile we use less pixels from x_buffer, since we have the ones of padding
  const int x_offset_h = index.height > 0 ? layer.padding.top : 0;
  const int x_offset_w = index.width > 0 ? layer.padding.left : 0;

  dma_transfer_async((DmaTransferConf) {
    .ext = dory_get_tile_3d(layer.addr.input,
                            index.height, index.width, index.input_channel,
                            body.input.height, body.input.width, body.input.channel,
                            layer.input.width, layer.input.channel,
                            0, 0, 0,
                            x_offset_h, x_offset_w, 0,
                            8),
    .loc = tile.addr.input,
    .number_of_2d_copies = tile.input.height,
    .number_of_1d_copies = tile.input.width,
    .length_1d_copy = tile.input.channel_size,
    .hwc_to_chw = 0,
    .stride_2d = 320,
    .stride_1d = 64,
    .dir = 1
  });
}

static void store_output_async(Layer tile, Layer body, Layer layer, TileIndex index) {
  dma_transfer_async((DmaTransferConf) {
    .ext = dory_get_tile_3d(layer.addr.output,
                            index.height, index.width, index.output_channel,
                            body.output.height, body.output.width, body.output.channel,
                            layer.output.width, layer.output.channel,
                            0, 0, 0,
                            0, 0, 0,
                            8),
    .loc = tile.addr.output,
    .number_of_2d_copies = tile.output.height,
    .number_of_1d_copies = tile.output.width,
    .length_1d_copy = tile.output.channel_size,
    .hwc_to_chw = 0,
    .stride_2d = 320,
    .stride_1d = 64,
    .dir = 0
  }); 
}

static void load_weights_async(Layer tile, Layer body, Layer layer, TileIndex index) {
  dma_transfer_async((DmaTransferConf) {
    .ext = dory_get_tile_3d(layer.addr.weights,
                            index.output_channel, 0, index.input_channel,
                            body.weights.output_channel, 1*1, body.weights.input_channel,
                            1*1, 64,
                            0, 0, 0,
                            0, 0, 0,
                            8),
    .loc = tile.addr.weights,
    .number_of_2d_copies = tile.weights.output_channel,
    .number_of_1d_copies = 1,
    .length_1d_copy = tile.weights.input_channel_size,
    .hwc_to_chw = 0,
    .stride_2d = 64,
    .stride_1d = 64,
    .dir = 1
  });

  dma_transfer_1d_async((DmaTransferConf) {
    .ext = layer.addr.scale + 256*index.output_channel,
    .loc = tile.addr.scale,
    .length_1d_copy = tile.weights.output_channel * 4,
    .dir = 1
  });

  dma_transfer_1d_async((DmaTransferConf) {
    .ext = layer.addr.bias + 256*index.output_channel,
    .loc = tile.addr.bias,
    .length_1d_copy = tile.weights.output_channel * 4,
    .dir = 1
  });
}

static void load_bias_async(Layer tile, Layer layer) {
  dma_transfer_1d_async((DmaTransferConf) {
    .ext = layer.addr.bias,
    .loc = tile.addr.bias,
    .length_1d_copy = 0,
    .dir = 1
  });
}

static void kernel(Layer tile, void * im2col, void * pwt_buffer) {
    pulp_nn_pointwise_HoWo_parallel(
      tile.addr.input,
      im2col,
      NULL,
      tile.addr.output,
      tile.addr.weights,
      tile.addr.scale, tile.addr.bias,
      1, 19,
      tile.input.width, tile.input.height, tile.input.channel,
      tile.output.width, tile.output.height, tile.output.channel,
      1, 1,
      tile.padding.top, tile.padding.bottom, tile.padding.left, tile.padding.right, 1, 1,
      1, 1
      );
}

typedef struct ConvolutionArgs {
  Layer tile;
  void * im2col;
  void * pwt_buffer;
} ConvolutionArgs;

static void convolution(void * args) {
  ConvolutionArgs * convolutionArgs = (ConvolutionArgs *)args;
  kernel(convolutionArgs->tile, convolutionArgs->im2col, convolutionArgs->pwt_buffer);
}


void __attribute__ ((noinline)) BNReluConvolution6(void *args) {
  //////////////////////////////////////////////////////////////////////////
  // arguments assigning: keeping same interface between L2 and L3 memory //
  //////////////////////////////////////////////////////////////////////////
  layer_args_t *layer_args = (layer_args_t *)args;
  unsigned int l1_buffer = layer_args->L1_buffer;

  const Layer layer = {
    .addr = {
      .input = layer_args->L2_input,
      .weights = layer_args->L2_weights,
      .scale = layer_args->L2_weights + 4096,
      .bias = layer_args->L2_weights + 4352,
      .output = layer_args->L2_output
    },
    .input = {
      .width = 5,
      .channel = 64
    },
    .output = {
      .width = 5,
      .channel = 64
    },
    .padding = {
        .top    = 0,
        .right  = 0,
        .bottom = 0,
        .left   = 0
    }
  };

  DoubleBuffer db_input = {
    .addrs = { l1_buffer + 0, l1_buffer + 0 + 6720 },
    .index = 0
  };
  DoubleBuffer db_output = {
    .addrs = { l1_buffer + 6728, l1_buffer + 6728 + 6720 },
    .index = 0
  };
  DoubleBuffer db_weights = {
    .addrs = { l1_buffer + 13456, l1_buffer + 13456 + 4608 },
    .index = 0
  };

  pi_team_config_offload(NUM_CORES);

  DmaTransfer transfer = dma_transfer_create();

  Layer tile_prev;
  TileIndex index_prev = { .height = 0, .width = 0, .input_channel = 0, .output_channel = 0 };
  TileIndex index = { .height = 0, .width = 0, .input_channel = 0, .output_channel = 0 };

  int is_input_load = 1, is_weights_load = 1;

  void * im2col = l1_buffer + 18088;
  void * pwt_buffer = NULL;

  const int total_tiles = index_end.output_channel * index_end.input_channel * index_end.height * index_end.width;

  // tile loop nest
  for(int iter=0; iter < total_tiles; iter++) {
    Address addr = {
      .input = double_buffer_get_addr(db_input),
      .weights = double_buffer_get_addr(db_weights),
      .scale = double_buffer_get_addr(db_weights) + 4096,
      .bias = double_buffer_get_addr(db_weights) + 4096 + 256,
      .output = double_buffer_get_addr(db_output)
    };
    Layer tile = tile_create(index, index_end, body, border, layer, addr);


    if (is_input_load) {
      load_input_async(tile, body, layer, index);
    }
    if (is_weights_load) {
      load_weights_async(tile, body, layer, index);
    }

    ConvolutionArgs convolutionArgs = {
      .tile = tile,
      .im2col = im2col,
      .pwt_buffer = pwt_buffer
    };

    dma_transfer_wait(transfer);

    if (iter > 0) {
      pi_team_offload_wait();
    }

    pi_team_offload_preset(convolution, &convolutionArgs);

    if (iter > 0) {
      store_output_async(tile_prev, body, layer, index_prev);
    }

    tile_prev = tile;
    index_prev = index;
    index = tile_index_get_next(index, index_end);

    is_input_load = index.input_channel!=index_prev.input_channel || index.width!=index_prev.width || index.height!=index_prev.height;
    is_weights_load = index.input_channel!=index_prev.input_channel || index.output_channel!=index_prev.output_channel;

    if (is_input_load) {
      double_buffer_increment(&db_input);
    }
    if (is_weights_load) {
      double_buffer_increment(&db_weights);
    }
    double_buffer_increment(&db_output);
  }

  pi_team_offload_wait();
  store_output_async(tile_prev, body, layer, index_prev);
  dma_transfer_wait(transfer);
}
