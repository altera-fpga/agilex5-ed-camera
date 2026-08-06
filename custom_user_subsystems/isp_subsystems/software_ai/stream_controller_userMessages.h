/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __STREAM_CONTROLLER_USERMESSAGES_H__
#define __STREAM_CONTROLLER_USERMESSAGES_H__

#include <stdint.h>
#include "stream_controller_messages.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  UserMessageType_VideoCoreConfig,
  UserMessageType_ModelInputSize,
  UserMessageType_Invalid
} UserMessageType;

typedef struct
{
  uint32_t inputQueueStartAddress;
  uint32_t inputQueueBufferOffset;
  uint32_t inputQueueScaledStartAddress;

  uint32_t FQ_FULL_RES_BPS;

  uint32_t FQ_INPUT_VIDEO_RESOLUTION_WIDTH;
  uint32_t FQ_INPUT_VIDEO_RESOLUTION_HEIGHT;
  uint32_t FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH;
  uint32_t FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT;

  uint32_t FQ_DEFAULT_ALIGNMENT;

  uint32_t FQ_FRAME_DELAY;

  uint32_t numInputQueueSlots;
} tVideoCoreConfig;

typedef struct
{
  uint32_t MODEL_INPUT_VIDEO_WIDTH;
  uint32_t MODEL_INPUT_VIDEO_HEIGHT;

  uint32_t MODEL_INPUT_LT_WIDTH;
  uint32_t MODEL_INPUT_LT_HEIGHT;
  uint32_t MODEL_INPUT_LT_OFFSET;
} tModelInputSize;

#ifdef __cplusplus
}
#endif

#endif //__STREAM_CONTROLLER_USERMESSAGES_H__
