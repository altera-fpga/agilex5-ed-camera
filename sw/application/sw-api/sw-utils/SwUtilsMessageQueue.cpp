/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwUtilsMessageQueue.h"

namespace SwUtils
{

    void Message::Add(Message* pNewMessage, uint32_t deltaTime)
    {
        _pQueue->Add(pNewMessage, deltaTime);
    }

    void MessageQueue::Add(CommandQueueCB commandCB, uint32_t deltaTime)
    {
        Add(new SingleCallbackCmd(std::move(commandCB)), deltaTime);
    }
    
    void MessageQueue::Add(AutoRepeatCommandQueueCB commandCB, uint32_t periodMS, uint32_t deltaStartTime)
    {
        Add(new AutoRepeatCmd(std::move(commandCB), periodMS), deltaStartTime);
    }
    
    void MessageQueue::Add(VariableRepeatCommandQueueCB commandCB, uint32_t deltaStartTime)
    {
        Add(new VariableRepeatCmd(std::move(commandCB)), deltaStartTime);
    }

} // namespace SwUtils
