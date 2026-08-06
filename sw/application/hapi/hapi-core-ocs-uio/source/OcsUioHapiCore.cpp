/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "OcsUioHapiCore.h"
#include "IpCapabilityNames.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "HapiOCS.h"
#include "OcsUioHapiItemTunnel.h"

namespace Hapi
{
    static const std::string uioDriverFolder = "/sys/class/uio";
    static constexpr DriverFrameworkType frameworkType = DriverFrameworkType::OCS;

    static bool ParseOcsUioName(const char* name, uint32_t& type, uint32_t& uniqueID, uint32_t& associatedID)
    {
        unsigned int parsedType = 0;
        unsigned int parsedUniqueID = 0;
        unsigned int parsedAssociatedID = 0;
        int consumedChars = 0;

        if (std::sscanf(name, "ocs_%x_%x_%x%n", &parsedType, &parsedUniqueID, &parsedAssociatedID, &consumedChars) != 3)
            return false;

        // Require a full-string match and enforce 16-bit bounds for each field.
        if ((static_cast<size_t>(consumedChars) != std::strlen(name)) ||
            (parsedType > 0xFFFFu) ||
            (parsedUniqueID > 0xFFFFu) ||
            (parsedAssociatedID > 0xFFFFu))
        {
            return false;
        }

        type = parsedType;
        uniqueID = parsedUniqueID;
        associatedID = parsedAssociatedID;
        return true;
    }

    std::shared_ptr<IHapi> IHapiOCS::Create()
    {
        std::shared_ptr<IHapi> spIHapi;

        // Have we got an IHapi for this framework type?
        auto matches = _wpHapiCoreInstances.equal_range(frameworkType);

        for(auto i = matches.first; i != matches.second; ++i)
        {
            std::shared_ptr<OcsUioHapiCore> spOcsHapi = std::dynamic_pointer_cast<OcsUioHapiCore>(i->second.lock());

            if(spOcsHapi)
            {
                spIHapi = spOcsHapi;
            }
        }


        if (!spIHapi)
        {
            spIHapi = std::shared_ptr<IHapi>(new OcsUioHapiCore());
            _wpHapiCoreInstances.insert({frameworkType, spIHapi});
        }

        return spIHapi;
    }

    const uint32_t OcsUioHapiCore::maxOcsUioNameLength = 128;

    OcsUioHapiCore::OcsUioHapiCore()
    :   HapiCore{frameworkType}
    {
        if (not std::filesystem::exists(uioDriverFolder))
            return;

        // Ensure deterministic ordering: uio2 comes before uio10.
        std::vector<std::pair<uint32_t, std::filesystem::path>> orderedUioDevices;
        for (const auto& dir_entry : std::filesystem::directory_iterator(uioDriverFolder))
        {
            const std::string uioDevName = dir_entry.path().filename();
            unsigned int uioIndex = 0;
            int consumedChars = 0;
            if (std::sscanf(uioDevName.c_str(), "uio%u%n", &uioIndex, &consumedChars) == 1 &&
                static_cast<size_t>(consumedChars) == uioDevName.size())
            {
                orderedUioDevices.emplace_back(uioIndex, dir_entry.path());
            }
        }

        std::sort(orderedUioDevices.begin(), orderedUioDevices.end(),
            [](const auto& left, const auto& right)
            {
                return left.first < right.first;
            });

        for (const auto& orderedDevice : orderedUioDevices)
        {
            const std::string uioDevName = orderedDevice.second.filename();
            std::string name = HapiCore::ReadStringFromFile(uioDriverFolder + "/" + uioDevName + "/name");

            uint32_t type = 0;
            uint32_t uniqueID = 0;
            uint32_t associatedID = 0;
            if (!ParseOcsUioName(name.c_str(), type, uniqueID, associatedID))
            {
                // Ignoring UIO device with unexpected name format
                continue;
            }

            // Create an OcsUioHapiItem for this device and add it to the list.
            auto itemPtr = std::make_shared<OcsUioHapiItem>(uioDevName, type, uniqueID, associatedID);
            _ocsUioHapiItems.emplace_back(itemPtr);
        }
    }

    OcsUioHapiCore::~OcsUioHapiCore()
    {
    }

    void OcsUioHapiCore::UnmapAll()
    {
        for (auto& itemPtr : _ocsUioHapiItems)
        {
            if (itemPtr)
            {
                itemPtr->UnmapRegisters();
            }
        }
    }

    bool OcsUioHapiCore::ValidHardware()
    {
        return true;
    }

    OcsUioHapiItemPtr OcsUioHapiCore::FindCapabilityByIndex(HapiItemBase* pHapiItem, uint32_t index)
    {
        if (pHapiItem != nullptr)
        {
            HapiItemIdentifier* itemID = pHapiItem->GetIpCoreItemIdentifier(GetDriverFrameworkType());

            if(itemID)
            {
                FPGA_CAPABILITY capabilityType = itemID->_ocsID._ocsCapabilityID;

                auto spCapability = FindCapabilityByType((uint32_t)capabilityType, index);

                if (spCapability)
                    return spCapability;
            }
        }

        return nullptr;
    }

    OcsUioHapiItemPtr OcsUioHapiCore::FindCapabilityByUniqueID(HapiItemBase* pHapiItem, uint32_t unique_id)
    {
        OcsUioHapiItemPtr itemPtr = nullptr;
        if (pHapiItem != nullptr)
        {
            HapiItemIdentifier* itemID = pHapiItem->GetIpCoreItemIdentifier(GetDriverFrameworkType());

            if(itemID)
            {
                FPGA_CAPABILITY capabilityType = itemID->_ocsID._ocsCapabilityID;

                for(auto& checkItemPtr : _ocsUioHapiItems)
                {
                    if ((checkItemPtr->GetType() == (uint32_t)capabilityType) and
                        (checkItemPtr->GetUniqueID() == unique_id))
                    {
                        itemPtr = checkItemPtr;
                        break;
                    }
                }
            }
        }

        return itemPtr;
    }

    OcsUioHapiItemPtr OcsUioHapiCore::FindCapabilityByAssociatedID(HapiItemBase* pHapiItem, uint32_t associated_id)
    {
        OcsUioHapiItemPtr itemPtr = nullptr;
        if (pHapiItem != nullptr)
        {
            HapiItemIdentifier* itemID = pHapiItem->GetIpCoreItemIdentifier(GetDriverFrameworkType());

            if(itemID)
            {
                FPGA_CAPABILITY capabilityType = itemID->_ocsID._ocsCapabilityID;

                for(auto& checkItemPtr : _ocsUioHapiItems)
                {
                    if ((checkItemPtr->GetType() == (uint32_t)capabilityType) and
                        (checkItemPtr->GetAssociatedID() == associated_id))
                    {
                        itemPtr = checkItemPtr;
                        break;
                    }
                }
            }
        }

        return itemPtr;
    }

    OcsUioHapiItemPtr OcsUioHapiCore::FindCapabilityByType(uint32_t type, uint32_t index)
    {
        OcsUioHapiItemPtr itemPtr = nullptr;
        uint32_t currentIndex = 0;
        for(auto& checkItemPtr : _ocsUioHapiItems)
        {
            if(checkItemPtr->GetType() == type)
            {
                if(currentIndex == index)
                {
                    itemPtr = checkItemPtr;
                    break;
                }
                currentIndex++;
            }
        }

        return itemPtr;
    }

    OcsUioHapiItemPtr OcsUioHapiCore::FindCapabilityByIndex(uint32_t index)
    {
        OcsUioHapiItemPtr itemPtr = nullptr;
        if(index < _ocsUioHapiItems.size())
        {
            auto it = _ocsUioHapiItems.begin();
            std::advance(it, index);
            itemPtr = *it;
        }

        return itemPtr;
    }

    bool OcsUioHapiCore::InitializeHapiItem(HapiItemBase* pHapiItem, const OcsUioHapiItemPtr& spCapability)
    {
        if (not pHapiItem)
            return false;

        std::shared_ptr<HapiCore> p_me = shared_from_this();
        std::weak_ptr<HapiCore> wp_me = p_me;

        // Already initialized?
        if (pHapiItem->_spTunnel)
            return false;

        auto spOcsUioHapiItemTunnel = std::make_shared<OcsUioHapiItemTunnel>();
        pHapiItem->_spTunnel = spOcsUioHapiItemTunnel;

        HapiItemIdentifier* ipCoreItemID = pHapiItem->GetIpCoreItemIdentifier(GetDriverFrameworkType());
        HapiItemIdentifier* driverItemID = pHapiItem->GetDriverItemIdentifier(GetDriverFrameworkType());

        if (!ipCoreItemID or !driverItemID)
        {
            std::cout << "Can't initialize HapiItem with OCS HapiCore.\n";
            std::cout << "No valid OCS ID has been found. Use a HapiCore with the relevant DriverFrameworkType\n"
                         "or change the last template parameter for your HAPI class definition to contain an OCS FPGA_CAPABILITY\n";
            return false;
        }

        if (!spCapability)
            return false;

        ipCoreItemID->_driverFrameworkType = GetDriverFrameworkType();
        ipCoreItemID->_ocsID._capabilityIndex = GetIndexOfCapability(spCapability);
        ipCoreItemID->_ocsID._capabilityAssociatedID = spCapability->GetAssociatedID();
        ipCoreItemID->_ocsID._capabilityUniqueID = spCapability->GetUniqueID();
        spOcsUioHapiItemTunnel->_spOcsUioHapiItem = spCapability;

        // Copy to the driver ID too
        driverItemID->_driverFrameworkType = GetDriverFrameworkType();
        driverItemID->_ocsID._capabilityIndex = ipCoreItemID->_ocsID._capabilityIndex;
        driverItemID->_ocsID._capabilityAssociatedID = ipCoreItemID->_ocsID._capabilityAssociatedID;
        driverItemID->_ocsID._capabilityUniqueID = ipCoreItemID->_ocsID._capabilityUniqueID;

        // If you override Initialize, you can access pHapiItem->_spDetails,
        // dynamic cast it to OcsHapiItemDetails, and get access to the capability
        pHapiItem->Initialize(wp_me);

        // Call the C instance initializer
        if (not pHapiItem->InitializeInstance())
            return false;

        // Cache this item if a request for the same spCapability come in
        pHapiItem->AddToActiveHapiItems(*driverItemID);
        return true;
    }

    bool OcsUioHapiCore::InitializeByIndex(HapiItemBase* pHapiItem, uint32_t index)
    {
        if (pHapiItem == nullptr)
            return false;

        auto spCapability = FindCapabilityByIndex(pHapiItem, index);

        if(spCapability == nullptr)
            return false;

        return InitializeHapiItem(pHapiItem, spCapability);
    }

    bool OcsUioHapiCore::InitializeByUniqueID(HapiItemBase* pHapiItem, uint32_t unique_id)
    {
        if (pHapiItem == nullptr)
            return false;

        auto spCapability = FindCapabilityByUniqueID(pHapiItem, unique_id);

        if(spCapability == nullptr)
            return false;

        return InitializeHapiItem(pHapiItem, spCapability);
    }

    bool OcsUioHapiCore::InitializeByAssociatedID(HapiItemBase* pHapiItem, uint32_t associated_id)
    {
        if (pHapiItem == nullptr)
            return false;

        auto spCapability = FindCapabilityByAssociatedID(pHapiItem, associated_id);

        if(spCapability == nullptr)
            return false;

        return InitializeHapiItem(pHapiItem, spCapability);
    }


    void OcsUioHapiCore::LogDevices()
    {
        std::stringstream ss;
        uint32_t i = 0;

        int n = _ocsUioHapiItems.size();
        ss << "Number of device spCapabilities: " << n << '\n';

        for (auto itemPtr : _ocsUioHapiItems)
        {
            uint32_t type = itemPtr->GetType();
            int32_t unique_id = itemPtr->GetUniqueID();
            uint32_t associated_id = itemPtr->GetAssociatedID();
            std::string friendlyName;
            const auto& capNames = GetCapabilityNames();
            auto pos = capNames.find(type);
            if (pos != capNames.end())
                friendlyName = pos->second;
            else
                friendlyName = "[no name]";

            uint64_t register_address = 0;
            register_address = itemPtr->GetPhysicalAddress();
            if (register_address)
            {

                ss << "[" << i << "]\t";
                ss << " type = 0x" << std::hex << type;
                ss << "\t" << friendlyName;
                ss << ", unique_id = 0x" << std::hex << unique_id;
                ss << ", associated_id = 0x" << std::hex << associated_id;
                ss << ", physicalAddress = 0x" << std::hex << (uint32_t)register_address;
                ss << ", device = /dev/" << itemPtr->GetUioDevName() << "\n";
            }
            else
            {
                ss << "[" << i << "]";
                ss << " type = 0x" << std::hex << type;
                ss << "\t" << friendlyName;
                ss << ", unique_id = 0x" << std::hex << unique_id;
                ss << ", associated_id = 0x" << std::hex << associated_id;
                ss << ", device = /dev/" << itemPtr->GetUioDevName() << "\n";
            }
            i++;
        }

        std::cout << ss.str() << std::flush;
    }

    uint32_t OcsUioHapiCore::GetIndexOfCapability(const OcsUioHapiItemPtr& spCapability)
    {
        uint32_t currentIndex = 0;
        bool found = false;
        for(auto& checkItemPtr : _ocsUioHapiItems)
        {
            if(checkItemPtr == spCapability)
            {
                found = true;
                break;
            }
            currentIndex++;
        }
        if(!found)
        {
            currentIndex = 0;
        }
        return currentIndex;
    }

} // namespace Hapi
