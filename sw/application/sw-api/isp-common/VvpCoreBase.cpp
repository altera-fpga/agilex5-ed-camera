/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <thread>
#include <chrono>
#include <iostream>
#include "VvpCoreBase.h"



VvpCoreBase::VvpCoreBase(const char* name):
    _name{name}
{
}


VvpCoreBase::~VvpCoreBase()
{
}


bool VvpCoreBase::UpdateHw(std::function<bool()> update_hw_func, UpdatePolicy policy)
{
    if(policy._waitForPendingCommit)
        if(WaitUntilCommitNoLongerPending() == false)
            return false;

    bool ret = update_hw_func();

    if(ret)
       if(policy._deferCommit == false)
            ret = CommitSettings();
    
    return ret;
}


bool VvpCoreBase::WaitUntilCommitNoLongerPending()
{
    // Use the fastest supported frame rate of 60fps
    static constexpr auto FRAME_PERIOD = std::chrono::milliseconds(16);
    // Wait enough for a single frame (250ms) at the slowest supported frame rate of 4fps
    static constexpr uint32_t WAIT_FRAMES = 16;

    bool ret = IsCommitPending();

    for(uint32_t i = 0; (ret && (i < WAIT_FRAMES)); i++)
    {
        std::this_thread::sleep_for(FRAME_PERIOD);
        ret = IsCommitPending();
    }

    if(ret)
    {
        std::cerr << "[" << _name << "]: Timeout committing changes\n";
    }
    
    return !ret;
}
