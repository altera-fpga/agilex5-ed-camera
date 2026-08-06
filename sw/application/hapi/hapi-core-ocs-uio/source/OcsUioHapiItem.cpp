/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <list>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cerrno>
#include "OcsUioHapiItem.h"
#include "HapiCore.h"

namespace Hapi
{
    static const std::string uioDriverFolder = "/sys/class/uio";

    OcsUioHapiItem::OcsUioHapiItem(const std::string& uioDevName, uint32_t type, uint32_t uniqueID, uint32_t associatedID)
        : _uioDevName{uioDevName}
        , _type{type}
        , _uniqueID{uniqueID}
        , _associatedID{associatedID}
        , _physicalAddress{0}
        , _fd{-1}
        , _size{0}
        , _offset{0}
        , _cpuPointer{nullptr}
    {
        _size = HapiCore::ReadValueFromFile(uioDriverFolder + "/" + _uioDevName + "/maps/map0/size");
        _offset = HapiCore::ReadValueFromFile(uioDriverFolder + "/" + _uioDevName + "/maps/map0/offset");
        _physicalAddress = HapiCore::ReadValueFromFile(uioDriverFolder + "/" + _uioDevName + "/maps/map0/addr") + _offset;
        MapRegisters();
    }

    OcsUioHapiItem::~OcsUioHapiItem()
    {
        UnmapRegisters();
    }

    bool OcsUioHapiItem::MapRegisters()
    {
        bool rc = false;
        if(_cpuPointer)
        {
            rc = true;
        }
        else
        {
            std::string devPath = "/dev/" + _uioDevName;
            _fd = open(devPath.c_str(), O_RDWR);
            if (_fd != -1)
            {
                _mapPointer = mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
                if (_mapPointer == MAP_FAILED)
                {
                    _mapPointer = nullptr;
                    _cpuPointer = nullptr;
                    _size = 0;
                    _maximumRegisterIndex = 0;
                    close(_fd);
                    _fd = -1;
                    std::cerr << "Memory map failed for /dev/" << _uioDevName << "!\n";
                }
                else
                {
                    _cpuPointer = static_cast<uint32_t*>(_mapPointer) + _offset / sizeof(uint32_t);
                    _maximumRegisterIndex = (_size-_offset) / sizeof(uint32_t);
                    rc = true;
                }
            }
            else
            {
                std::cerr << "Failed to open /dev/" << _uioDevName << "!\n";
            }
        }
        
        return rc;
    }

    void OcsUioHapiItem::UnmapRegisters()
    {
        if (_mapPointer && _mapPointer != MAP_FAILED)
        {
            munmap(_mapPointer, _size);
            _cpuPointer = nullptr;
            _mapPointer = nullptr;
            _maximumRegisterIndex = 0;
        }
        if (_fd != -1)
        {
            close(_fd);
            _fd = -1;
        }
    }

    uint32_t OcsUioHapiItem::ReadRegister(uint32_t registerIndex)
    {
        uint32_t rc = 0;
        if(MapRegisters())
        {
            if(registerIndex >= _maximumRegisterIndex)
            {
                std::cerr << "Register index " << registerIndex << " out of bounds for /dev/" << _uioDevName << "!\n";
                rc = 0;
            }
            else
            {
                rc = static_cast<uint32_t*>(_mapPointer)[_offset / sizeof(uint32_t) + registerIndex];
            }
        }
        return rc;
    }

    void OcsUioHapiItem::WriteRegister(uint32_t registerIndex, uint32_t value)
    {
        if(MapRegisters())
        {
            if(registerIndex >= _maximumRegisterIndex)
            {
                std::cerr << "Register index " << registerIndex << " out of bounds for /dev/" << _uioDevName << "!\n";
            }
            else
            {
                static_cast<uint32_t*>(_mapPointer)[_offset / sizeof(uint32_t) + registerIndex] = value;
                //#msync(static_cast<uint32_t*>(_mapPointer) + _offset / sizeof(uint32_t) + registerIndex, sizeof(uint32_t), MS_SYNC);
            }
        }
    }
}
