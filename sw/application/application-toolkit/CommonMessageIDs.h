/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <cstdint>
#include "AppToolkitMessageQueue.h"

enum class CommonMessageIDs : uint32_t
{
    // The messages ID's must be unique, which includes AtUtils::QueueMessageIDs
    // value, so start counting after AtUtils::QueueMessageIDs::LAST_ID
    UI_PROCESS_WEB_SOCKET_MESSAGE_CMD = (uint32_t)AtUtils::QueueMessageIDs::LAST_ID,
    UI_PROCESS_BINARY_WEB_SOCKET_MESSAGE_CMD,

    LAST_ID
};