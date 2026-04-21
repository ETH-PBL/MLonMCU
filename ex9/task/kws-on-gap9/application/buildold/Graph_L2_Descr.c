#include "SFU_RT.h"
#include "Graph.h"
unsigned int Graph_Clock[] = {
	0X00000001, /* N_CLOCK: 1, BASEID: 0 */
	0X00000003, /* C[0], C[1] */
	0X0001001E, /* CLK_CFG[0].VECT[0] Clk: UNSP SFU_StreamOut[0.0.0] */
	0X00000000, /* CLK_CFG[0].VECT[1] */
	0X00000000, /* CLK_CFG[0].VECT[2] */
	0X00000103, /* CLK_CFG[1].VECT[0] Clk: PDM0 SFU_MemOut[0.0.0] */
	0X00000000, /* CLK_CFG[1].VECT[1] */
	0X00000000, /* CLK_CFG[1].VECT[2] */
};
unsigned int Graph_Graph[] = {
	0X10010001, /* Used Blocks[0] SFU_MemIn.0 SFU_PdmIn.0 SFU_MemOut.0 */
	0X00000010, /* Used Blocks[1] SFU_StreamOut.0 */
	0X00000000, /* Used Blocks[2] */
	0X00000000, /* Used Blocks[3] */
	0X00000003, /* Used Clocks[0] */
	0X07FF0008, /* 00000005:       SFU_MemIn.0.0 CFG0: Routing:  SFU_StreamOut.0.0.0, UDMA_SOURCE_ID: 255, DATASIZE:  3, SIGN: 1 */
	0X00000000, /* 00000006:       SFU_MemIn.0.0 CFG1: CLK_SEL: 0, RT_EN: 0, RESAMPLING_D: 0, RESAMPLING_V:   0, PHASE:   0, COUNTER: 0 */
	0X00230000, /* 00000007:       SFU_PdmIn.0.0 CFG0: Routing:     SFU_MemOut.0.0.0, PDM_SAI_SEL:  3, PDM_CH_SEL:  0, CLK_SEL:  1, RT_EN: 0 */
	0X000183FF, /* 00000008:       SFU_PdmIn.0.0 CFG1: CIC_N:  7, CIC_M:  1, CIC_R: 63, CIC_SHIFT: 24 */
	0X000403FF, /* 00000009:      SFU_MemOut.0.0 CFG0: UDMA_TARGET_ID: 255, DATASIZE:  3, CLK_SEL:  1, RT_EN: 0 */
	0X0000030F, /* 0000000A:   SFU_StreamOut.0.0 CFG0: STREAM_ID: 15, DATASIZE:  3, CLK_SEL:   0, RT_EN: 0 */
};
SFU_RunTimeDescr_T Graph_RT_Descr = {
	0X1, /* Bitmask of enabled MEM_IN blocks */
	11, /* Number of 32-bit words in Graph descriptor array */
	5, /* Number of entries */
	Graph_Graph,
	Graph_Clock,
	0,
	{
		{0, 1,    7, 0,  0, 0,  0,  2, 0,  0,  1},  /*          PdmIn1:       SFU_PdmIn.0.0, Bound:  No, Offset:   7, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 1 */
		{0, 1,    5, 0,  0, 0,  0,  0, 0,  0,  4},  /*          MemIn1:       SFU_MemIn.0.0, Bound:  No, Offset:   5, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 4 */
		{1, 1,   10, 0,  0, 0,  0,  4, 0,  0,  4},  /*         PcmOut1:   SFU_StreamOut.0.0, Bound:  No, Offset:   A, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 4 */
		{1, 1,    9, 0,  0, 0,  0,  3, 0,  0,  1},  /*         MemOut1:      SFU_MemOut.0.0, Bound:  No, Offset:   9, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 1 */
		{3, 1,    2, 0,  0, 0,  0,  0, 0,  0,  0},  /* C[ 0]:           Clock, Bound:  No, Offset: 2 */
	}
};
