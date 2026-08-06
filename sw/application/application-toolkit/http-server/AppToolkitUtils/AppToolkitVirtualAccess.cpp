/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "AppToolkitVirtualAccess.h"

namespace AtUtils
{

    VirtualAccess::VirtualAccess(uintptr_t phyAddr, uint32_t size)
    :   _physAddr(phyAddr)
    ,   _size(size)
    {
        if ((size < 4) || ((size % 4) != 0))
            return;
        if ((_physAddr % 4) != 0)
            return;

        _platform._fd = open("/dev/mem", O_RDWR | O_SYNC);
        _platform._baseAdjust = phyAddr & 0xfff;
        _platform._mapSize = size + _platform._baseAdjust;
        if (_platform._fd == -1)
        {
            std::cerr << "Failed to open /dev/mem: " << strerror(errno) << std::endl;
            return;
        }

        _platform._pVirtAddr = (uint8_t*)mmap(NULL, _platform._mapSize, PROT_READ | PROT_WRITE,
                                            MAP_SHARED, _platform._fd, phyAddr & 0xfffff000);
        if (_platform._pVirtAddr == MAP_FAILED)
        {
            close(_platform._fd);
            _platform._fd = -1;

            std::ofstream save_state{};
            save_state.copyfmt(std::cerr);
            std::cerr << "Failed to map 0x" << std::hex << std::setw(8) << std::setfill('0')
                        << phyAddr << std::dec << ": " << strerror(errno) << std::endl;
            std::cerr.copyfmt(save_state);

            return;
        }
    }

    VirtualAccess::~VirtualAccess()
    {
        if (_platform._fd != -1)
        {
            // Unmap the memory
            munmap(_platform._pVirtAddr, _platform._mapSize);
            _platform._pVirtAddr = nullptr;

            // Close devmem
            close(_platform._fd);
            _platform._fd = -1;
        }
    }

    volatile uint32_t* VirtualAccess::GetVirtualAddress()
    {
        return (volatile uint32_t*)(_platform._pVirtAddr + _platform._baseAdjust);
    }

    bool VirtualAccess::IsValid()
    {
        return (_platform._fd != -1);
    }

} // namespace AtUtils
