#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include "MfccKernels.h"
#ifdef __EMUL__
unsigned int __L3_Read, __L3_Write, __L2_Read, __L2_Write;
#endif
L1_CL_MEM AT_L1_POINTER L1_Memory;
L2_MEM AT_L2_POINTER L2_Memory;
extern AT_HYPERFLASH_FS_T HyperFlash;
void  Tensorflow_MFCC(
		f16 * __restrict__ In,
		f16 * __restrict__ Out,
		f16 * __restrict__ Twiddles_fft_int,
		f16 * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		short int * Mel_FilterBank,
		f16 * __restrict__ Mel_Coeffs,
		f16 * __restrict__ DCT_Coeff)

{
	/* Shared L1: 45164 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	AT_L2_EVENT _DmaR_Evt1, *DmaR_Evt1 = &_DmaR_Evt1;
	AT_L2_EVENT _DmaW_Evt1, *DmaW_Evt1 = &_DmaW_Evt1;
	Windowing_T S_KerArg0, *KerArg0 = &S_KerArg0;
	RFFT_Arg_T S_KerArg1, *KerArg1 = &S_KerArg1;
	CmplxMag_T S_KerArg2, *KerArg2 = &S_KerArg2;
	MelFilterBank_T S_KerArg3, *KerArg3 = &S_KerArg3;
	KerPieceWise_DSP_T S_KerArg4, *KerArg4 = &S_KerArg4;
	MatVect_DSP_T S_KerArg5, *KerArg5 = &S_KerArg5;

	/* Iteration space related variables */
	int D0Ind, D0Ind_Last;
	int T0Ind, T0Ind_Last;
	/* User kernel arguments related variables */
	/*============================= Ker Arg Iter Spaces =========================================
	User Kernel Iteration Space:
		[D0 Dim: 49][Tile0 Dim: 1]
	Ker Arg: In, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 49 logical tiles, 1 physical tiles
			@ 0 (Total Size: 32768 )[D0, [48 x 2048, 2048]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [48 x 2048, 2048]]
		Tile0: [0, 32768, 32768], Tile1: [0, 32768, 32768], Tile2; [0, 32768, 32768]
	Ker Arg: Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 49 logical tiles, 1 physical tiles
			@ 32768 (Total Size: 980 )[D0, [48 x 20, 20]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [48 x 20, 20]]
		Tile0: [0, 980, 980], Tile1: [0, 980, 980], Tile2; [0, 980, 980]
	Ker Arg: In_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 33748 (Total Size: 2048 )[Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: FFT_Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 35796 (Total Size: 2052 )[Tile0, 1:[1x1026], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1026], 2]
		Tile0: [0, 2052, 2052], Tile1: [0, 2052, 2052], Tile2; [0, 2052, 2052]
	Ker Arg: Buff2, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 37848 (Total Size: 2052 )[Tile0, 1:[1x1026], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1026], 2]
		Tile0: [0, 2052, 2052], Tile1: [0, 2052, 2052], Tile2; [0, 2052, 2052]
	Ker Arg: Twiddles_fft_int, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 39900 (Total Size: 1024 )[Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: SwapTable_fft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 40924 (Total Size: 1024 )[Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: Twiddles_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 41948 (Total Size: 2048 )[Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: Mel_FilterBank, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 43996 (Total Size: 60 )[Tile0, 1:[1x10], 6]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x10], 6]
		Tile0: [0, 60, 60], Tile1: [0, 60, 60], Tile2; [0, 60, 60]
	Ker Arg: Mel_Coeffs, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 44056 (Total Size: 908 )[Tile0, 1:[1x454], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x454], 2]
		Tile0: [0, 908, 908], Tile1: [0, 908, 908], Tile2; [0, 908, 908]
	Ker Arg: DCT_Coeff, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 44964 (Total Size: 200 )[Tile0, 1:[1x100], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x100], 2]
		Tile0: [0, 200, 200], Tile1: [0, 200, 200], Tile2; [0, 200, 200]
	======================== End Ker Arg Iter Spaces =========================================*/
	/*=========================== Call Kernel, Invariant assignment =====================*/
	KerArg0->OutFrame = (void *__restrict__) (L1_Memory+33748);
	KerArg0->WinSize = (int) (1024);
	KerArg0->FrameSize = (int) (1024);
	KerArg1->Data = (void * __restrict__) (L1_Memory+33748);
	KerArg1->RFFT_Out = (void * __restrict__) (L1_Memory+35796);
	KerArg1->Twiddles = (void * __restrict__) (L1_Memory+39900);
	KerArg1->RTwiddles = (void * __restrict__) (L1_Memory+41948);
	KerArg1->SwapTable = (void * __restrict__) (L1_Memory+40924);
	KerArg1->N_fft = (short int) (1024);
	KerArg2->FrameIn = (void *__restrict__) (L1_Memory+35796);
	KerArg2->FrameOut = (void *__restrict__) (L1_Memory+37848);
	KerArg2->N = (int) (513);
	KerArg3->FramePower = (void *__restrict__) (L1_Memory+37848);
	KerArg3->MelSpectr = (void *__restrict__) (L1_Memory+33748);
	KerArg3->Mel_Coeffs = (void *__restrict__) (L1_Memory+44056);
	KerArg3->Mel_FilterBank = (short int *__restrict__) (L1_Memory+43996);
	KerArg3->Mel_NBanks = (short int) (10);
	KerArg4->In = (void *__restrict__) (L1_Memory+33748);
	KerArg4->Out = (void *__restrict__) (L1_Memory+37848);
	KerArg4->N = (unsigned short int) (10);
	KerArg4->ExtraArg0 = (float) (0.000000e+00);
	KerArg4->ExtraArg1 = (float) (0.000000e+00);
	KerArg5->In1 = (void *__restrict__) (L1_Memory+37848);
	KerArg5->In2 = (void *__restrict__) (L1_Memory+44964);
	KerArg5->InDim = (unsigned short int) (10);
	KerArg5->OutDim = (unsigned short int) (10);
	/*================================= Read Tiles Prolog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) In+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+0), 32768, 0, DmaR_Evt1);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_rfft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+41948), 2048, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_fft_int+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+39900), 1024, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) SwapTable_fft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+40924), 1024, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Mel_Coeffs+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+44056), 908, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) DCT_Coeff+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+44964), 200, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Mel_FilterBank+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+43996), 60, 0, 0);
	AT_L2_WAIT(0, DmaR_Evt1); /* Wait previous DMA read Mel_FilterBank */
	/*============================= End Read Tiles Prolog ===============================*/
	for (D0Ind=0; D0Ind<49; D0Ind++) { /* Iteration on D0 */
		int D0Ind_Last = (D0Ind==48);
		{ /* Single iteration on Tile0 */
			int T0Ind_Last = 1;
			/*====================== Call Kernel LOC_LOOP =========================*/
			KerArg0->Frame = (void *__restrict__) (L1_Memory+0+((D0Ind)*640));
			AT_FORK(gap_ncore(), (void *) WindowingReal2Real_PadCenter_f16, (void *) KerArg0);
			__CALL(WindowingReal2Real_PadCenter_f16, KerArg0);
			AT_FORK(gap_ncore(), (void *) RFFT_DIF_Par_f16, (void *) KerArg1);
			__CALL(RFFT_DIF_Par_f16, KerArg1);
			AT_FORK(gap_ncore(), (void *) CmplxMag_f16, (void *) KerArg2);
			__CALL(CmplxMag_f16, KerArg2);
			AT_FORK(gap_ncore(), (void *) MelFilterBank_f16, (void *) KerArg3);
			__CALL(MelFilterBank_f16, KerArg3);
			AT_FORK(gap_ncore(), (void *) Log_f16_f32, (void *) KerArg4);
			__CALL(Log_f16_f32, KerArg4);
			KerArg5->Out = (void *__restrict__) (L1_Memory+32768+((D0Ind)*20));
			AT_FORK(gap_ncore(), (void *) KerParMatVectDSP_f16, (void *) KerArg5);
			__CALL(KerParMatVectDSP_f16, KerArg5);
		} /* End iteration on Tile0 */
	} /* End iteration on D0 */
	/*================================ Write Tiles Epilog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Out+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+32768), 980, 1, DmaW_Evt1);
	AT_L2_WAIT(0, DmaW_Evt1); /* Wait DMA write Out */
	/*============================ End Write Tiles Epilog ===============================*/
}
#pragma GCC diagnostic pop
