/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwUtilsThread.h"
#include <system_error>

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace SwUtils
{

    Thread::Thread(const char* threadName, WorkerThreadPriority priority)
    :   _threadName(threadName)
    ,   _priority(priority)
    {
    }

    Thread::~Thread()
    {
        StopThread();
    }

    bool Thread::StartThread()
    {
        if (_spStdThread)
            return false;

        _spStdThread.reset(new std::thread(&Thread::ThreadFunction, this));
        _threadRunning.Wait();
        return true;
    }

    bool Thread::StopThread()
    {
        if (_spStdThread)
        {
            _shutdownEvent.Set();

            try
            {
                _spStdThread->join();
                _spStdThread.reset();
            }
            catch (const std::system_error& /*e*/)
            {

            }

            return true;
        }
        else
            return false;
    }

    bool Thread::IsRunning()
    {
        return _threadRunning.IsSignalled();
    }

    void Thread::ThreadFunction()
    {
    #ifdef __linux__
        int threadID = syscall(SYS_gettid);
        int result = setpriority(PRIO_PROCESS, threadID, (uint32_t)_priority);
        (void)result;
        _threadID = (uint32_t)threadID;
    #else
        uint32_t priority_value = THREAD_PRIORITY_NORMAL;
        if (_priority == WorkerThreadPriority::LOWEST)
            priority_value = THREAD_PRIORITY_LOWEST;
        else if (_priority == WorkerThreadPriority::BELOW_NORMAL)
            priority_value = THREAD_PRIORITY_BELOW_NORMAL;
        else if (_priority == WorkerThreadPriority::NORMAL)
            priority_value = THREAD_PRIORITY_NORMAL;
        else if (_priority == WorkerThreadPriority::ABOVE_NORMAL)
            priority_value = THREAD_PRIORITY_ABOVE_NORMAL;
        else if (_priority == WorkerThreadPriority::HIGHEST)
            priority_value = THREAD_PRIORITY_HIGHEST;
        ::SetThreadPriority(::GetCurrentThread(), priority_value);
        _threadID = ::GetCurrentThreadId();
    #endif

        _threadRunning.Set();
        RunThread();
        _threadRunning.Reset();
        _threadExited.Set();
    }

} // namespace SwUtils