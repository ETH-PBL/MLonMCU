/*
 * pooling_layer_template.c
 * Alessio Burrello <alessio.burrello@unibo.it>
 *
 * Copyright (C) 2019-2020 University of Bologna
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

#include "Pooling9.h"
#include "pmsis.h"
#include "dory_get_tile.h"
#include "dory_dma.h"
#include "pulp_nn_kernels.h"
#include "tile_index.h"
#include "layer.h"
#include "net_utils.h"



static const TileIndex index_end = {
  .height = 1,
  .width = 1,
  .input_channel = 1,
  .output_channel = 1
};

static const Layer body = {
  .input = {
    .height = 20,
    .width = 5,
    .channel = 64,
    .channel_size = 64
  },
  .output = {
    .height = 1,
    .width = 1,
    .channel = 64,
    .channel_size = 64
  }
};

static const Layer border = {
  .input = {
    .height = 20,
    .width = 5,
    .channel = 64,
    .channel_size = 64
  },
  .output = {
    .height = 1,
    .width = 1,
    .channel = 64,
    .channel_size = 64
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
                            19, 4, 0,
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
    .stride_2d = 64,
    .stride_1d = 64,
    .dir = 0
  }); 
}


static void kernel(Layer tile) {
  pulp_nn_avgpool(
    tile.addr.input, tile.addr.output,
    1,
    0,
    0,
    tile.input.width,
    tile.input.height,
    tile.input.channel,
    tile.output.width,
    tile.output.height,
    5,
    20,
    tile.padding.top,
    tile.padding.bottom,
    tile.padding.left,
    tile.padding.right,
    1,
    1,
    0
  );
}

typedef struct PoolingArgs {
  Layer tile;
} PoolingArgs;

static void pooling(void * args) {
  PoolingArgs * poolingArgs = (PoolingArgs *)args;
  kernel(poolingArgs->tile);
}


void __attribute__ ((noinline)) Pooling9(
  void *args
) {
  layer_args_t *layer_args = (layer_args_t *)args;
  unsigned int l1_buffer = layer_args->L1_buffer;

  Layer layer = {
    .addr = {
      .input = layer_args->L2_input,
      .output = layer_args->L2_output
    },
    .input = {
      .width = 5,
      .channel = 64
    },
    .output = {
      .width = 1,
      .channel = 64
    },
    .padding = {
        .top    = 0,
        .right  = 0,
        .bottom = 0,
        .left   = 0
    }
  };

  pi_team_config_offload(NUM_CORES);

  DmaTransfer transfer = dma_transfer_create();

  TileIndex index = { .height = 0, .width = 0, .input_channel = 0, .output_channel = 0 };

  const int total_tiles = index_end.output_channel * index_end.height * index_end.width * index_end.input_channel;

  // tile loop nest
  for(int iter=0; iter<total_tiles; iter++) {
    Address addr = {
      .input = l1_buffer + 0,
      .output = l1_buffer + 6408
    };
    Layer tile = tile_create(index, index_end, body, border, layer, addr);

    load_input_async(tile, body, layer, index);

    dma_transfer_wait(transfer);
    PoolingArgs poolingArgs = { .tile = tile };
    pi_team_offload_preset(pooling, &poolingArgs);
    pi_team_offload_wait();

    store_output_async(tile, body, layer, index);

    index = tile_index_get_next(index, index_end);
  }
}
