/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <thread>
#include <string>
#include "SwUtilsOS.h"

namespace SwUtils
{
    class Thread
    {
    public:
        enum class WorkerThreadPriority : int32_t
        {
            IDLE = 19,			// Only run when the CPU is IDLE, nice Level = 19
            LOWEST = 10,		// Will be running with very low timesclice, nice level = 10
            BELOW_NORMAL = 1,	// Just below normal working, nice level = 1
            NORMAL = 0,			// Normal run level, nice level = 0
            ABOVE_NORMAL = -1,	// Just above normal, nice level = 1
            HIGHEST = -15,		// Will be running with a very high timeslice, nice level = -15
            TIME_CRITICAL = -19,// Will grab as much as possible, nice level = -19, I wouldn't recommend this level
            DPC = -20			// Runs at the same level as kernel DPCs, nice level = -20. I wouldn't recommend this level
        };

        Thread(const char* threadName, WorkerThreadPriority priority = WorkerThreadPriority::NORMAL);
        virtual ~Thread();
        virtual void RunThread() = 0;
        virtual bool StartThread();
        virtual bool StopThread();
        bool IsRunning();
        uint32_t GetThreadId() { return _threadID; }
        EventHandle GetThreadExitedEvent() { return _threadExited; }

    protected:
        std::shared_ptr<std::thread> _spStdThread;
        Event _threadRunning;
        Event _shutdownEvent;
        Event _threadExited;
        uint32_t _threadID = (uint32_t)-1;
        std::string _threadName;

    private:
        void ThreadFunction();
        WorkerThreadPriority _priority;
    };
} // namespace SwUtils