/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "HapiCore.h"
#include "HapiItem.h"
#include "OcsUioHapiItem.h"
#include <list>

namespace Hapi
{
    class OcsUioHapiItem;
    using OcsUioHapiItemPtr = std::shared_ptr<OcsUioHapiItem>;
    using OcsUioHapiList = std::list<OcsUioHapiItemPtr>;

    class OcsUioHapiCore : public HapiCore
    {
    public:
        OcsUioHapiCore();
        virtual ~OcsUioHapiCore();

        uint32_t GetIndexOfCapability(const OcsUioHapiItemPtr& spCapability);
        void UnmapAll();

    private:
        static const uint32_t maxOcsUioNameLength;
        void LogDevices() override;
        bool ValidHardware() override;
        bool InitializeHapiItem(HapiItemBase* pHapiItem, const OcsUioHapiItemPtr& capability);
        bool InitializeByIndex(HapiItemBase* pHapiItem, uint32_t index) override;
        bool InitializeByUniqueID(HapiItemBase* pHapiItem, uint32_t uniqueId) override;
        bool InitializeByAssociatedID(HapiItemBase* pHapiItem, uint32_t associatedId) override;

        OcsUioHapiItemPtr FindCapabilityByIndex(HapiItemBase* pHapiItem, uint32_t index);
        OcsUioHapiItemPtr FindCapabilityByUniqueID(HapiItemBase* pHapiItem, uint32_t uniqueId);
        OcsUioHapiItemPtr FindCapabilityByAssociatedID(HapiItemBase* pHapiItem, uint32_t associatedId);
        OcsUioHapiItemPtr FindCapabilityByType(uint32_t type, uint32_t index);
        OcsUioHapiItemPtr FindCapabilityByIndex(uint32_t index);

    private:
        OcsUioHapiList _ocsUioHapiItems;
    };
}