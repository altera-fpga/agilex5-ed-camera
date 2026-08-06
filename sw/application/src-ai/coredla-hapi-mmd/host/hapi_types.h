/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#ifndef HAPI_TYPES_H

#define HAPI_TYPES_H

#include <vector>
#include <string>

typedef std::vector<std::string> board_names;

#define DMA

#define SUCCESS 0
#define FAILURE -1

typedef enum {
    HPS_MMD_COREDLA_CSR_HANDLE = 1, // COREDLA CSR Interface
    HPS_MMD_MEMORY_HANDLE = 2,      // Device Memory transfers
    HPS_MMD_STREAM_CONTROLLER_HANDLE = 3   // Stream Controller Interface
  } hps_mmd_interface_t;
  
#endif