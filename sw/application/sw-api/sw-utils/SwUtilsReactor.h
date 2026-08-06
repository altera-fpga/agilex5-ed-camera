/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "SwUtilsThread.h"
#include "SwUtilsOS.h"

#include "SwUtilsReactorInternal.h"

#include <functional>
#include <vector>
#include <memory>
#include <mutex>

namespace SwUtils
{
    class Reactor : public IReactorInternal, public Thread
    {
    public:
        Reactor( const char* name, Thread::WorkerThreadPriority priority, AllCallbacks allCallbacks )
        : Thread{ name, priority }
        , m_handlesMutex{}
        , m_cb{ std::move(allCallbacks) }
        , m_handles{}
        , m_unfrozenEvent{}
        {
            m_unfrozenEvent.Set();
        }

        bool Start() override
        {
            return StartThread();
        }

        bool Stop() override
        {
            return StopThread();
        }

        bool IsRunning() override
        {
            return Thread::IsRunning();
        }

        uint32_t GetThreadId() override
        {
            return Thread::GetThreadId();
        }

        void Freeze(bool freeze) override
        {
            if (freeze)
            {
                m_unfrozenEvent.Reset();
            }
            else
            {
                m_unfrozenEvent.Set();
            }
        }

        bool IsFrozen() override
        {
            return !m_unfrozenEvent.IsSignalled();
        }

        void SetHandles(std::vector<EventHandle> handles) override
        {
            std::lock_guard lock{m_handlesMutex};
            m_handles = std::move(handles);
        }

        void WaitIfFrozen() override
        {
            EventHandle handles[2];
            handles[0] = _shutdownEvent;
            handles[1] = m_unfrozenEvent;
            OS::Get()->WaitForMultipleObjects(2, handles, false, INFINITE);
        }

        bool IsShuttingDown() override
        {
            return _shutdownEvent.IsSignalled();
        }

    private:
        void RunThread() override
        {
            m_cb.threadStarted();

            _threadRunning.Set();
            ProcessEvents();

            m_cb.threadStopped();
        }

        void ProcessEvents()
        {
            std::vector<EventHandle> handlesWithShutdown{};

            while (!IsShuttingDown())
            {
                WaitIfFrozen();

                uint32_t wait_time = m_cb.calculateWaitTime();

                {
                    std::lock_guard lock{m_handlesMutex};
                    handlesWithShutdown = m_handles;
                    handlesWithShutdown.push_back(_shutdownEvent);
                }

                uint32_t waitResult = OS::Get()->WaitForMultipleObjects(static_cast<int>( handlesWithShutdown.size() ), &handlesWithShutdown[0],
                                                                        false, wait_time);
                m_cb.handleEvent(waitResult, m_handles);
            }
        }

        std::mutex m_handlesMutex;
        AllCallbacks m_cb;
        std::vector<EventHandle> m_handles;
        Event m_unfrozenEvent;
    };

} // namespace SwUtils
