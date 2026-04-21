#ifndef __MFCCKERNEL_H__
#define __MFCCKERNEL_H__

#include "AutoTilerLibTypes.h"
#include "DspLib.h"
#define _L1_Memory_SIZE 45164
#define _L2_Memory_SIZE 0
extern char *L1_Memory; /* Size given for generation: 114688 bytes, used: 45164 bytes */
extern char *L2_Memory; /* Size used for generation: 0 bytes */
extern void Tensorflow_MFCC(
		f16 * __restrict__ In,
		f16 * __restrict__ Out,
		f16 * __restrict__ Twiddles_fft_int,
		f16 * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		short int * Mel_FilterBank,
		f16 * __restrict__ Mel_Coeffs,
		f16 * __restrict__ DCT_Coeff);
#endif
