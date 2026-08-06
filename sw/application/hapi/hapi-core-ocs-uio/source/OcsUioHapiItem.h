/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include <list>
#include <memory>

namespace Hapi
{
    class OcsUioHapiItem;
    using OcsUioHapiItemPtr = std::shared_ptr<OcsUioHapiItem>;
    using OcsUioHapiList = std::list<OcsUioHapiItemPtr>;

    class OcsUioHapiItem
    {
    public:
        OcsUioHapiItem(const std::string& uioDevName, uint32_t type, uint32_t uniqueID, uint32_t associatedID);
        virtual ~OcsUioHapiItem();
        OcsUioHapiItem(const OcsUioHapiItem& other) = delete;
        OcsUioHapiItem& operator=(const OcsUioHapiItem& other) = delete;
        OcsUioHapiItem(OcsUioHapiItem&& other) = delete;
        OcsUioHapiItem& operator=(OcsUioHapiItem&& other) = delete;

        bool MapRegisters();
        void UnmapRegisters();

        std::string GetUioDevName() { return _uioDevName; }
        uint32_t GetType() { return _type; }
        uint32_t GetAssociatedID()  { return _associatedID; }
        uint32_t GetUniqueID() { return _uniqueID; } 
        uint32_t GetPhysicalAddress() { return _physicalAddress; }

        uint32_t ReadRegister(uint32_t registerIndex);
        void WriteRegister(uint32_t registerIndex, uint32_t value);
    private:
        std::string _uioDevName;
        uint32_t _type;
        uint32_t _uniqueID;
        uint32_t _associatedID;
        uint32_t _physicalAddress;
        int _fd;
        size_t _size;
        uint32_t _offset;
        uint32_t _maximumRegisterIndex;
        void* _mapPointer;
        uint32_t* _cpuPointer;
    };
}