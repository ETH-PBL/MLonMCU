/*
 * network.h
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stddef.h>
#include "pmsis.h"


struct network_run_token {
  struct pi_device cluster_dev;
};


void network_terminate();
void network_initialize();
void network_run_cluster(void * args);
struct network_run_token network_run_async(void *l2_buffer, size_t l2_buffer_size, void *l2_final_output, void **l3_buffer, int exec, int initial_dir);
void network_run_wait(struct network_run_token token);
void network_run(void *l2_buffer, size_t l2_buffer_size, void *l2_final_output, void **l3_buffer, int exec, int initial_dir);
void execute_layer_fork(void *arg);


#ifdef DEFINE_CONSTANTS
// allocation of buffers with parameters needed by the network execution
static const char * L3_weights_files[] = {
  "BNReluConvolution0_weights.hex", "BNReluConvolution1_weights.hex", "BNReluConvolution2_weights.hex", "BNReluConvolution3_weights.hex", "BNReluConvolution4_weights.hex", "BNReluConvolution5_weights.hex", "BNReluConvolution6_weights.hex", "BNReluConvolution7_weights.hex", "BNReluConvolution8_weights.hex", "FullyConnected10_weights.hex"
};
static int L3_weights_size[10];
static int layers_pointers[11];
static char * Layers_name[11] = {"BNReluConvolution0", "BNReluConvolution1", "BNReluConvolution2", "BNReluConvolution3", "BNReluConvolution4", "BNReluConvolution5", "BNReluConvolution6", "BNReluConvolution7", "BNReluConvolution8", "Pooling9", "FullyConnected10"};
static int L3_input_layers[11] = {1,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int L3_output_layers[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int allocate_layer[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
static int branch_input[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int branch_output[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int branch_change[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int weights_checksum[11] = {112647, 114277, 559949, 121682, 565129, 112070, 569185, 111784, 579197, 0, 105403};
static int weights_size[11] = {1088, 1088, 4608, 1088, 4608, 1088, 4608, 1088, 4608, 0, 816};
static int activations_checksum[11][1] = {{
  59535  },
{
  331087  },
{
  221781  },
{
  226505  },
{
  153618  },
{
  243098  },
{
  169633  },
{
  142342  },
{
  37319  },
{
  94198  },
{
  914  }
};
static int activations_size[11] = {490, 7680, 7360, 7360, 7040, 7040, 6720, 6720, 6400, 6400, 64};
static int out_mult_vector[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static int out_shift_vector[11] = {19, 19, 19, 19, 19, 19, 19, 19, 19, 0, 0};
static int activations_out_checksum[11][1] = {{
  331087 },
{
  221781 },
{
  226505 },
{
  153618 },
{
  243098 },
{
  169633 },
{
  142342 },
{
  37319 },
{
  94198 },
{
  914 },
{
  8421 }
};
static int activations_out_size[11] = {7680, 7360, 7360, 7040, 7040, 6720, 6720, 6400, 6400, 64, 48};
static int layer_with_weights[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
#endif

#endif  // __NETWORK_H__
