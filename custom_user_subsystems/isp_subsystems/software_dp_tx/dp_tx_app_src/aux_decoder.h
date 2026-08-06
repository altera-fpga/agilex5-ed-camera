/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

//--------------------------------------------------------------------------
//!
//! @brief DisplayPort Aux Decoder Definitions 
//!
//! @file aux_decoder.h
//!
//! @struct AuxDecoderInstance
//!
//--------------------------------------------------------------------------
#pragma once
#if (BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX) || (BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX)

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef ENABLE_AUX_RING_BUFFER
#define ENABLE_AUX_RING_BUFFER 1 // Set to one to enable the AUX channel soft ring buffer
#endif // ENABLE_AUX_RING_BUFFER

//--------------------------------------------------------------------------
//!
//! @brief eDecoderState - State the decoder enters
//!
//--------------------------------------------------------------------------
typedef enum
{
    kIdle = 0, //!< Waiting for a new packet to start
    kWaitingForAddr1 = 1, //!< Read the first byte of the address
    kWaitingForAddr2 = 2, //!< Read the second byte of the address
    kReadingPayload  = 3, //!< Expect payload data
} eDecoderStates;

#if ENABLE_AUX_RING_BUFFER
#define MAX_FIFO_BURST_SAMPLE (128) //!< Burst size when data available in the fifo
#define MAX_FIFO_SAMPLE (MAX_FIFO_BURST_SAMPLE * 8) //!< Max number of samples we'll read at any time
#define MAX_FIFO_BUFFER_SIZE (2*MAX_FIFO_SAMPLE) //!< Size of the buffer to cache the fifo

//--------------------------------------------------------------------------
//!
//! @brief Fifo - A cache used to pull data into memory and process when available
//!
//--------------------------------------------------------------------------
typedef struct
{
    unsigned int _base_addr_csr; //!< Pointer to the base address of the Avalon Fifo CSR
    unsigned int _base_addr_fifo; //!< Pointer to the base address of the Avalon Fifo

    uint32_t _data[MAX_FIFO_BUFFER_SIZE]; //!< Buffer to keep a local copy of the fifo data
    uint32_t _rd; //!< read pointer
    uint32_t _wr; //!< write pointer
    bool     _full; //!< Indicates that the ring buffer is full
} Fifo;
#else // ENABLE_AUX_RING_BUFFER
//--------------------------------------------------------------------------
//!
//! @brief Fifo - Hardware Fifo
//!
//--------------------------------------------------------------------------
typedef struct
{
    unsigned int _base_addr_csr; //!< Pointer to the base address of the Avalon Fifo CSR
    unsigned int _base_addr_fifo; //!< Pointer to the base address of the Avalon Fifo
} Fifo;
#endif // ENABLE_AUX_RING_BUFFER
#define MAX_AUX_MESSAGE_BUFFER_SIZE (128) //!< Allow this to be the maximum buffered message size
//--------------------------------------------------------------------------
//!
//! @brief AuxDecoderInstance - Structure to hold an instance of the Decoder
//!
//--------------------------------------------------------------------------
typedef struct
{
    bool            _is_sink; //!< Indicates that this is a sink instance
    uint32_t        _aux_addr; //!< The next aux address
    eDecoderStates  _state; //!< Current state of the decoder
    uint32_t        _time_stamp_old; //!< Last timestamp
    bool            _last_req_native; //!< Indicates that we are processing a native command
    Fifo           _fifo; //!< Hardware Fifo Access
    char           _message_buffer[MAX_AUX_MESSAGE_BUFFER_SIZE]; //!< Buffer to store the message until ready to display
} AuxDecoderInstance;

//----------------------------------------------------------------------
//!
//! @brief dp_dump_aux_debug_init - Initialize an instance of a decoder
//!
//! @param[in] pInstance - AuxDecoderInstance* - pointer to the AuxDecoderInstance
//! @param[in] base_addr_csr - unsigned int - The offset to the Avalon Fifo CSR registers
//! @param[in] base_addr_fifo - unsigned int - The offset to the Avalon Fifo registers
//! @param[in] is_sink - bool - True if this is a Sink decoder
//!
//! @return int - No zero indicates error
//!
//----------------------------------------------------------------------
int dp_dump_aux_debug_init(AuxDecoderInstance *pInstance, unsigned int base_addr_csr, unsigned int base_addr_fifo, const bool is_sink );

//----------------------------------------------------------------------
//!
//! @brief dp_dump_aux_debug - Read and display decoder data
//!
//! @param[in] pInstance - AuxDecoderInstance* - pointer to the AuxDecoderInstance
//!
//----------------------------------------------------------------------
void dp_dump_aux_debug(AuxDecoderInstance *pInstance);
#endif /* (BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX) || (BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX) */