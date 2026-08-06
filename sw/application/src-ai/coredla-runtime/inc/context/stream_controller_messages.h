/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include <stdint.h>

#define STREAM_CONTROLLER_MAX_USER_DATA 0xEC // 256 - 3*4 (header) - 2*4 (user message header)

typedef enum
{
    MessageType_Invalid,
    MessageType_NoOperation,
    MessageType_GetStatus,
    MessageType_Status,
    MessageType_ScheduleItem,
    MessageType_ItemComplete,
    MessageType_Ping,
    MessageType_Pong,
    MessageType_InitializeStreamController,
    MessageType_ManualArmDmaTransfer,
    MessageType_ManualScheduleDlaInference,
    MessageType_WriteCSR,
    MessageType_UserPayload,
    MessageType_UserPayloadResult
} MessageType;

typedef enum
{
    NiosStatusType_OK = 1000,
    NiosStatusType_Error,
    NiosStatusType_BadMessage,
    NiosStatusType_BadMessageSequence,
    NiosStatusType_BadDescriptor,
    NiosStatusType_AsyncTransferFailed,
    NiosStatusType_VfwInputSplitterSwitchFailed,
    NiosStatusType_VfwFullResFailed,
    NiosStatusType_VfrFullResFailed,
    NiosStatusType_VfrLowResFailed,
    NiosStatusType_CoreDLAInputScalerFailed,
    NiosStatusType_VfrOverlayFailed,
    NiosStatusType_OverlayOutputScalerFailed,
    NiosStatusType_FullResScalerFailed,
    NiosStatusType_FullResScalerSwFailed,
    NiosStatusType_InvalidParameter
} NiosStatusType;

typedef struct
{
    uint32_t _messageReadyMagicNumber;
    uint32_t _messageType;
    uint32_t _sequenceID;
    uint32_t _payload;
} MessageHeader;

// Message payloads:

typedef struct
{
    uint32_t _configurationBaseAddressDDR;
    uint32_t _configurationSize;
    uint32_t _inputAddressDDR;
    uint32_t _outputAddressDDR;
} CoreDlaJobPayload;

typedef struct
{
    uint32_t _sourceBufferSize;
    uint32_t _dropSourceBuffers;
    uint32_t _numInferenceRequests;
} InitializeStreamControllerPayload;

typedef struct
{
    NiosStatusType _status;
    uint32_t _statusLineNumber;
    uint32_t _numReceivedSourceBuffers;
    uint32_t _numScheduledInferences;
    uint32_t _numExecutedJobs;
    uint32_t _vfwLowResIsrCount;
    uint32_t _vfwFullResIsrCount;
} StatusMessagePayload;

typedef struct
{
    uint32_t _sourceBufferSize;
    uint32_t _inputAddressDDR;
    uint32_t _fromHPS;
} ManualArmDmaTransferPayload;

typedef struct
{
    uint32_t _configurationBaseAddressDDR;
    uint32_t _configurationSize;
    uint32_t _inputAddressDDR;
} ManualScheduleDlaInferencePayload;

typedef struct
{
    uint32_t _address;
    uint32_t _data;
} MessageType_WriteCSRPayload;

typedef struct
{
    uint32_t _userMessageType;
    uint32_t _userMessageLength;
} UserPayloadHeader;

typedef struct
{
    UserPayloadHeader _userMessageHeader;
    uint8_t _userMessageData[STREAM_CONTROLLER_MAX_USER_DATA];
} UserPayload;
