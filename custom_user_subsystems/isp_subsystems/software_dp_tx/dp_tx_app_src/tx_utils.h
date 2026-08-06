/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

// DisplayPort Core test code debug routines definitions
//
// Description:
//
// ********************************************************************************

#if BITEC_TX_CAPAB_MST

typedef enum                   // PC fsm states
{
    PC_FSM_IDLE = 0,           // no operation
    PC_FSM_HPD_0,              // HPD set to 0
    PC_FSM_HPD_1,              // HPD set to 1
    PC_FSM_START,              // start checking a new connected sink
    PC_FSM_GET_PORTS,          // find connected MST ports
    PC_FSM_FIND_STREAM_1,      // find a port for Stream 1
    PC_FSM_FIND_STREAM_2,      // find a port for Stream 2
    PC_FSM_FIND_STREAM_3,      // find a port for Stream 3
    PC_FSM_RDEDID_0,           // EDID Stream 0 read
    PC_FSM_RDEDID_1,           // EDID Stream 1 read
    PC_FSM_RDEDID_2,           // EDID Stream 2 read
    PC_FSM_RDEDID_3,           // EDID Stream 3 read
    PC_FSM_ALLOCATE_STREAM_0,  // Allocate Stream 0 to port0_idx
    PC_FSM_WAIT_ALLOCATED_0,   // Wait for Stream 0 allocation
    PC_FSM_ALLOCATE_STREAM_1,  // Allocate Stream 1 to port1_idx
    PC_FSM_WAIT_ALLOCATED_1,   // Wait for Stream 1 allocation
    PC_FSM_ALLOCATE_STREAM_2,  // Allocate Stream 2 to port2_idx
    PC_FSM_WAIT_ALLOCATED_2,   // Wait for Stream 2 allocation
    PC_FSM_ALLOCATE_STREAM_3,  // Allocate Stream 3 to port3_idx
    PC_FSM_WAIT_ALLOCATED_3,   // Wait for Stream 3 allocation
    PC_FSM_MST_DATA,           // Start MST payload flow
    PC_FSM_MST_ON,             // MST ON
    PC_FSM_NOOUT               // no suitable output port available
} BTC_PC_STATE;

extern BTC_PC_STATE pc_fsm;  // PC fsm state

void bitec_dptx_pc();

#endif

void bitec_dptx_init();
void bitec_dptx_linktrain();
void bitec_dptx_linktrain_parameterized(unsigned int link_rate, unsigned int lane_count, unsigned int bpc);

