/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include "sys/alt_irq.h"
#include "sys/msw_interrupt.h"
#include "altera_avalon_pio_regs.h"

#include <time.h>
#include "stream_controller.h"
#include "dla_registers.h"
#include "stream_controller_messages.h"
#include "stream_controller_userMessages.h"

#define DLA_LT_REG_VER 0
#define DLA_LT_REG_CONTROL 1
#define DLA_LT_REG_PIX_VAL 2
#define DLA_LT_REG_FRAME_IN_STAT 3
#define DLA_LT_REG_FRAME_OUT_STAT 4
#define DLA_LT_REG_FRAME_ERR_STAT 5
#define DLA_LT_REG_FRAME_ERR_IN_STAT 6
#define DLA_LT_REG_FRAME_ERR_OUT_STAT 7

#define VFR_REG_IRQ_CONTROL (INTEL_VVP_CORE_IRQ_BASE_REG+0)
#define VFR_REG_IRQ_STATUS (INTEL_VVP_CORE_IRQ_BASE_REG+1)
#define VFW_REG_IRQ_CONTROL (INTEL_VVP_CORE_IRQ_BASE_REG+0)
#define VFW_REG_IRQ_STATUS (INTEL_VVP_CORE_IRQ_BASE_REG+1)
#define VFW_EOF_IRQ (1)
#define VFW_SOF_IRQ (2)

#define MESSAGE_READY_MAGIC_NUMBER 0x55225522
#define MESSAGE_QUEUE_SIZE 0x200U
#define COREDLA_BRIDGE 0x40000000

static StreamController _streamController = {};

int stream_controller_main(uintptr_t cpuPointer);

#define NIOS_ISR

#ifdef NIOS_ISR

#undef ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_IRQ
#undef ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_IRQ_INTERRUPT_CONTROLLER_ID
#define ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_IRQ 2
#define ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_IRQ_INTERRUPT_CONTROLLER_ID 0

#undef ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_IRQ
#undef ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_IRQ_INTERRUPT_CONTROLLER_ID
#define ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_IRQ 3
#define ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_IRQ_INTERRUPT_CONTROLLER_ID 0

#undef ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_IRQ
#undef ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_IRQ_INTERRUPT_CONTROLLER_ID
#define ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_IRQ 4
#define ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_IRQ_INTERRUPT_CONTROLLER_ID 0

#undef ISP_AI_SUBSYSTEM_ISP_AI_VFR_OLAY_IRQ
#undef ISP_AI_SUBSYSTEM_ISP_AI_VFR_OLAY_IRQ_INTERRUPT_CONTROLLER_ID
#define ISP_AI_SUBSYSTEM_ISP_AI_VFR_OLAY_IRQ 5
#define ISP_AI_SUBSYSTEM_ISP_AI_VFR_OLAY_IRQ_INTERRUPT_CONTROLLER_ID 0

static void vfwHighRes_isr(void* isr_context);
static void vfrHighRes_isr(void* isr_context);
static void vfwLowRes_isr(void* isr_context);
static void msg_isr(void* isr_context);
#endif

static uint32_t Align(uint32_t x, uint32_t alignment)
{
    return (1 + ((x - 1) / alignment)) * alignment;
}

static uint32_t AlignLine(uint32_t width, uint8_t bps, uint8_t channels, uint16_t alignment)
{
    return Align((width / 8) * bps * channels, alignment);
}

static uint32_t AlignBuffer(uint32_t lineByteSize, uint16_t lines, uint16_t alignment)
{
    return Align(lineByteSize * lines, alignment);
}

int main()
{
    memset(&_streamController, 0, sizeof(StreamController));

    _streamController._exit_thread = false;
    _streamController._video.FQ_INPUT_VIDEO_RESOLUTION_WIDTH = 3840;
    _streamController._video.FQ_INPUT_VIDEO_RESOLUTION_HEIGHT = 2160;
    _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH = 3840;
    _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT = 2160;

    StreamController_Reset();
    StreamController_RegisterISRs();
    StreamController_Start();

    return 0;
}

void StreamController_Start()
{
    // Clear the mailbox memory
    uint8_t* pMailbox = (uint8_t*)(ISP_AI_SUBSYSTEM_ISP_AI_MSG_Q_BASE);
    memset(pMailbox, 0, MESSAGE_QUEUE_SIZE);
    for (int n = 0; n < MESSAGE_QUEUE_SIZE; n++)
    {
        pMailbox[n] = n;
    }

    // Run the main event loop
    StreamController_RunEventLoop();
}

void StreamController_InitialiseFullSizePathway()
{
    _streamController._vfwFullResVideoIdx = 0U;
    _streamController._lastVfwFullResVideoIdx = 0U;
    _streamController._vfrFullResVideoIdx = 0U;

    uint32_t buffer_address = _streamController._video.inputQueueStartAddress;

    int lineStride = AlignLine(_streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH, _streamController._video.FQ_FULL_RES_BPS, 3, _streamController._video.FQ_DEFAULT_ALIGNMENT);
    int bufferStride = AlignBuffer(lineStride, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT, _streamController._video.FQ_DEFAULT_ALIGNMENT);
    (void)bufferStride; // Unused


    // Input Full Res Frame Writer
    {
        int ret;
        ret = intel_vvp_vfw_init(&_streamController._vfwFullResInstance, (intel_vvp_core_base)ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_BASE);

        if (ret != kIntelVvpCoreOk)
        {
            StreamController_SetStatus(NiosStatusType_VfwFullResFailed, __LINE__);
        }
        else
        {
            intel_vvp_vfw_set_base_addr(&_streamController._vfwFullResInstance, buffer_address);
            intel_vvp_vfw_set_num_buffers(&_streamController._vfwFullResInstance, 1);
            intel_vvp_vfw_set_inter_line_offset(&_streamController._vfwFullResInstance, lineStride);
            intel_vvp_vfw_set_inter_buffer_offset(&_streamController._vfwFullResInstance, _streamController._video.inputQueueBufferOffset);

            intel_vvp_vfw_overwrite_broken_fields(&_streamController._vfwFullResInstance, true);

            intel_vvp_vfw_set_run_mode(&_streamController._vfwFullResInstance, kIntelVvpVfwStop);

            intel_vvp_core_set_img_info_interlace(&_streamController._vfwFullResInstance, 0);
            intel_vvp_core_set_img_info_subsampling(&_streamController._vfwFullResInstance, 0x3);
            intel_vvp_core_set_img_info_cositing(&_streamController._vfwFullResInstance, 0);
            intel_vvp_core_set_img_info_colorspace(&_streamController._vfwFullResInstance, 0);

            intel_vvp_core_set_img_info_width(&_streamController._vfwFullResInstance, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH);
            intel_vvp_core_set_img_info_height(&_streamController._vfwFullResInstance, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT);

            intel_vvp_vfw_commit_writes(&_streamController._vfwFullResInstance);
            intel_vvp_vfw_acknowledge_buffer(&_streamController._vfwFullResInstance);
        }
    }

    for (int i = 0; i < _streamController._video.numInputQueueSlots; i++)
    {
        intel_vvp_vfr_set_bufset_width(&_streamController._vfrFullResInstance, i, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH);
        intel_vvp_vfr_set_bufset_height(&_streamController._vfrFullResInstance, i, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT);
    }

    // Output Full Res Frame Reader
    {
        int ret;
        ret = intel_vvp_vfr_init(&_streamController._vfrFullResInstance, (intel_vvp_core_base)ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_BASE);

        if (ret != kIntelVvpCoreOk)
        {
            StreamController_SetStatus(NiosStatusType_VfrFullResFailed, __LINE__);
        }
        else
        {
            intel_vvp_vfr_set_bufset_base_addr(&_streamController._vfrFullResInstance, 0, _streamController._video.inputQueueStartAddress);
            intel_vvp_vfr_set_bufset_inter_buffer_offset(&_streamController._vfrFullResInstance, 0, _streamController._video.inputQueueBufferOffset);
            intel_vvp_vfr_set_bufset_inter_line_offset(&_streamController._vfrFullResInstance, 0, lineStride);
            intel_vvp_vfr_set_bufset_field_count(&_streamController._vfrFullResInstance, 0, 1);

            intel_vvp_vfr_set_bufset_bps(&_streamController._vfrFullResInstance, 0, _streamController._video.FQ_FULL_RES_BPS);
            intel_vvp_vfr_set_bufset_colorspace(&_streamController._vfrFullResInstance, 0, 0);
            intel_vvp_vfr_set_bufset_cositing(&_streamController._vfrFullResInstance, 0, 0);
            intel_vvp_vfr_set_bufset_interlace(&_streamController._vfrFullResInstance, 0, 0);
            intel_vvp_vfr_set_bufset_subsampling(&_streamController._vfrFullResInstance, 0, 0x3);

            intel_vvp_vfr_set_bufset_width(&_streamController._vfrFullResInstance, 0, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH);
            intel_vvp_vfr_set_bufset_height(&_streamController._vfrFullResInstance, 0, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT);

            intel_vvp_core_set_img_info_interlace(&_streamController._vfrFullResInstance, 0);
            intel_vvp_core_set_img_info_subsampling(&_streamController._vfrFullResInstance, 0x3);
            intel_vvp_core_set_img_info_cositing(&_streamController._vfrFullResInstance, 0);
            intel_vvp_core_set_img_info_colorspace(&_streamController._vfrFullResInstance, 0);

            intel_vvp_vfr_set_buffer_mode(&_streamController._vfrFullResInstance, kIntelVvpVfrSingleSet);

            intel_vvp_vfr_set_run_mode(&_streamController._vfrFullResInstance, kIntelVvpVfwStop);

            intel_vvp_vfr_commit_writes(&_streamController._vfrFullResInstance);

            intel_vvp_core_set_img_info_width(&_streamController._vfrFullResInstance, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH);
            intel_vvp_core_set_img_info_height(&_streamController._vfrFullResInstance, _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT);

            intel_vvp_vfr_set_starting_buffer_set(&_streamController._vfrFullResInstance, 0);
            intel_vvp_vfr_set_run_mode(&_streamController._vfrFullResInstance, kIntelVvpVfrFreeRunning);
            intel_vvp_vfr_commit_writes(&_streamController._vfrFullResInstance);
        }
    }
}

void StreamController_InitialiseScaledPathway()
{
    int ret;

    ret = intel_vvp_scaler_init(&_streamController._coreDLAInputScalerInstance, (intel_vvp_core_base)ISP_AI_SUBSYSTEM_ISP_AI_SCALER_DOWN_BASE);

    if (ret != kIntelVvpCoreOk)
    {
        StreamController_SetStatus(NiosStatusType_CoreDLAInputScalerFailed, __LINE__);
    }
    else
    {
        intel_vvp_core_set_img_info_interlace(&_streamController._coreDLAInputScalerInstance, 0);
        intel_vvp_core_set_img_info_subsampling(&_streamController._coreDLAInputScalerInstance, 0x3);
        intel_vvp_core_set_img_info_cositing(&_streamController._coreDLAInputScalerInstance, 0);
        intel_vvp_core_set_img_info_colorspace(&_streamController._coreDLAInputScalerInstance, 0);
        intel_vvp_core_set_img_info_width(&_streamController._coreDLAInputScalerInstance, _streamController._video.FQ_INPUT_VIDEO_RESOLUTION_WIDTH);
        intel_vvp_core_set_img_info_height(&_streamController._coreDLAInputScalerInstance, _streamController._video.FQ_INPUT_VIDEO_RESOLUTION_HEIGHT);

        intel_vvp_scaler_commit_writes(&_streamController._coreDLAInputScalerInstance);
        _streamController._scaledPathwayInitialised = true;
    }
}

void StreamController_InitializeVfwLowRes()
{
    int ret;

    ret = intel_vvp_vfw_init(&_streamController._vfwLowResInstance, (intel_vvp_core_base)ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_BASE);

    if (ret != kIntelVvpCoreOk)
    {
        StreamController_SetStatus(NiosStatusType_VfrLowResFailed, __LINE__);
    }
    else
    {
        intel_vvp_vfw_set_base_addr(&_streamController._vfwLowResInstance, 0x10000000);
        intel_vvp_vfw_set_num_buffers(&_streamController._vfwLowResInstance, 1); // We use address offsets instead of multi-buffers.
        intel_vvp_vfw_set_inter_line_offset(&_streamController._vfwLowResInstance, _streamController._modelInputSize.MODEL_INPUT_LT_WIDTH * 8);
        intel_vvp_vfw_set_inter_buffer_offset(&_streamController._vfwLowResInstance, 0x03000000); // single buffer

        intel_vvp_vfw_overwrite_broken_fields(&_streamController._vfwLowResInstance, true);

        intel_vvp_vfw_set_run_mode(&_streamController._vfwLowResInstance, kIntelVvpVfwStop);

        intel_vvp_core_set_img_info_interlace(&_streamController._vfwLowResInstance, 0);
        intel_vvp_core_set_img_info_subsampling(&_streamController._vfwLowResInstance, 0x3);
        intel_vvp_core_set_img_info_cositing(&_streamController._vfwLowResInstance, 0);
        intel_vvp_core_set_img_info_colorspace(&_streamController._vfwLowResInstance, 0);

        intel_vvp_vfw_commit_writes(&_streamController._vfwLowResInstance);
        intel_vvp_vfw_acknowledge_buffer(&_streamController._vfwLowResInstance);
    }
}

void StreamController_ConfigueInput()
{
    if (_streamController._input_size_changed)
    {
        // CoreDLA scaler
        intel_vvp_scaler_set_output_width(&_streamController._coreDLAInputScalerInstance, _streamController._modelInputSize.MODEL_INPUT_VIDEO_WIDTH);
        intel_vvp_scaler_set_output_height(&_streamController._coreDLAInputScalerInstance, _streamController._modelInputSize.MODEL_INPUT_VIDEO_HEIGHT);

        intel_vvp_scaler_commit_writes(&_streamController._coreDLAInputScalerInstance);

        intel_vvp_core_set_img_info_width(&_streamController._vfwLowResInstance, _streamController._modelInputSize.MODEL_INPUT_LT_WIDTH);
        intel_vvp_core_set_img_info_height(&_streamController._vfwLowResInstance, _streamController._modelInputSize.MODEL_INPUT_LT_HEIGHT);
        intel_vvp_vfw_set_inter_line_offset(&_streamController._vfwLowResInstance, _streamController._modelInputSize.MODEL_INPUT_LT_WIDTH * 8);

        intel_vvp_vfw_commit_writes(&_streamController._vfwLowResInstance);
        intel_vvp_vfw_acknowledge_buffer(&_streamController._vfwLowResInstance);

        IOWR(ISP_AI_SUBSYSTEM_ISP_AI_DLA_LT_BASE, DLA_LT_REG_CONTROL, (_streamController._modelInputSize.MODEL_INPUT_VIDEO_HEIGHT-2U));
        _streamController._input_size_changed = false;
    }
}

void StreamController_ArmVfwLowRes(CoreDlaJobItem* pFillJob)
{
    _streamController._pFillingImageJob = pFillJob;

    intel_vvp_vfw_set_base_addr(&_streamController._vfwLowResInstance, _streamController._pFillingImageJob->_payload._inputAddressDDR + _streamController._modelInputSize.MODEL_INPUT_LT_OFFSET);
    intel_vvp_vfw_set_run_mode(&_streamController._vfwLowResInstance, kIntelVvpVfwSingleShot);
    intel_vvp_vfw_commit_writes(&_streamController._vfwLowResInstance);
    _streamController._vfwLowResPending = true;
}

void StreamController_ArmVfwFullRes()
{
    _streamController._vfwFullResVideoIdx++;
    if (_streamController._vfwFullResVideoIdx >= _streamController._video.numInputQueueSlots)
    {
        _streamController._vfwFullResVideoIdx = 0;
    }

    const uint32_t frameAddressDDR = _streamController._video.inputQueueStartAddress + _streamController._vfwFullResVideoIdx*_streamController._video.inputQueueBufferOffset;

    intel_vvp_vfw_set_base_addr(&_streamController._vfwFullResInstance, frameAddressDDR);
    intel_vvp_vfw_set_run_mode(&_streamController._vfwFullResInstance, kIntelVvpVfwSingleShot);
    intel_vvp_vfw_commit_writes(&_streamController._vfwFullResInstance);
    _streamController._vfwFullResPending = true;
}

void StreamController_ArmVfrFullRes()
{
    if (_streamController._lastVfwFullResVideoIdx >= _streamController._video.FQ_FRAME_DELAY)
    {
        _streamController._vfrFullResVideoIdx = _streamController._lastVfwFullResVideoIdx - _streamController._video.FQ_FRAME_DELAY;
    }
    else
    {
        _streamController._vfrFullResVideoIdx = _streamController._video.numInputQueueSlots + _streamController._lastVfwFullResVideoIdx - _streamController._video.FQ_FRAME_DELAY;
    }

    const uint32_t frameAddressDDR = _streamController._video.inputQueueStartAddress + _streamController._vfrFullResVideoIdx*_streamController._video.inputQueueBufferOffset;

    intel_vvp_vfr_set_bufset_base_addr(&_streamController._vfrFullResInstance, 0, frameAddressDDR);
    intel_vvp_vfr_commit_writes(&_streamController._vfrFullResInstance);
    _streamController._vfrFullResPending = true;
}

void StreamController_RunEventLoop()
{
    volatile MessageHeader* pReceiveMessage = (MessageHeader*)(ISP_AI_SUBSYSTEM_ISP_AI_MSG_Q_BASE);

    while (!_streamController._exit_thread)
    {
#ifndef NIOS_ISR
        if (_streamController._initialised)
        {
            if (_streamController._vfwLowResPending)
            {
                if (intel_vvp_vfw_get_commit_status(&_streamController._vfwLowResInstance) == false)
                {
                    _streamController._vfwLowResPending = false;
                    _streamController._vfwLowResSofIsrCount++;
                    _streamController._vfwLowResEofIsrCount++;
                }
            }
            if (_streamController._vfwFullResPending)
            {
                if (intel_vvp_vfw_get_commit_status(&_streamController._vfwFullResInstance) == false)
                {
                    _streamController._vfwFullResPending = false;
                    _streamController._vfwFullResSofIsrCount++;
                    _streamController._vfwFullResEofIsrCount++;
                }
            }

            if (_streamController._vfrFullResPending)
            {
                if (intel_vvp_vfr_get_commit_status(&_streamController._vfrFullResInstance) == false)
                {
                    _streamController._vfrFullResPending = false;
                    _streamController._vfrFullResIsrCount++;
                }
            }
        }
        if (pReceiveMessage->_messageReadyMagicNumber == MESSAGE_READY_MAGIC_NUMBER)
        {
            _streamController._msgIsrCount++;
        }
#endif

        if (_streamController._vfwFullResSofIsrCount != _streamController._vfwFullResSofPreviousIsrCount)
        {
            StreamController_ArmVfwFullRes();

            _streamController._vfwFullResSofPreviousIsrCount = _streamController._vfwFullResSofIsrCount;
        }

        if (_streamController._vfwFullResEofIsrCount != _streamController._vfwFullResEofPreviousIsrCount)
        {
            _streamController._vfwFullResFpsCount++;
            _streamController._lastVfwFullResVideoIdx = _streamController._vfwFullResVideoIdx;
            if (_streamController._vfwFullResEofIsrCount == _streamController._video.FQ_FRAME_DELAY)
            {
                StreamController_ArmVfrFullRes();
            }

            _streamController._vfwFullResEofPreviousIsrCount = _streamController._vfwFullResEofIsrCount;
        }

        if (_streamController._vfrFullResIsrCount != _streamController._vfrFullResPreviousIsrCount)
        {
            _streamController._vfrFullResFpsCount++;
            StreamController_ArmVfrFullRes();

            _streamController._vfrFullResPreviousIsrCount = _streamController._vfrFullResIsrCount;
        }

        if (_streamController._vfwLowResSofIsrCount != _streamController._vfwLowResSofPreviousIsrCount)
        {
            _streamController._vfwLowResSofPreviousIsrCount = _streamController._vfwLowResSofIsrCount;
        }

        if (_streamController._vfwLowResEofIsrCount != _streamController._vfwLowResEofPreviousIsrCount)
        {
            _streamController._vfwLowResFpsCount++;
            if (_streamController._pFillingImageJob != NULL)
            {
                StreamController_NewSourceBuffer();
            }
            _streamController._vfwLowResEofPreviousIsrCount = _streamController._vfwLowResEofIsrCount;
        }

        if (_streamController._msgIsrCount != _streamController._msgPreviousIsrCount)
        {
            StreamController_ReceiveMessage(pReceiveMessage);
            _streamController._msgPreviousIsrCount = _streamController._msgIsrCount;
        }
    }
}

MessageType StreamController_ReceiveMessage(volatile MessageHeader* pReceiveMessage)
{
    MessageType messageType = pReceiveMessage->_messageType;
    uint32_t sequenceId = pReceiveMessage->_sequenceID;
    _streamController._commandCounter++;

    volatile uint32_t* pPayload = &pReceiveMessage->_payload;

    if (messageType == MessageType_Ping)
    {
        StreamController_SendMessage(MessageType_Pong, NULL, 0);
    }
    else if (messageType == MessageType_GetStatus)
    {
        StatusMessagePayload statusMessagePayload;
        statusMessagePayload._status = _streamController._status;
        statusMessagePayload._statusLineNumber = _streamController._statusLineNumber;
        statusMessagePayload._numReceivedSourceBuffers = _streamController._numReceivedSourceBuffers;
        statusMessagePayload._numScheduledInferences = _streamController._numScheduledInferences;
        statusMessagePayload._numExecutedJobs = _streamController._numExecutedJobs;
        statusMessagePayload._vfwLowResIsrCount = _streamController._vfwLowResEofIsrCount;
        statusMessagePayload._vfwFullResIsrCount = _streamController._vfwFullResEofIsrCount;

        StreamController_SendMessage(MessageType_Status, &statusMessagePayload, sizeof(statusMessagePayload));
    }
    else if (messageType == MessageType_ScheduleItem)
    {
        volatile CoreDlaJobPayload* pCoreDlaJobPayload = (volatile CoreDlaJobPayload*)pPayload;
        StreamController_NewInferenceRequestReceived(pCoreDlaJobPayload);
        StreamController_SendMessage(MessageType_NoOperation, NULL, 0);
    }
    else if (messageType == MessageType_ItemComplete)
    {
        StreamController_InferenceRequestComplete();
        StreamController_SendMessage(MessageType_NoOperation, NULL, 0);
    }
    else if (messageType == MessageType_InitializeStreamController)
    {
        InitializeStreamControllerPayload* pInitializePayload = (InitializeStreamControllerPayload*)pPayload;
        StreamController_InitializeStreamController(
                                         pInitializePayload->_sourceBufferSize,
                                         pInitializePayload->_dropSourceBuffers,
                                         pInitializePayload->_numInferenceRequests);
        StreamController_SendMessage(MessageType_NoOperation, NULL, 0);
    }
    else if (messageType == MessageType_UserPayload)
    {
        UserPayload* pUserPayload = (UserPayload*)pPayload;
        // copy the volatile data to local array
        uint32_t userMessageType = pUserPayload->_userMessageHeader._userMessageType;
        uint32_t userMessageLength = pUserPayload->_userMessageHeader._userMessageLength;
        uint8_t userMessageData[STREAM_CONTROLLER_MAX_USER_DATA];
        for (uint32_t index = 0; index < userMessageLength; index++)
        {
            userMessageData[index] = pUserPayload->_userMessageData[index];
        }
        StreamController_UserMessage(userMessageType, userMessageData, userMessageLength);
    }
    else
    {
        StreamController_SetStatus(NiosStatusType_BadMessage, __LINE__);
    }

    pReceiveMessage->_messageReadyMagicNumber = sequenceId;

    if ((_streamController._lastReceiveSequenceID != 0) && ((_streamController._lastReceiveSequenceID + 1) != sequenceId))
    {
        // If the DLA plugin has restarted, the first message will be InitializeStreamController
        // with a sequence ID of 0
        if ((sequenceId != 0) || (messageType != MessageType_InitializeStreamController))
            StreamController_SetStatus(NiosStatusType_BadMessageSequence, __LINE__);
    }

    _streamController._lastReceiveSequenceID = sequenceId;
    return messageType;
}

bool StreamController_SendMessage(
                        MessageType messageType,
                        void *pPayload,
                        size_t payloadSize)
{
    uintptr_t mailboxSendAddress = ISP_AI_SUBSYSTEM_ISP_AI_MSG_Q_BASE + (MESSAGE_QUEUE_SIZE / 2);
    uint32_t* pMailbox = (uint32_t*)mailboxSendAddress;
    MessageHeader* pSendMessage = (MessageHeader*)(pMailbox);
    void* pPayloadDestination = &pSendMessage->_payload;

    pSendMessage->_messageType = messageType;
    pSendMessage->_sequenceID = _streamController._sendSequenceID;

    if (payloadSize > 0)
    {
        memcpy(pPayloadDestination, pPayload, payloadSize);
    }

    // Signal the message as ready
    pSendMessage->_messageReadyMagicNumber = MESSAGE_READY_MAGIC_NUMBER;

    IOWR_ALTERA_AVALON_PIO_DATA(ISP_AI_SUBSYSTEM_ISP_AI_CPU_ISSUE_IRQ_BASE, 1);
    IOWR_ALTERA_AVALON_PIO_DATA(ISP_AI_SUBSYSTEM_ISP_AI_CPU_ISSUE_IRQ_BASE, 0);

    _streamController._sendSequenceID++;
    return true;
}

// We have received a new source buffer via the msgdma
void StreamController_NewSourceBuffer()
{
    if (_streamController._pFillingImageJob != NULL)
    {
        // Read the response to flush the buffer
        CoreDlaJobItem* pJustFilledJob = _streamController._pFillingImageJob;
        CoreDlaJobItem* pNextFillJob = NULL;
        _streamController._pFillingImageJob = NULL;

        uint32_t bufferSequence = _streamController._numReceivedSourceBuffers;
        (void)bufferSequence;//Unused
        _streamController._numReceivedSourceBuffers++;

        if (_streamController._dropSourceBuffers > 0)
        {
            // If _dropSourceBuffers = 1, we process 1, drop 1 etc
            // if _dropSourceBuffers = 2, we process 1, drop 2, process 1, drop 2 etc
            if (bufferSequence % (_streamController._dropSourceBuffers + 1) != 0)
            {
                // Drop this buffer, capture the next one in its place
                // Do not call ConfigueInput as we have not completed current inference
                StreamController_ArmVfwLowRes(pJustFilledJob);
                return;
            }
        }

        pJustFilledJob->_hasSourceBuffer = true;

        if (!pJustFilledJob->_hasInference)
        {
            // No inference yet, so keep filling the same job
            pNextFillJob = pJustFilledJob;

            // It already has a buffer but we have to
            // consider this as dropped as we will write another
            // in its place
            pNextFillJob->_hasSourceBuffer = false;

        }
        else if (pJustFilledJob->_pNextJob->_hasSourceBuffer)
        {
            // No space in the next job, so keep filling the same job
            pNextFillJob = pJustFilledJob;

            // It already has a buffer but we have to
            // consider this as dropped as we will write another
            // in its place
            pNextFillJob->_hasSourceBuffer = false;
        }
        else
        {
            pNextFillJob = pJustFilledJob->_pNextJob;
        }

        // Re-arm the DMA transfer
        if (pNextFillJob->_hasInference)
        {
            if (_streamController._input_size_changed)
            {
                StreamController_ConfigueInput();
            }
            StreamController_ArmVfwLowRes(pNextFillJob);
        }

        if ((_streamController._pNextScheduledInferenceRequestJob == NULL) && (pJustFilledJob->_hasSourceBuffer))
        {
            StreamController_ScheduleDlaInference(pJustFilledJob);
        }
    }
}

void StreamController_NewInferenceRequestReceived(volatile CoreDlaJobPayload* pJobPayload)
{
    // Once we have received all '_totalNumInferenceRequests' inference requests,
    // we set the state to running and can now capture the input dma's
    _streamController._numInferenceRequests++;
    _streamController._running = (_streamController._numInferenceRequests >= _streamController._totalNumInferenceRequests);

    CoreDlaJobItem* pselfJob = _streamController._pNextInferenceRequestJob;

    // Store the job details and move to the next
    pselfJob->_payload = *pJobPayload;

    pselfJob->_hasInference = true;
    pselfJob->_scheduledWithDLA = false;
    pselfJob->_hasSourceBuffer = false;

    _streamController._pNextInferenceRequestJob = _streamController._pNextInferenceRequestJob->_pNextJob;

    if (_streamController._pFillingImageJob == NULL)
    {
        if (_streamController._input_size_changed)
        {
            StreamController_ConfigueInput();
        }
        StreamController_ArmVfwLowRes(pselfJob);
    }
}

void StreamController_InferenceRequestComplete()
{
    CoreDlaJobItem* pselfJob = _streamController._pNextScheduledInferenceRequestJob;
    if (pselfJob != NULL)
    {
        _streamController._pNextScheduledInferenceRequestJob = NULL;
        pselfJob->_hasInference = false;
        pselfJob->_scheduledWithDLA = false;
        pselfJob->_hasSourceBuffer = false;
        _streamController._numCompleteInferences++;

        if (pselfJob->_pNextJob->_hasSourceBuffer)
        {
            if (!pselfJob->_pNextJob->_scheduledWithDLA)
            {
                StreamController_ScheduleDlaInference(pselfJob->_pNextJob);
            }
        }
    }
}

void StreamController_ScheduleDlaInference(CoreDlaJobItem* pJob)
{
    // The DLA has an input FIFO. By setting the base address register,
    // we add this request to the FIFO
    pJob->_scheduledWithDLA = true;
    _streamController._numScheduledInferences++;

    if (_streamController._pNextScheduledInferenceRequestJob == NULL)
    {
        _streamController._pNextScheduledInferenceRequestJob = pJob;
    }

    CoreDlaJobPayload* pJobPayload = &pJob->_payload;
    StreamController_WriteToDlaCsr(DLA_DMA_CSR_OFFSET_CONFIG_BASE_ADDR, pJobPayload->_configurationBaseAddressDDR);
    StreamController_WriteToDlaCsr(DLA_DMA_CSR_OFFSET_CONFIG_RANGE_MINUS_TWO, pJobPayload->_configurationSize);
    StreamController_WriteToDlaCsr(DLA_DMA_CSR_OFFSET_INPUT_OUTPUT_BASE_ADDR, pJobPayload->_inputAddressDDR);
}

void StreamController_SetStatus(NiosStatusType statusType, uint32_t lineNumber)
{
    _streamController._status = statusType;
    _streamController._statusLineNumber = lineNumber;
}

void StreamController_InitializeStreamController(
                                       uint32_t sourceBufferSize,
                                       uint32_t dropSourceBuffers,
                                       uint32_t numInferenceRequests)
{
    (void)sourceBufferSize; // Unused
    // this is called once when the inference app is run,
    // so acts like a reset
    _streamController._dropSourceBuffers = dropSourceBuffers;
    _streamController._totalNumInferenceRequests = numInferenceRequests;
    _streamController._jobs = malloc(sizeof(CoreDlaJobItem) * _streamController._totalNumInferenceRequests);

    // Reset any previous state
    StreamController_Reset();

    _streamController._initialised = true;
}

void StreamController_Reset()
{
    uint32_t lastIndex = _streamController._totalNumInferenceRequests - 1;

    // Set up the circular job buffers
    for (uint32_t i = 0; i < _streamController._totalNumInferenceRequests; i++)
    {
        _streamController._jobs[i]._index = i;
        _streamController._jobs[i]._hasInference = false;
        _streamController._jobs[i]._hasSourceBuffer = false;
        _streamController._jobs[i]._scheduledWithDLA = false;
        _streamController._jobs[i]._payload._configurationBaseAddressDDR = 0U;
        _streamController._jobs[i]._payload._configurationSize = 0U;
        _streamController._jobs[i]._payload._inputAddressDDR = 0U;
        _streamController._jobs[i]._payload._outputAddressDDR = 0U;

        uint32_t previousIndex = (i == 0) ? lastIndex : i - 1;
        uint32_t nextIndex = (i == lastIndex) ? 0 : i + 1;
        _streamController._jobs[i]._pPreviousJob = &_streamController._jobs[previousIndex];
        _streamController._jobs[i]._pNextJob = &_streamController._jobs[nextIndex];
    }

    _streamController._pNextInferenceRequestJob = &_streamController._jobs[0];
    _streamController._pFillingImageJob = NULL;
    _streamController._pNextScheduledInferenceRequestJob = NULL;
    _streamController._status = NiosStatusType_OK;
    _streamController._statusLineNumber = 0;
    _streamController._commandCounter = 0;
    _streamController._numInferenceRequests = 0;
    _streamController._numExecutedJobs = 0;
    _streamController._numScheduledInferences = 0;
    _streamController._numCompleteInferences = 0;
    _streamController._lastReceiveSequenceID = 0;
    _streamController._sendSequenceID = 0;
    _streamController._initialised = false;
    _streamController._running = false;
    _streamController._vfwLowResSofIsrCount = 0;
    _streamController._vfwLowResEofIsrCount = 0;
    _streamController._vfwFullResSofIsrCount = 0;
    _streamController._vfwFullResEofIsrCount = 0;
    _streamController._vfrFullResIsrCount = 0;
    _streamController._msgIsrCount = 0;
    _streamController._vfwLowResSofPreviousIsrCount = 0;
    _streamController._vfwLowResEofPreviousIsrCount = 0;
    _streamController._vfwFullResSofPreviousIsrCount = 0;
    _streamController._vfwFullResEofPreviousIsrCount = 0;
    _streamController._vfrFullResPreviousIsrCount = 0;
    _streamController._msgPreviousIsrCount = 0;
    _streamController._numReceivedSourceBuffers = 0;
    _streamController._vfwLowResPending = false;
    _streamController._vfwFullResPending = false;
    _streamController._vfrFullResPending = false;
    _streamController._vfwLowResFpsCount = 0;
    _streamController._vfwFullResFpsCount = 0;
}

void StreamController_WriteToDlaCsr(uint32_t addr, uint32_t data)
{
    if (_streamController._dlaBaseAddress != 0U)
    {
        uint32_t* pRegister = (uint32_t*)(_streamController._dlaBaseAddress + addr);
        pRegister[0] = data;
    }
}

#ifdef NIOS_ISR
static void vfwHighRes_isr(void* isr_context)
{
    (void)isr_context;

    // Disable VFW Core interrupts
    INTEL_VVP_VFW_REG_IOWR((&_streamController._vfwFullResInstance), VFW_REG_IRQ_CONTROL, 0);

    // Allow for nested interrupts

    // MOD
    // Allows for nested interrupts using the Enhanced Interrupt API
    // but without requiring the External Interrupt Controller (EIC)
    // and the Vectored Interrupt Controller (VIC)
    alt_niosv_enable_msw_interrupt();

    // Retrieve interrupt status
    uint32_t irq_status = INTEL_VVP_VFW_REG_IORD((&_streamController._vfwFullResInstance), VFW_REG_IRQ_STATUS);

    // Clear the interrupt
    INTEL_VVP_VFW_REG_IOWR((&_streamController._vfwFullResInstance), VFW_REG_IRQ_STATUS, irq_status);

    if (irq_status & VFW_SOF_IRQ)
    {
        _streamController._vfwFullResSofIsrCount++;
    }
    if (irq_status & VFW_EOF_IRQ)
    {
        _streamController._vfwFullResEofIsrCount++;
    }

    // Prevent nested interrupts
    alt_niosv_disable_msw_interrupt();

    // Enable VFW Core interrupts
    INTEL_VVP_VFW_REG_IOWR((&_streamController._vfwFullResInstance), VFW_REG_IRQ_CONTROL, VFW_SOF_IRQ|VFW_EOF_IRQ);
}

static void vfrHighRes_isr(void* isr_context)
{
    (void)isr_context;

    // Disable VFR Core interrupts
    INTEL_VVP_VFR_REG_IOWR((&_streamController._vfrFullResInstance), VFR_REG_IRQ_CONTROL, 0);

    // Allow for nested interrupts

    // MOD
    // Allows for nested interrupts using the Enhanced Interrupt API
    // but without requiring the External Interrupt Controller (EIC)
    // and the Vectored Interrupt Controller (VIC)
    alt_niosv_enable_msw_interrupt();

    _streamController._vfrFullResIsrCount++;

    // Clear the interrupt
    INTEL_VVP_VFR_REG_IOWR((&_streamController._vfrFullResInstance), VFR_REG_IRQ_STATUS, 1);

    // Prevent nested interrupts
    alt_niosv_disable_msw_interrupt();

    // Enable VFR Core interrupts
    INTEL_VVP_VFR_REG_IOWR((&_streamController._vfrFullResInstance), VFR_REG_IRQ_CONTROL, 1);
}

static void vfwLowRes_isr(void* isr_context)
{
    (void)isr_context;

    // Disable VFW Core interrupts
    INTEL_VVP_VFW_REG_IOWR((&_streamController._vfwLowResInstance), VFW_REG_IRQ_CONTROL, 0);

    // Allow for nested interrupts

    // MOD
    // Allows for nested interrupts using the Enhanced Interrupt API
    // but without requiring the External Interrupt Controller (EIC)
    // and the Vectored Interrupt Controller (VIC)
    alt_niosv_enable_msw_interrupt();

    // Retrieve interrupt status
    uint32_t irq_status = INTEL_VVP_VFW_REG_IORD((&_streamController._vfwLowResInstance), VFW_REG_IRQ_STATUS);

    // Clear the interrupt
    INTEL_VVP_VFW_REG_IOWR((&_streamController._vfwLowResInstance), VFW_REG_IRQ_STATUS, irq_status);

    if (irq_status & VFW_SOF_IRQ)
    {
        _streamController._vfwLowResSofIsrCount++;
    }
    if (irq_status & VFW_EOF_IRQ)
    {
        _streamController._vfwLowResEofIsrCount++;
    }

    // Prevent nested interrupts
    alt_niosv_disable_msw_interrupt();

    // Enable VFW Core interrupts
    INTEL_VVP_VFW_REG_IOWR((&_streamController._vfwLowResInstance), VFW_REG_IRQ_CONTROL, VFW_SOF_IRQ|VFW_EOF_IRQ);
}

static void msg_isr(void* isr_context)
{
    (void)isr_context;

    // Disable PIO Core interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(ISP_AI_SUBSYSTEM_ISP_AI_CPU2_IRQ_FOR_CPU_BASE, 0);

    // Allow for nested interrupts

    // MOD
    // Allows for nested interrupts using the Enhanced Interrupt API
    // but without requiring the External Interrupt Controller (EIC)
    // and the Vectored Interrupt Controller (VIC)
    alt_niosv_enable_msw_interrupt();

    _streamController._msgIsrCount++;

    // Clear the interrupt
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(ISP_AI_SUBSYSTEM_ISP_AI_CPU2_IRQ_FOR_CPU_BASE, 1);

    // Prevent nested interrupts
    alt_niosv_disable_msw_interrupt();

    // Enable PIO Core interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(ISP_AI_SUBSYSTEM_ISP_AI_CPU2_IRQ_FOR_CPU_BASE, 1);
}
#endif

void StreamController_RegisterISRs()
{
#ifdef NIOS_ISR
    alt_ic_isr_register(
        ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_IRQ_INTERRUPT_CONTROLLER_ID,
        ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_IRQ,
        vfwHighRes_isr,
        NULL,
        0x0);

    alt_ic_isr_register(
        ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_IRQ_INTERRUPT_CONTROLLER_ID,
        ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_IRQ,
        vfwLowRes_isr,
        NULL,
        0x0);

    alt_ic_isr_register(
        ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_IRQ_INTERRUPT_CONTROLLER_ID,
        ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_IRQ,
        vfrHighRes_isr,
        NULL,
        0x0);

    alt_ic_isr_register(
        ISP_AI_SUBSYSTEM_ISP_AI_CPU2_IRQ_FOR_CPU_IRQ_INTERRUPT_CONTROLLER_ID,
        ISP_AI_SUBSYSTEM_ISP_AI_CPU2_IRQ_FOR_CPU_IRQ,
        msg_isr,
        NULL,
        0x0);

    IOWR(ISP_AI_SUBSYSTEM_ISP_AI_VFW_FRES_BASE, VFW_REG_IRQ_CONTROL, VFW_SOF_IRQ|VFW_EOF_IRQ);
    IOWR(ISP_AI_SUBSYSTEM_ISP_AI_VFW_LRES_BASE, VFW_REG_IRQ_CONTROL, VFW_SOF_IRQ|VFW_EOF_IRQ);
    IOWR(ISP_AI_SUBSYSTEM_ISP_AI_VFR_FRES_BASE, VFR_REG_IRQ_CONTROL, 1);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(ISP_AI_SUBSYSTEM_ISP_AI_CPU2_IRQ_FOR_CPU_BASE, 1);
#endif
}

void StreamController_UserMessage(uint32_t userMessageType, void* pPayload, size_t size)
{
    switch(userMessageType)
    {
    case UserMessageType_VideoCoreConfig:
        {
            tVideoCoreConfig* pVideoCoreConfig = (tVideoCoreConfig*)pPayload;
            StreamController_VideoCoreConfigPayloadHandler(pVideoCoreConfig);
            StreamController_SendMessage(MessageType_NoOperation, NULL, 0);
        }
        break;
    case UserMessageType_ModelInputSize:
        {
            tModelInputSize* pModelInputSize = (tModelInputSize*)pPayload;
            StreamController_ModelInputSizePayloadHandler(pModelInputSize);
            StreamController_SendMessage(MessageType_NoOperation, NULL, 0);
        }
        break;
    default:
        StreamController_SetStatus(NiosStatusType_BadMessage, __LINE__);
        break;
    }
}

void StreamController_VideoCoreConfigPayloadHandler(tVideoCoreConfig* pVideoCoreConfig)
{
    _streamController._video.inputQueueStartAddress = pVideoCoreConfig->inputQueueStartAddress;
    _streamController._video.inputQueueBufferOffset = pVideoCoreConfig->inputQueueBufferOffset;
    _streamController._video.inputQueueScaledStartAddress = pVideoCoreConfig->inputQueueScaledStartAddress;

    _streamController._video.FQ_FULL_RES_BPS = pVideoCoreConfig->FQ_FULL_RES_BPS;
    _streamController._video.FQ_INPUT_VIDEO_RESOLUTION_WIDTH = pVideoCoreConfig->FQ_INPUT_VIDEO_RESOLUTION_WIDTH;
    _streamController._video.FQ_INPUT_VIDEO_RESOLUTION_HEIGHT = pVideoCoreConfig->FQ_INPUT_VIDEO_RESOLUTION_HEIGHT;
    _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH = pVideoCoreConfig->FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH;
    _streamController._video.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT = pVideoCoreConfig->FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT;
    _streamController._video.FQ_DEFAULT_ALIGNMENT = pVideoCoreConfig->FQ_DEFAULT_ALIGNMENT;
    _streamController._video.FQ_FRAME_DELAY = pVideoCoreConfig->FQ_FRAME_DELAY;
    _streamController._video.numInputQueueSlots = pVideoCoreConfig->numInputQueueSlots;

   _streamController._dlaBaseAddress = COREDLA_BRIDGE + AI_SUBSYSTEM_AI_IP_CORE_BASE;

    StreamController_InitialiseFullSizePathway();

    StreamController_ArmVfwFullRes();
}

void StreamController_ModelInputSizePayloadHandler(tModelInputSize* pModelInputSize)
{
    _streamController._modelInputSize.MODEL_INPUT_VIDEO_WIDTH = pModelInputSize->MODEL_INPUT_VIDEO_WIDTH;
    _streamController._modelInputSize.MODEL_INPUT_VIDEO_HEIGHT = pModelInputSize->MODEL_INPUT_VIDEO_HEIGHT;
    _streamController._modelInputSize.MODEL_INPUT_LT_WIDTH = pModelInputSize->MODEL_INPUT_LT_WIDTH;
    _streamController._modelInputSize.MODEL_INPUT_LT_HEIGHT = pModelInputSize->MODEL_INPUT_LT_HEIGHT;
    _streamController._modelInputSize.MODEL_INPUT_LT_OFFSET = pModelInputSize->MODEL_INPUT_LT_OFFSET;
    _streamController._input_size_changed = true;
    if (!_streamController._scaledPathwayInitialised)
    {
        StreamController_InitialiseScaledPathway();
        StreamController_InitializeVfwLowRes();
        StreamController_ConfigueInput();
    }
}
