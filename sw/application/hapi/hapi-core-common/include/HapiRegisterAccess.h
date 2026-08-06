/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#ifdef __cplusplus
#include <cstdint>
extern "C" uint32_t HapiReadRegister(void* pHapiItem, uint32_t registerIndex);
extern "C" void HapiWriteRegister(void* pHapiItem, uint32_t registerIndex, uint32_t registerValue);
#else
#include <stdint.h>
extern uint32_t HapiReadRegister(void* pHapiItem, uint32_t registerIndex);
extern void HapiWriteRegister(void* pHapiItem, uint32_t registerIndex, uint32_t registerValue); 
#endif
