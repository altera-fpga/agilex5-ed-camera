/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <functional>

namespace AtUtils
{
    class IReactorInternal
    {
    public:
        struct AllCallbacks
        {
            std::function<uint32_t()> calculateWaitTime;
            std::function<void()> threadStarted;
            std::function<void()> threadStopped;

            // waitResult is either index + WAIT_OBJECT_0 if an event was triggered, or WAIT_TIMEOUT if a timeout occurred
            // handles is the vector of event handles, index (see above) corresponds to an entry in this vector
            std::function<void(uint32_t waitResult, const std::vector<EventHandle>& handles)> handleEvent;
        };

        virtual bool Start() = 0;
        virtual bool Stop() = 0;
        virtual bool IsRunning() = 0;
        virtual uint32_t GetThreadId() = 0;
        virtual void Freeze(bool freeze) = 0;
        virtual bool IsFrozen() = 0;
        virtual void SetHandles(std::vector<EventHandle> handles) = 0;
        virtual void WaitIfFrozen() = 0;
        virtual bool IsShuttingDown() = 0;
    };

} // namespace AtUtils
