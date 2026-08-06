/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "system.h"
#include "stream_controller_messages.h"
#include "stream_controller_userMessages.h"
#include "intel_vvp_scaler.h"
#include "intel_vvp_vfw.h"
#include "intel_vvp_vfr.h"

typedef struct CoreDlaJobItem
{
    uint32_t                _index;
    bool                    _hasInference;
    bool                    _hasSourceBuffer;
    bool                    _scheduledWithDLA;
    CoreDlaJobPayload       _payload;
    struct CoreDlaJobItem*  _pPreviousJob;
    struct CoreDlaJobItem*  _pNextJob;
} CoreDlaJobItem;


typedef struct StreamController
{
    void*       queueInputBufferUserData;

    bool _exit_thread;

    CoreDlaJobItem* _jobs;
    CoreDlaJobItem* _pNextInferenceRequestJob;
    CoreDlaJobItem* _pFillingImageJob;
    CoreDlaJobItem* _pNextScheduledInferenceRequestJob;
    NiosStatusType  _status;
    uint32_t        _statusLineNumber;
    uint32_t        _commandCounter;
    uint32_t        _dropSourceBuffers;
    uint32_t        _totalNumInferenceRequests;
    uint32_t        _numInferenceRequests;
    uint32_t        _numExecutedJobs;
    uint32_t        _numScheduledInferences;
    uint32_t        _numCompleteInferences;
    uint32_t        _lastReceiveSequenceID;
    uint32_t        _sendSequenceID;
    bool            _initialised;
    bool            _running;
    uint32_t        _numReceivedSourceBuffers;
    volatile uint32_t   _vfwLowResSofIsrCount;
    volatile uint32_t   _vfwLowResEofIsrCount;
    volatile uint32_t   _vfwFullResSofIsrCount;
    volatile uint32_t   _vfwFullResEofIsrCount;
    volatile uint32_t   _vfrFullResIsrCount;
    volatile uint32_t   _msgIsrCount;
    uint32_t        _vfwLowResSofPreviousIsrCount;
    uint32_t        _vfwLowResEofPreviousIsrCount;
    uint32_t        _vfwFullResSofPreviousIsrCount;
    uint32_t        _vfwFullResEofPreviousIsrCount;
    uint32_t        _vfrFullResPreviousIsrCount;
    uint32_t        _msgPreviousIsrCount;
    intel_vvp_scaler_instance _coreDLAInputScalerInstance;
    intel_vvp_vfw_instance _vfwLowResInstance;
    intel_vvp_vfr_instance _vfrFullResInstance;
    intel_vvp_vfw_instance _vfwFullResInstance;

    tVideoCoreConfig _video;
    tModelInputSize _modelInputSize;

    uintptr_t _dlaBaseAddress;

    uint32_t _vfwFullResVideoIdx;
    uint32_t _lastVfwFullResVideoIdx;
    uint32_t _vfrFullResVideoIdx;
    bool _vfwLowResPending;
    bool _vfwFullResPending;
    bool _vfrFullResPending;
    volatile uint32_t   _vfwLowResFpsCount;
    volatile uint32_t   _vfwFullResFpsCount;
    volatile uint32_t   _vfrFullResFpsCount;

    bool        _scaledPathwayInitialised;
    bool        _input_size_changed;

} StreamController;

void StreamController_Start(void);
void StreamController_Reset(void);
void StreamController_InitialiseFullSizePathway(void);
void StreamController_InitialiseScaledPathway(void);
void StreamController_InitializeVfwLowRes(void);
void StreamController_ConfigueInput(void);
void StreamController_ArmVfwLowRes(CoreDlaJobItem* pFillJob);
void StreamController_ArmVfwFullRes(void);
void StreamController_ArmVfrFullRes(void);
void StreamController_QueueInputBuffer(uint32_t inputAddressDDR, void* user_data);
void StreamController_RunEventLoop(void);
void StreamController_WriteToDlaCsr(uint32_t addr, uint32_t data);
void StreamController_InitializeStreamController(
                                          uint32_t sourceBufferSize,
                                          uint32_t dropSourceBuffers,
                                          uint32_t numInferenceRequests);
void StreamController_SetStatus(NiosStatusType statusType, uint32_t lineNumber);
MessageType StreamController_ReceiveMessage(volatile MessageHeader* pReceiveMessage);
bool StreamController_SendMessage(
                           MessageType messageType,
                           void* pPayload,
                           size_t payloadSize);
void StreamController_NewSourceBuffer(void);
void StreamController_ScheduleDlaInference(CoreDlaJobItem* pJob);
void StreamController_NewInferenceRequestReceived(volatile CoreDlaJobPayload* pJob);
void StreamController_InferenceRequestComplete(void);

void StreamController_RegisterISRs(void);

void StreamController_UserMessage(uint32_t userMessageType, void* pPayload, size_t size);
bool StreamController_UserPayloadMessageHandler(volatile uint32_t* pPayload);
void StreamController_VideoCoreConfigPayloadHandler(tVideoCoreConfig* pVideoCoreConfig);
void StreamController_ModelInputSizePayloadHandler(tModelInputSize* pModelInputSize);
