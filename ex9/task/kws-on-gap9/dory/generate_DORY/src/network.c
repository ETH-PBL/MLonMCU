/*
 * network.c
 * Alessio Burrello <alessio.burrello@unibo.it>
 * Thorir Mar Ingolfsson <thoriri@iis.ee.ethz.ch>
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
#define DEFINE_CONSTANTS
#include "net_utils.h"
#include "pmsis.h"
#include "network.h"
#include "directional_allocator.h"
#include "mem.h"
#include <string.h>
#include "BNReluConvolution6.h"
#include "BNReluConvolution7.h"
#include "Pooling9.h"
#include "BNReluConvolution0.h"
#include "BNReluConvolution4.h"
#include "BNReluConvolution3.h"
#include "FullyConnected10.h"
#include "BNReluConvolution8.h"
#include "BNReluConvolution5.h"
#include "BNReluConvolution1.h"
#include "BNReluConvolution2.h"



#define L3_WEIGHTS_SIZE 4000000
#define L3_INPUT_SIZE 1500000
#define L3_OUTPUT_SIZE 1500000
static void *L3_weights = NULL;
static void *L3_input = NULL;
static void *L3_output = NULL;
int cycle_network_execution;
/* Moves the weights and the biases from hyperflash to hyperram */
void network_initialize() {

  L3_weights = ram_malloc(L3_WEIGHTS_SIZE);
  L3_input = ram_malloc(L3_INPUT_SIZE);
  L3_output = ram_malloc(L3_OUTPUT_SIZE);

#ifdef VERBOSE
  printf("\nL3 Buffer alloc initial\t@ %d:\t%s\n", (unsigned int)L3_weights, L3_weights?"Ok":"Failed");
  printf("\nL3 Buffer alloc initial\t@ %d:\t%s\n", (unsigned int)L3_input, L3_input?"Ok":"Failed");
  printf("\nL3 Buffer alloc initial\t@ %d:\t%s\n", (unsigned int)L3_output, L3_output?"Ok":"Failed");
#endif

  void *w_ptr = L3_weights;
  for (int i = 0; i < 10; i++) {
    size_t size = load_file_to_ram(w_ptr, L3_weights_files[i]);
    L3_weights_size[i] = size;
    w_ptr += size;
  }
}

/* Remove RAM memory */
void network_terminate() {
  ram_free(L3_weights, L3_WEIGHTS_SIZE);
  ram_free(L3_input, L3_INPUT_SIZE);
  ram_free(L3_output, L3_OUTPUT_SIZE);
}

void execute_layer_fork(void *args) {
  layer_args_t *layer_args = (layer_args_t *)args;
#ifdef TARGET_CHIP_FAMILY_GAP9
  layer_args->L1_buffer = pi_cl_l1_malloc(NULL, 92700);
#else
  layer_args->L1_buffer = pmsis_l1_malloc(92700);
#endif
  // PATCH: L1-only DSCNNL in Trainlib
  // layer_args->L1_buffer = 0x100072B0;


  if (NULL == layer_args->L1_buffer) {
#ifdef VERBOSE
    printf("ERROR: Failed to allocate the L1 buffer.\n");
#endif // VERBOSE
    return;
  }

  switch (layer_args->layer_id)
  {
    case 0:
      BNReluConvolution0(args);
      break;
    case 1:
      BNReluConvolution1(args);
      break;
    case 2:
      BNReluConvolution2(args);
      break;
    case 3:
      BNReluConvolution3(args);
      break;
    case 4:
      BNReluConvolution4(args);
      break;
    case 5:
      BNReluConvolution5(args);
      break;
    case 6:
      BNReluConvolution6(args);
      break;
    case 7:
      BNReluConvolution7(args);
      break;
    case 8:
      BNReluConvolution8(args);
      break;
    case 9:
      Pooling9(args);
      break;
    case 10:
      FullyConnected10(args);
      break;
  }

#ifdef TARGET_CHIP_FAMILY_GAP9
  pi_cl_l1_free(NULL, layer_args->L1_buffer, 92700);
#else
  pmsis_l1_malloc_free(layer_args->L1_buffer, 92700);
#endif
}

struct network_run_token network_run_async(void *l2_buffer, size_t l2_buffer_size, void *l2_final_output, void **l3_buffer, int exec, int initial_dir)
{
  struct pi_device cluster_dev = {0};
  struct pi_cluster_conf conf;
  struct pi_cluster_task cluster_task = {0};
  // First open the cluster
  pi_cluster_conf_init(&conf);
  conf.id=0;
#ifdef TARGET_CHIP_FAMILY_GAP9
  conf.icache_conf = PI_CLUSTER_MASTER_CORE_ICACHE_ENABLE | PI_CLUSTER_ICACHE_PREFETCH_ENABLE | PI_CLUSTER_ICACHE_ENABLE;
#endif
  unsigned int args[4];
  args[0] = (unsigned int) l2_buffer;
  args[1] = (unsigned int) l2_buffer_size;
  args[2] = (unsigned int) l2_final_output;
  args[3] = (unsigned int) l3_buffer;
  args[4] = (unsigned int) exec;
  // open cluster...
  pi_cluster_task(&cluster_task, network_run_cluster, args);
  pi_open_from_conf(&cluster_dev, &conf);
  if (pi_cluster_open(&cluster_dev))
    return;
  // Then offload an entry point, this will get executed on the cluster controller
#ifndef TARGET_CHIP_FAMILY_GAP9
  cluster_task.stack_size = 3500;
#endif
  cluster_task.slave_stack_size = 3400;
  pi_cluster_send_task_to_cl(&cluster_dev, &cluster_task);
  return (struct network_run_token) {
    .cluster_dev = cluster_dev
  };

  *l3_buffer = L3_weights;
  for (int i = 0; i < 11; i++){
    *l3_buffer += L3_weights_size[i]; 
  }
}

void network_run_wait(struct network_run_token token)
{
  pi_cluster_close(&token.cluster_dev);
  print_perf("Final", cycle_network_execution, 2078848);
}

// ODDA
void network_run(void *l2_buffer, size_t l2_buffer_size, void *l2_final_output, void **l3_buffer, int exec, int initial_dir)
{
  network_run_wait(network_run_async(l2_buffer, l2_buffer_size, l2_final_output, l3_buffer, exec, initial_dir));
}

void network_run_cluster(void *args) {
  unsigned int * real_args = (unsigned int *) args;
  void * l2_buffer = (void *) real_args[0];
  size_t l2_buffer_size = (size_t) real_args[1];
  void * l2_final_output = (void *) real_args[2];
  void ** l3_buffer = (void **) real_args[3];
  int exec = (int) real_args[4];
  int dir = (int) real_args[5];
/*
  - initial buffer allocation L2 and L1
  - variable declaration
*/
/* ---------------------------------- */
/* -------- SECTION 0 BEGIN --------- */
/* ---------------------------------- */
  void *L2_output = NULL;
  void *L2_input = NULL;
  void *L2_weights = NULL;
  void *L3_weights_curr = L3_weights;
  void *bypass_activations = NULL;

  int residual_number = 0;
  int bypass_dimension = 0;

  pi_perf_conf(1<<PI_PERF_CYCLES);
  int perf_cyc = 0;
  int io_cyc = 0;
/* ---------------------------------- */
/* --------- SECTION 0 END ---------- */
/* ---------------------------------- */

/*
  - initial copies from L3 of input
  - copies of weights of first 2 layers
*/
/* ---------------------------------- */
/* -------- SECTION 1 BEGIN --------- */
/* ---------------------------------- */
  directional_allocator_init(l2_buffer, l2_buffer_size);

/* ---------------------------------- */
/* --------- SECTION 1 END ---------- */
/* ---------------------------------- */
  // perf measurement begin
  cycle_network_execution = 0;
/* MAIN SECTION
  - for loop over all the layers of the network
  - double buffering using L3
  - check on layers to be executed from L3
  - residual check at the end of each layer
*/
/* ---------------------------------- */
/* -------- SECTION 2 BEGIN --------- */
/* ---------------------------------- */

  // IO cycle cycle measurement start
  pi_perf_reset();
  pi_perf_stop();
  pi_perf_start();

  int weight_l_cnt = 0; // count how many layers with weights we have processed to increment the weights_L3 pointer

  for (int i = 0; i < 11; i++) {
/* MEMORY ALLOCATION
  - allocate memory if layer is executed from L3;
  - allocate weights
  - read weights
*/
    L2_output = dmalloc(activations_out_size[i], !dir);
    if (L3_input_layers[i] == 1)
      L2_input = dmalloc(activations_size[i], dir);

    if (layer_with_weights[i] == 1)
      L2_weights = dmalloc(weights_size[i], dir);

    if (allocate_layer[i] == 1)
      cl_ram_read(L2_weights, L3_weights_curr, weights_size[i]);


    layer_args_t largs = {
      .L3_input = (unsigned int) L3_input,
      .L3_output = (unsigned int) L3_output,
      .L3_after_weights = (unsigned int) L3_weights_curr,
      .L2_input = (unsigned int) L2_input,
      .bypass = (unsigned int) bypass_activations,
      .L2_output = (unsigned int) L2_output,
      .L2_weights = (unsigned int) L2_weights,
      .L1_buffer = 0,
      .ram = (unsigned int) get_ram_ptr(),
      .padding = NET_UTILS_PAD_TOP | NET_UTILS_PAD_BOTTOM,
      .layer_id = i
    };

    pi_perf_stop();
    io_cyc += pi_perf_read(PI_PERF_CYCLES);

/*
- Execution of the layers_pointers
*/
    // perf measurement begin
    pi_perf_reset();
    pi_perf_stop();
    pi_perf_start();
    execute_layer_fork((void *) &largs);
    // performance measurements: end
    pi_perf_stop();
    perf_cyc =  pi_perf_read(PI_PERF_CYCLES);
    cycle_network_execution += perf_cyc;


    pi_perf_reset();
    pi_perf_stop();
    pi_perf_start();

    // TODO: What error?
    // prevents error from compiler
    asm volatile("": : :"memory");
    unsigned int temp = L3_input;
    L3_input = L3_output;
    asm volatile("": : :"memory");
    L3_output = temp;
    asm volatile("": : :"memory");

#ifdef VERBOSE
    pi_perf_stop();
    printf("Layer %s %d ended: \n", Layers_name[i], i);
    pi_perf_start();
#endif

    // Free memory
    if (layer_with_weights[i] == 1)
      dfree(weights_size[i], dir);
    dfree(activations_size[i], dir);
    if (branch_input[i] == 1)
      dfree(bypass_dimension, dir);
    L2_input = L2_output;
    // Residual connections
    if (i < 10) {
      if (branch_input[i+1] == 1) {
        bypass_activations = dmalloc(bypass_dimension, !dir);
        residual_number--;
        cl_ram_read(bypass_activations, layers_pointers[residual_number], bypass_dimension);
        cl_ram_free(layers_pointers[residual_number], bypass_dimension);
      }

      // TODO I feel like this should look ahead instead of back
      if (i > 0 && branch_output[i-1]==1 && L3_input_layers[i]==1) { // TODO don't understand this condition
        L3_input = cl_ram_malloc(1500000);
      }
      if (branch_output[i]==1 && L3_output_layers[i]==1) {
        cl_ram_free(L3_input + activations_out_size[i], 1500000 - activations_out_size[i]);
        layers_pointers[residual_number] = L3_input;
        residual_number++;
        bypass_dimension = activations_out_size[i];
      } else
      if (branch_output[i]==1 || branch_change[i] == 1) {
          layers_pointers[residual_number] = cl_ram_malloc(activations_out_size[i]);
          cl_ram_write(layers_pointers[residual_number], L2_output, activations_out_size[i]);
          residual_number++;
          bypass_dimension = activations_out_size[i];
      }

      if (branch_change[i]==1) {
        dfree(activations_out_size[i], !dir);
        L2_input = dmalloc(activations_size[i + 1], !dir);
        cl_ram_read(L2_input, layers_pointers[residual_number - 2], activations_size[i + 1]);
        cl_ram_free(layers_pointers[residual_number - 2], activations_size[i + 1]);
      }
      if (L3_output_layers[i] == 1)
        dfree(activations_out_size[i], !dir);
    }
    if (layer_with_weights[i])
       L3_weights_curr += L3_weights_size[weight_l_cnt++];
    dir = !dir;
  }

  pi_perf_stop();
  io_cyc += pi_perf_read(PI_PERF_CYCLES);
  cycle_network_execution += io_cyc;


  //memcpy(L2_output, l2_final_output, activations_out_size[10]); // BUGGY!
  for (int i=0; i<activations_out_size[10]; i++)
    *((uint8_t*)(l2_final_output+i)) = *((uint8_t*)(L2_output+i));

/* ---------------------------------- */
/* --------- SECTION 2 END ---------- */
/* ---------------------------------- */

/* ---------------------------------- */
/* -------- SECTION 3 BEGIN --------- */
/* ---------------------------------- */


/* ---------------------------------- */
/* --------- SECTION 3 END ---------- */
/* ---------------------------------- */
}
