/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <vector>
#include <stdint.h>

using ByteArray = std::vector<uint8_t>;

enum class StartupPhase
{
    // Use phase one to create/allocate any members, eg shared_ptr's
    ONE,

    // Use phase two to finalise initialisation. If you refer to other
    // objects, they will have been created in their phase one
    TWO
};

enum class ShutdownPhase
{
    // Use phase one to stop/cancel items/threads
    ONE,

    // Use phase two to tear down any shared_ptr's etc
    TWO
};
