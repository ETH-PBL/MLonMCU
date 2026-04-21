#include "AutoTilerLib.h"
#include "AutoTilerLibTypes.h"
#include "DSP_Generators.h"
#include "MFCC_params.h"

void MFCCConfiguration(unsigned int L1Memory)
{
  SetInlineMode(ALWAYS_INLINE);
  SetSymbolDynamics();

  SetUsedFilesNames(0, 1, "DspLib.h");
  SetGeneratedFilesNames("MfccKernels.c", "MfccKernels.h");

  SetL1MemorySize(L1Memory);
}

int main(int argc, char **argv)
{
    if (TilerParseOptions(argc, argv)) GenTilingError("Failed to initialize or incorrect output arguments directory.\n");
    CNN_GenControl_T Tensorflow_Settings;
    CNN_InitGenCtrl(&Tensorflow_Settings);
    CNN_SetGenCtrl(&Tensorflow_Settings, "PADTYPE", AT_OPT_VAL(3)); // Pad Balanced Right == Center pad of Tensorflow

    // Set Auto Tiler configuration, given shared L1 memory is 51200
    MFCCConfiguration(112*1024);
    // Load FIR basic kernels
    // LoadMFCCLibrary();
    LoadDSPLibrary();
    // Generate code for MFCC applied to 49 of size FRAME_SIZE with FRAME_STEP as stride
    DSP_MFCC_Generator("Tensorflow_MFCC",                    &Tensorflow_Settings, 49, FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 1, 1, 0, 0, 0, DATA_TYPE);
    // MFCC_Generator("Tensorflow_MFCC",                    &Tensorflow_Settings, 49, 640, 320, 1024, 40, 494, 40, 0, 0, 0, 1, DATA_TYPE, 2, 0);

    // // Generate code for MFCC applied to a single frame just for code generation testing
    // MFCC_Generator("Tensorflow_MFCC_single_Fix16",       &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 0, 2, 0);
    // MFCC_Generator("Tensorflow_MFCC_single_Fix16_FFT",   &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 0, 2, 1);
    // MFCC_Generator("Tensorflow_LogMel_single_Fix16",     &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 0, 2, 0);
    // MFCC_Generator("Tensorflow_LogMel_single_Fix16_FFT", &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 0, 2, 1);

    // MFCC_Generator("Tensorflow_MFCC_single_Fix32",       &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 1, 2, 0);
    // MFCC_Generator("Tensorflow_MFCC_single_Fix32_FFT",   &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 1, 2, 1);
    // MFCC_Generator("Tensorflow_LogMel_single_Fix32",     &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 1, 2, 0);
    // MFCC_Generator("Tensorflow_LogMel_single_Fix32_FFT", &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 1, 2, 1);

    // MFCC_Generator("Tensorflow_MFCC_single_f16",         &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 2, 2, 0);
    // MFCC_Generator("Tensorflow_MFCC_single_f16_FFT",     &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 2, 2, 1);
    // MFCC_Generator("Tensorflow_LogMel_single_f16",       &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 2, 2, 0);
    // MFCC_Generator("Tensorflow_LogMel_single_f16_FFT",   &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 2, 2, 1);

    // MFCC_Generator("Tensorflow_MFCC_single_f32",         &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 3, 2, 0);
    // MFCC_Generator("Tensorflow_MFCC_single_f32_FFT",     &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, 3, 2, 1);
    // MFCC_Generator("Tensorflow_LogMel_single_f32",       &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 3, 2, 0);
    // MFCC_Generator("Tensorflow_LogMel_single_f32_FFT",   &Tensorflow_Settings, 1,  FRAME_SIZE, FRAME_STEP, N_FFT, N_MELS, MEL_COEFF_CNT, 0,     0, 0, 0, USE_POWER, 3, 2, 1);
    GenerateTilingCode();
}
