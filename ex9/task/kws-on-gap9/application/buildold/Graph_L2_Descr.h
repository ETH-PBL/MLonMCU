#ifndef _GRAPH_L2_DESCR_H_
#define _GRAPH_L2_DESCR_H_
#include "SFU_RT.h"
extern unsigned int Graph_Clock[];
extern unsigned int Graph_Graph[];
#define Graph_PdmIn1 0
#define Graph_MemIn1 1
#define Graph_PcmOut1 2
#define Graph_MemOut1 3
#define Graph_Clock0 4
extern SFU_RunTimeDescr_T Graph_RT_Descr;
#endif
