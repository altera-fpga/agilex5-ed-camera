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
#include <string>

// ISP cores pick new changes upon writting to the Commit register
// The commit is pending until the start of the next frame.
// This is when the new changes are applied.
// The UpdatePolicy structure allows to specify the exact behaviour
// when new settings are passed to the hardware:
// DeferCommit  - new settings are written to the core but not commited
// Async        - new settings are written to the core and committed
// Sync         - the software busy waits for any previous commit to clear
//                before writting new settings and commiting them
struct UpdatePolicy
{
    bool _waitForPendingCommit = false;
    bool _deferCommit = false;

    static UpdatePolicy Sync(){
        return {true, false};
    }

    static UpdatePolicy Async(){
        return {false, false};
    }

    static UpdatePolicy DeferCommit(){
        return {false, true};
    }
};


class VvpCoreBase
{
public:
    VvpCoreBase(const char* name);
    ~VvpCoreBase();

protected:
    bool UpdateHw(std::function<bool()> update_hw_func, UpdatePolicy policy);
    std::string GetName() const { return _name; }

private:
    bool WaitUntilCommitNoLongerPending();
    virtual bool CommitSettings() = 0;
    virtual bool IsCommitPending() = 0;

    std::string _name;
};
