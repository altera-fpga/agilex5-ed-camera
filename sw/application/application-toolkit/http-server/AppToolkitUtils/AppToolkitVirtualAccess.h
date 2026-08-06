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

namespace AtUtils
{

    class PlatformVirtualAccess
    {
    public:
        uint8_t*    _pVirtAddr = nullptr;
        uint32_t    _mapSize = 0;
        int         _fd = -1;
        uint32_t    _baseAdjust = 0;
    };

    class VirtualAccess
    {
    public:
        VirtualAccess() = delete;
        VirtualAccess(uintptr_t phyAddr, uint32_t size);
        ~VirtualAccess();

        VirtualAccess(const VirtualAccess& other) = delete;
        VirtualAccess& operator=(const VirtualAccess& other) = delete;
        VirtualAccess(VirtualAccess&& other) = delete;
        VirtualAccess& operator=(VirtualAccess&& other) = delete;

        volatile uint32_t* GetVirtualAddress();
        uintptr_t GetPhysicalAddress() const
        {
            return _physAddr;
        }
        uint32_t GetSize() const
        {
            return _size;
        }
        bool IsValid();

    private:
        uintptr_t _physAddr;
        uint32_t _size;
        PlatformVirtualAccess _platform;
    };

} // namespace AtUtils
