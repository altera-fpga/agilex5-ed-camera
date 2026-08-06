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
#include <memory>
#include <string>
#include <unordered_map>
#include <map>

#ifndef HAPI
#define HAPI
#endif

namespace Hapi
{
    class HapiItemBase;
    class ILogCallback;

    enum class DriverFrameworkType
    {
        OCS, // Omnitek Capability Structure
        UIO, // Userspace Input Output
        DFL, // Device Feature List,
        MMA  // Memory Mapped Access, for use during development stage only
    };

    class IHapi
    {
    public:
        // static std::shared_ptr<IHapi> Create(DriverFrameworkType driverFrameworkType,
        //                                      uint32_t boardIndex = 0,
        //                                      uint32_t fpgaIndex = 0,
        //                                      bool testDmaChannels = false);
        virtual ~IHapi() = 0;

        template<class T, typename... ARGS>
        std::shared_ptr<T> Create(ARGS... args);

        template<class T, typename... ARGS>
        std::shared_ptr<T> CreateByIndex(uint32_t index, ARGS... args);

        template<class T, typename... ARGS>
        std::shared_ptr<T> CreateByUniqueID(uint32_t uniqueID, ARGS... args);

        template<class T, typename... ARGS>
        std::shared_ptr<T> CreateByAssociatedID(uint32_t associatedID, ARGS... args);

        virtual bool ValidHardware() = 0;
        virtual void SetLogCallback(ILogCallback* pLogCallback) = 0;
        virtual void LogDevices() = 0;
        virtual DriverFrameworkType GetDriverFrameworkType() = 0;

    protected:
        virtual bool InitializeByIndex(HapiItemBase* pHapiItem, uint32_t index) = 0;
        virtual bool InitializeByUniqueID(HapiItemBase* pHapiItem, uint32_t uniqueID) = 0;
        virtual bool InitializeByAssociatedID(HapiItemBase* pHapiItem, uint32_t associatedID) = 0;

        //static std::map<DriverFrameworkType, std::weak_ptr<IHapi>> _wpHapiCoreInstances;
        static std::multimap<DriverFrameworkType, std::weak_ptr<IHapi>> _wpHapiCoreInstances;
    };

    using IHapiPtr = std::shared_ptr<IHapi>;

    class ILogCallback
    {
    public:
        virtual void LogMessage(std::string& message) = 0;
    };

} // namespace Hapi