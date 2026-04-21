/*
 * Copyright (C) 2022 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

/* 
    include files
*/


#include "application.h"

#define DATA_TYPE 2
#if (DATA_TYPE==2)
typedef float16 MFCC_IN_TYPE;
typedef float16 OUT_TYPE;
#elif (DATA_TYPE==3)
typedef float MFCC_IN_TYPE;
typedef float OUT_TYPE;
#else
typedef short int OUT_TYPE;
typedef short int MFCC_IN_TYPE;
#endif

// Peripherals
#include "Gap.h"
#include "ram.h"
#include "hostfs.h"
#include "gaplib/wavIO.h" 
#include "Graph_L2_Descr.h" // pdm_in_test
#include "localutil.h"

// PMSIS SFU
#include "sfu_pmsis_runtime.h"
#define NB_BUF_IN_RING 2
#define FREQ_PDM_BIT (3072000)
#define FREQ_PCM (48000)
#define SAI_RX (1)
#define SAI_TX (0)

// MFCC
#include "MFCC_params.h"
#include "MfccKernels.h"
#include "DCTTwiddles.def"
#include "MelFBSparsity.def"
#include "WindowLUT.def"
#include "FFTTwiddles.def"
#include "RFFTTwiddles.def"
#include "MelFBCoeff.def"
#include "SwapTable.def"

// DORY
#include "mem.h"
#include "network.h"

#include "input.h"

#define WAVRAM 2*16000 // int16, 1-second @ 16 kHz 
#define EPSILON 0.2746

// measurement
pi_gpio_e gpio_pin_measurement;
unsigned int gpio_pin_measurement_id = 89;

/* 
     global variables
*/
struct pi_device DefaultRam; 
struct pi_device* ram = &DefaultRam;

//static struct pi_default_flash_conf flash_conf;
static pi_fs_file_t * file[1];
static struct pi_device fs;
static struct pi_device flash;

// Load args
char *WavName = NULL;
char *mfcc = NULL;
char *appl_input = NULL;

// Arrays handling data movement
short int *inWav;
MFCC_IN_TYPE *MfccInSig;
MFCC_IN_TYPE *MfccInSig_prev;
int16_t *MfccInSig_int16; // for saving
MFCC_IN_TYPE *RecordedNoise;
int16_t *RecordedNoise_int16; // for saving
OUT_TYPE *out_feat;
char * feat_char;

void *l2_buffer;
void * L2_FC_weights_float;
void *L2_FC_weights_int8;

SFU_uDMA_Channel_T *ChanOutCtxt_0;
void * BufferInList;
void * BufferOutList;

// PMSIS SFU
// SFU
static pi_sfu_graph_t *sfu_graph;
static uint8_t sfu_input_id;
static uint8_t sfu_output_id;

// SAI used for receiving and sending PDM
static pi_device_t sai_dev_rx;
static pi_device_t sai_dev_tx;

// Audio buffers
static pi_sfu_buffer_t sfu_out_buffers[NB_BUF_IN_RING]; // Buffers for SFU(MEM_OUT) -> L2 transfers
static pi_sfu_buffer_t sfu_in_buffers[NB_BUF_IN_RING]; // BUffers for L2 -> SFU(MEM_IN) transfer
static int sfu_out_buffer_idx = 0;
static int sfu_out_buffer_cnt = 0;
static int sfu_in_buffer_idx = 0;
static int sfu_in_buffer_cnt = 0;

static pi_evt_t sfu_out_task;
static pi_evt_t sfu_in_task;

static pi_sfu_mem_port_t * memin_port;
static pi_sfu_mem_port_t * memout_port;

static int sfu_buffer_filled = 0;

int noise_seconds = 1;

static const pi_gpio_e gpio_boot_pin_1 = PAD_GPIO_UPB;

// Global declaration 
struct pi_device cluster_dev;
struct pi_cluster_conf cl_conf;
struct pi_cluster_task cl_task;

static pi_event_t inference_task;

static void *L3_wavs = NULL;

// Configure PDM RX interface
static int configure_pdm()
{
    int res = 0;
    int err;

    pi_pad_function_set(SAI_SCK(SAI_RX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_WS (SAI_RX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDI(SAI_RX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDO(SAI_RX), PI_PAD_FUNC0);

    struct pi_i2s_conf i2s_conf;
    pi_i2s_conf_init(&i2s_conf);
    i2s_conf.options = PI_I2S_OPT_INT_CLK | PI_I2S_OPT_REF_CLK_FAST;
    i2s_conf.frame_clk_freq = FREQ_PDM_BIT;
    i2s_conf.itf = SAI_RX;
    i2s_conf.mode = PI_I2S_MODE_PDM;
    i2s_conf.pdm_direction = 0b11;
    i2s_conf.pdm_diff = 0b00;

    pi_open_from_conf(&sai_dev_rx, &i2s_conf);
    if (pi_i2s_open(&sai_dev_rx))
    {
        printf("Failed to open PDM Rx\n");
        res = -1;
    }

    // Connect to SFU
    if (res == 0)
    {
        pi_sfu_pdm_itf_id_t itf_id =
        {
            SAI_RX,
            2,
            0
        };
        err = pi_sfu_graph_pdm_bind(sfu_graph, SFU_Name(Graph, PdmIn1), &itf_id);
        if (err != 0)
            res = -1;
    }

    return res;
}

// Configure I2S Tx interface
static int configure_i2s()
{
    int err;

    pi_pad_function_set(SAI_SCK(SAI_TX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_WS(SAI_TX),  PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDI(SAI_TX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDO(SAI_TX), PI_PAD_FUNC0);

    int32_t stream_ch;
    struct pi_i2s_conf i2s_conf;
    pi_i2s_conf_init(&i2s_conf);

    i2s_conf.itf = SAI_TX;
    i2s_conf.frame_clk_freq = FREQ_PCM;
    i2s_conf.slot_width = 32;
    i2s_conf.channels = 1;

    pi_open_from_conf(&sai_dev_tx, &i2s_conf);
    if (pi_i2s_open(&sai_dev_tx))
        printf("Failed to open SAI %d in I2S mode\n", SAI_TX);

    // Tx slot
    pi_sfu_i2s_itf_id_t itf_id = {SAI_TX, 1};
    err = pi_sfu_graph_i2s_bind(sfu_graph, SFU_Name(Graph, PcmOut1), &itf_id, &stream_ch);
    if (err != 0)
    {
        printf("Unable to bind I2S(SAI: %d, Ch: %d) to SFU STREAM block\n", SAI_TX, 0);
        return -1;
    }

    struct pi_i2s_channel_conf i2s_slot_conf;
    pi_i2s_channel_conf_init(&i2s_slot_conf);
    i2s_slot_conf.options = PI_I2S_OPT_IS_TX | PI_I2S_OPT_ENABLED;
    i2s_slot_conf.word_size = 32;
    i2s_slot_conf.format = PI_I2S_CH_FMT_DATA_ORDER_MSB | PI_I2S_CH_FMT_DATA_ALIGN_LEFT | PI_I2S_CH_FMT_DATA_SIGN_NO_EXTEND;
    i2s_slot_conf.stream_id = stream_ch;

    if (pi_i2s_channel_conf_set(&sai_dev_tx, 0, &i2s_slot_conf))
        return -1;

    return 0;
}

// PMSIS SFU
static void handle_out_transfer_end(void *arg)
{
    pi_sfu_enqueue(sfu_graph, memout_port, &sfu_out_buffers[sfu_out_buffer_idx]);

    /*
     * Buffer received from MEM_OUT.
     * Here we just do a simple copy to the MEM_IN buffer that is not currently being transferred.
     */
    int in_idx = sfu_in_buffer_idx ^ 1;
    int out_idx = sfu_out_buffer_idx;
        
    int start;
    int elapsed;

    memcpy(BufferInList+sfu_out_buffer_cnt*DOUBLE_BUFF_SIZE*sizeof(int32_t), sfu_out_buffers[out_idx].data, DOUBLE_BUFF_SIZE*sizeof(int32_t));

    sfu_out_buffer_cnt++;
    if (sfu_out_buffer_cnt*DOUBLE_BUFF_SIZE*sizeof(int32_t) >= BUFF_SIZE) { // one seccond is added to the main buffer
        sfu_buffer_filled = 1;
    }
    if (sfu_out_buffer_cnt == BUFF_SIZE/DOUBLE_BUFF_SIZE/sizeof(int32_t)){
        sfu_out_buffer_cnt = 0;
    }
    if (sfu_buffer_filled){
        pi_evt_push(&inference_task);
    }
    sfu_out_buffer_idx ^= 1;
}

static void handle_in_transfer_end(void *arg)
{
    pi_sfu_enqueue(sfu_graph, memin_port, &sfu_in_buffers[sfu_in_buffer_idx]);

    sfu_in_buffer_cnt++;
    sfu_in_buffer_idx ^= 1;
}

// MFCC Computation
static void RunMFCC()
{   
    // Compute MFCC following Tensorflow settings
    #if (N_DCT == 0)
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #else
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #endif
    #else
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        // DEBUG
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, DCTTwiddles);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM, DCTTwiddles);
        #else
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM, DCTTwiddles);
        #endif
    #endif
}


void configure_microphone(int save, int free, int noise){

    int err;

    // Open SFU with default frequency
    pi_sfu_conf_t conf = { .sfu_frequency=0 };
    if (pi_sfu_open(&conf))
        printf("SFU device open failed\n");
    printf("SFU activated\n");

    sfu_graph = pi_sfu_graph_open(&SFU_RTD(Graph));
    if (sfu_graph == NULL)
        printf("SFU graph open failed\n");
    printf("Graph opened\n");

    // Allocate IO buffers
    for (int i = 0; i < NB_BUF_IN_RING; i++)
    {
        void *data_out = pi_l2_malloc(DOUBLE_BUFF_SIZE * sizeof(int));
        if (data_out == NULL) return -1;
        pi_sfu_buffer_init(&sfu_out_buffers[i], data_out, DOUBLE_BUFF_SIZE, sizeof(int));

    }

    // Configure interfaces
    err = configure_pdm();
    if (err != 0)
        printf("PDM interface init failed\n");
    printf("PDM Rx interface configured\n");

    memout_port = pi_sfu_mem_port_get(sfu_graph, SFU_Name(Graph, MemOut1));
    if (memout_port == NULL)
        printf("Failed to get memout_port references\n");

    // Prepare buffer transfer callbacks
    pi_evt_callback_irq_init(&sfu_out_task, handle_out_transfer_end, NULL);
    
    // Enqueue first two buffers on each side
    for (int i = 0; i < NB_BUF_IN_RING; i++)
    {
        sfu_out_buffers[i].task = &sfu_out_task;
        pi_sfu_enqueue(sfu_graph, memout_port, &sfu_out_buffers[i]);
    }

    pi_sfu_graph_load(sfu_graph);
    pi_i2s_ioctl(&sai_dev_rx, PI_I2S_IOCTL_START, NULL);

    // pi_time_wait_us(2000000);
    // pi_time_wait_us(100000);

    // pi_i2s_ioctl(&sai_dev_rx, PI_I2S_IOCTL_STOP, NULL);
    // // pi_i2s_ioctl(&sai_dev_tx, PI_I2S_IOCTL_STOP, NULL);

}


void input_wav(int save, int free, char* wavfile, int noise){
    // Allocate L3 buffers for audio IN
     
    header_struct header_info;

    int step1 = pi_time_get_us();

    inWav = NULL;
    inWav    = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
    if (inWav == NULL){
        printf("Failed allocating inWav.\n");
        pmsis_exit(-1);
    }

    PRINTF("File is: %s\n", wavfile);

    int step2 = pi_time_get_us();

    if (ReadWavFromFile(wavfile, inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
        printf("Error reading wav file\n");
        pmsis_exit(1);
    }

    for (int i = 0; i < 5; i++){
        PRINTF("inWav[%i] = %i, ", i, inWav[i]);
    }
    PRINTF("\n");

    int step3 = pi_time_get_us();

    if (noise){
        RecordedNoise = NULL;
        RecordedNoise = (MFCC_IN_TYPE *) pi_l2_malloc(noise_seconds*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        if (RecordedNoise == NULL){
            printf("Failed allocating RecordedNoise.\n");
            pmsis_exit(-1);
        }
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<noise_seconds*AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                // READ WAV
                RecordedNoise[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
                // READ TEXT
                // RecordedNoise[i] = (MFCC_IN_TYPE) noisemeeting[i] / (1<<15);
            }
        #else
            for (int i=0; i<noise_seconds*AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                // READ WAV
                RecordedNoise[i] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
                // READ TEXT
                // RecordedNoise[i] = (MFCC_IN_TYPE) gap_fcip(((int) noisemeeting[i]), 15);
            }
        #endif

    }
    else {
        MfccInSig = NULL;
        MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        if (MfccInSig == NULL){
            printf("Failed allocating MfccInSig.\n");
            pmsis_exit(-1);
        }
    
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                MfccInSig[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
            }
        #else
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                MfccInSig[i] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
            }
        #endif
    }

    int step4 = pi_time_get_us();
    
    // DEBUG
    save = 1;
    if (save){
        // Log WAV 
        // TODO: use *_int16 for saving
        if (noise){
            dump_wav_open("noise_file.wav", 16, 16000, 1, noise_seconds*sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_write(inWav, noise_seconds*sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_close();
            PRINTF("Writing wav file to noise_file.wav completed successfully\n"); 
        }
        else{   
            dump_wav_open("utter_file.wav", 16, 16000, 1, sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_write(inWav, sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_close();
            PRINTF("Writing wav file to utter_file.wav completed successfully\n");
        }
    }

    int step5 = pi_time_get_us();

    if (free){
        if (noise){
            pi_l2_free(inWav, noise_seconds*AUDIO_BUFFER_SIZE * sizeof(short));
        }
        else{
            pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));
        }
    }

    int step6 = pi_time_get_us();

}

void compute_mfcc(){
    /******
        Compute the MFCC
    ******/
    out_feat = (OUT_TYPE *) pi_l2_malloc(49 * N_MELS * sizeof(OUT_TYPE));    


    // struct pi_cluster_task task_mfcc;

    struct pi_cluster_task* task_mfcc;
    task_mfcc = pi_l2_malloc(sizeof(struct pi_cluster_task));
    pi_cluster_task(task_mfcc, &RunMFCC, NULL);
    pi_cluster_task_stacks(task_mfcc, NULL, SLAVE_STACK_SIZE);

    pi_cluster_conf_init(&cl_conf);
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
      return -1;
    }

    L1_Memory = pi_l1_malloc(&cluster_dev, _L1_Memory_SIZE);
    if (L1_Memory==NULL){
        printf("Error allocating L1\n");
        pmsis_exit(-1);
    }
   
    // pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(task_mfcc, RunMFCC, NULL));

    
    pi_cluster_send_task_to_cl(&cluster_dev, task_mfcc);
    pi_l2_free(task_mfcc, sizeof(struct pi_cluster_task));

    pi_cluster_close(&cluster_dev);

    pi_l2_free(MfccInSig, noise_seconds * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));

}


int application(){

    // printf ("----------------------------- Initializing environment ---------------------------\n");

    // Voltage-Frequency settings
    uint32_t voltage =VOLTAGE;
    pi_freq_set(PI_FREQ_DOMAIN_FC,      FREQ_FC*1000*1000);
    pi_freq_set(PI_FREQ_DOMAIN_PERIPH,  FREQ_FC*1000*1000);

#ifdef AUDIO_EVK
    pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, VOLTAGE);
    pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, VOLTAGE);
#endif 

    //PMU_set_voltage(voltage, 0);
    printf("Set VDD voltage as %.2f, FC Frequency as %d MHz, CL Frequency = %d MHz\n", 
        (float)voltage/1000, FREQ_FC, FREQ_CL);

    /****
        Configure And Open the External Ram. 
    ****/
    struct pi_default_ram_conf ram_conf;
    pi_default_ram_conf_init(&ram_conf);
    ram_conf.baudrate = FREQ_FC*1000*1000;
    pi_open_from_conf(&DefaultRam, &ram_conf);
    if (pi_ram_open(&DefaultRam))
    {
        printf("Error ram open !\n");
        pmsis_exit(-3);
    }
    printf("RAM Opened\n");

    /****
        Configure And open cluster. 
    ****/
    
    pi_cluster_conf_init(&cl_conf);
    cl_conf.cc_stack_size = STACK_SIZE;
    cl_conf.id = 0;                /* Set cluster ID. */
                       // Enable the special icache for the master core
    cl_conf.icache_conf = PI_CLUSTER_MASTER_CORE_ICACHE_ENABLE |   
                       // Enable the prefetch for all the cores, it's a 9bits mask (from bit 2 to bit 10), each bit correspond to 1 core
                       PI_CLUSTER_ICACHE_PREFETCH_ENABLE |      
                       // Enable the icache for all the cores
                       PI_CLUSTER_ICACHE_ENABLE;
    pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        PRINTF("Cluster open failed !\n");
        pmsis_exit(-4);
    }
    // printf("Cluster Opened\n");
    pi_freq_set(PI_FREQ_DOMAIN_CL, FREQ_CL*1000*1000);


    // Configure User Button
    /* set pad to gpio mode */
    /* This will open the gpio automatically */
    pi_pad_function_set(gpio_boot_pin_1, PI_PAD_FUNC1);
    /* configure gpio input */
    pi_gpio_flags_e flags_upb = PI_GPIO_INPUT;
    pi_gpio_pin_configure(gpio_boot_pin_1, flags_upb);


    // printf ("----------------------------- Initializing backbone ---------------------------\n");

    // Dory init
    mem_init();
    network_initialize(); // Absent in L2-only
    pi_cluster_close(&cluster_dev);

    // printf ("----------------------------- Read WAV from filesystem ---------------------------\n");
    L3_wavs = ram_malloc(WAVRAM);
    printf("\nL3_wavs alloc initial\t@ %d:\t%s\n", (unsigned int)L3_wavs, L3_wavs?"Ok":"Failed");

    header_struct header_info;
    inWav = NULL;
    inWav    = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
    if (inWav == NULL){
        printf("Failed allocating inWav.\n");
        pmsis_exit(-1);
    }

    printf ("File to save: %s\n", WavName);
    if (ReadWavFromFile(WavName, inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
        printf("Error reading wav file\n");
        pmsis_exit(1);
    }

    // printf ("----------------------------- Write WAV to RAM ---------------------------\n");
    ram_write(L3_wavs, inWav, AUDIO_BUFFER_SIZE*sizeof(short));
    pi_l2_free(inWav, AUDIO_BUFFER_SIZE*sizeof(short));


    BufferInList = (void*) pi_l2_malloc(BUFF_SIZE);
    if (BufferInList == NULL) return -1;

    int button_was_pressed = 0;

    l2_buffer = NULL;
    l2_buffer = pi_l2_malloc(L2_MEMORY_SIZE);
    if (l2_buffer == NULL) {
        printf("failed to allocate memory for l2_buffer\n");
    }

    pi_evt_sig_init(&inference_task);

    configure_microphone(1, 1, 0);

    int iterations = 0;
    int sfu_out_buffer_cnt_prev = 0;
    int sfu_out_buffer_cnt_curr = 0;

    MfccInSig_prev = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));
    int len = 0;
    int upperlim = 0;
    int lowerlim = 0;

    printf ("----------------------------- Starting application ---------------------------\n");

    while (1){
            
        if (appl_input == "0"){

            // ----------------------------- Start acquisition ---------------------------    

	    int threshold_counter = 0;
            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));

            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){ 
                MfccInSig[i] = MfccInSig_prev[i];
            }  
            pi_evt_wait(&inference_task);

            sfu_out_buffer_cnt_curr = sfu_out_buffer_cnt;

            if (sfu_out_buffer_cnt_curr > sfu_out_buffer_cnt_prev){
                len = sfu_out_buffer_cnt_curr - sfu_out_buffer_cnt_prev;
                upperlim = sfu_out_buffer_cnt_curr * DOUBLE_BUFF_SIZE;
                lowerlim = 0;
            }
            else{
                len = (48 - sfu_out_buffer_cnt_prev) + sfu_out_buffer_cnt_curr;
                upperlim = BUFF_SIZE/sizeof(int32_t);
                lowerlim = sfu_out_buffer_cnt_curr * DOUBLE_BUFF_SIZE;
            }

            // TODO: FIGURE OUT SCALING 2^32 or 2^31???
            int outidx = 0;
            for (int i = sfu_out_buffer_cnt_prev*DOUBLE_BUFF_SIZE; i < upperlim; i+=3){
                // using MfccInSig_prev as buffer
                MfccInSig_prev[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                outidx++;
            }
            for (int i = 0; i < lowerlim; i+=3){
                // using MfccInSig_prev as buffer
                MfccInSig_prev[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                outidx++;
            }

            int j = 0;
            for (int i = DOUBLE_BUFF_SIZE*(len)/3; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[j] = MfccInSig[i];    
                j++;
            }
            
            int k = 0;
            for (int i = AUDIO_BUFFER_SIZE - DOUBLE_BUFF_SIZE*(len)/3; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[i] = MfccInSig_prev[k];
                k++;
            }

            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig_prev[i] = MfccInSig[i];
            }
            sfu_out_buffer_cnt_prev = sfu_out_buffer_cnt_curr;
            
            MfccInSig_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);

            float mean = 0;
            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){
                MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
                mean = mean + MfccInSig_int16[i]; 
            }

            mean = mean/AUDIO_BUFFER_SIZE;

            // TODO: FIGURE OUT THRESHOLD
            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){
                if(MfccInSig_int16[i] < mean - 1000 || MfccInSig_int16[i] > mean + 1000){
                    threshold_counter++;
                }
            }
        }
        else if (appl_input == "1"){

            // ----------------------------- Read .wav ---------------------------   
           
            
            int start_readwav = pi_time_get_us();
            
            short int *prepWav = NULL;
            prepWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));

            // Read the 0th .wav saved in RAM
            ram_read(prepWav, L3_wavs, AUDIO_BUFFER_SIZE*sizeof(short));

            MfccInSig = NULL;
            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
            if (MfccInSig == NULL){
                printf("Failed allocating MfccInSig.\n");
                pmsis_exit(-1);
            }
        
            #if (DATA_TYPE==2) || (DATA_TYPE==3)
                for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                    MfccInSig[i] = (MFCC_IN_TYPE) prepWav[i] / (1<<15);
                }
            #else
                for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                    MfccInSig[i]] = (MFCC_IN_TYPE) gap_fcip(((int) prepWav[i]), 15);
                }
            #endif

            MfccInSig_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);

            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){
                MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
            }

            pi_l2_free(prepWav, AUDIO_BUFFER_SIZE * sizeof(short int));


            int end_readwav = pi_time_get_us();
            // printf("Time spent reading wav: %i\n", end_readwav - start_readwav);
        }

        // ***************************** Computing MFCC ************************** 

        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_mfcc = gap_fc_readhwtimer();        
        int start_readmfcc = pi_time_get_us();

        compute_mfcc();

        int elapsed_timer_mfcc = gap_fc_readhwtimer() - start_timer_mfcc;
        int end_readmfcc = pi_time_get_us();
        #ifdef PERF
        printf("Compute mfcc: %d cycles (%i us)\n", elapsed_timer_mfcc, end_readmfcc - start_readmfcc);
        #endif

        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_processing = gap_fc_readhwtimer();        
        int start_readprocessing = pi_time_get_us();

        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));

        int k = 0;
        for (int i = 0; i < 49 * N_MELS;i++){                
            
            // feat_char[k] = (char) ((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05)) + 128);
            // feat_char[k] = (char) ((int) floor(out_feat[i] * 0.1118) + 128);
            feat_char[k] = (char) ((int) floor(out_feat[i] * EPSILON) + 128);

            if (N_MELS == 40){
                // Select 10 MFCC per window
                if (i == 40*(k/10) + 9){
                    i = 40*(k/10) + 39;
                }
            }

            // Fill input buffer
            if (mfcc == "1"){
                ((uint8_t *)l2_buffer)[k] = L2_input_h[k]; // Precomputed MFCC
            }
            else {
                ((uint8_t *)l2_buffer)[k] = feat_char[k]; // Online computed MFCC
            }

            k++;
        } 

        // if DEBUG
        // dump_data_write("mfccdump.dat", feat_char, 49 * 10 * sizeof(char));


        pi_l2_free(out_feat, 49 * N_MELS * sizeof(OUT_TYPE));
        pi_l2_free(feat_char, 49 * 10 * sizeof(char));

        int elapsed_timer_processing = gap_fc_readhwtimer() - start_timer_processing;
        int end_readprocessing = pi_time_get_us();
        #ifdef PERF
        printf("Processing: %d cycles (%i us)\n", elapsed_timer_processing, end_readprocessing - start_readprocessing);
        #endif 

        // ***************************** Backbone inference **************************

        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_backbone = gap_fc_readhwtimer();
        int start_backbone = pi_time_get_us();

        // Extract backbone features
        void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0, 1);

        int end_backbone = pi_time_get_us();
        int elapsed_timer_backbone = gap_fc_readhwtimer() - start_timer_backbone;
        #ifdef PERF
        printf("Backbone: %i cycles (%i us)\n", elapsed_timer_backbone, end_backbone - start_backbone);
        #endif 

        int n_classes = 12;
        predict(l2_buffer, n_classes);

        // #ifdef PERF
        // Saving .wav is slow and will affect sampling
        dump_wav_open("utterance.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        dump_wav_close();        
        // #endif

        pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);        
        
        // ***************************** Application complete *****************************

        return 0; // comment out for always-on inference
    }



    return 0;
}

int main()
{
    PRINTF("\n\n\t *** Application ***\n\n");

    #define __XSTR(__s) __STR(__s)
    #define __STR(__s) #__s
    WavName = __XSTR(WAV_FILE); 
    mfcc = __XSTR(MFCC);
    appl_input = __XSTR(APPL);

    return application();
}
