/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "HapiItem.h"
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <iomanip>

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

static std::recursive_mutex _activeHapiItemsCS;

// custom specialization of std::hash can be injected in namespace std
template<>
struct std::hash<Hapi::HapiItemIdentifier>
{
    std::size_t operator()(const Hapi::HapiItemIdentifier& itemID) const noexcept
    {
        if (itemID._driverFrameworkType == Hapi::DriverFrameworkType::OCS)
            return (std::size_t)itemID._ocsID._ocsCapabilityID;
        else if (itemID._driverFrameworkType == Hapi::DriverFrameworkType::UIO)
        {
            std::string stringToHash(itemID._uioID._uioName);
            return std::hash<std::string>{}(stringToHash);
        }
        else if (itemID._driverFrameworkType == Hapi::DriverFrameworkType::DFL)
        {
            std::string stringToHash(itemID._dflID._UUID);
            return std::hash<std::string>{}(stringToHash);
        }
        else
            return 0;
    }
};

// All active HAPI items, when the key is the driver ID
static std::unordered_map<Hapi::HapiItemIdentifier, std::vector<Hapi::HapiItemBase*>> _activeHapiItems;

namespace Hapi
{
    HapiItemBase::HapiItemBase(const HapiItemIdentifierList& driverItemIDs,
                               const HapiItemIdentifierList& coreItemIDs,
                               size_t addressOffsetFromParent)
    :   _driverItemIDs(driverItemIDs)
    ,   _ipCoreItemIDs(coreItemIDs)
    ,   _encapsulatedRegisterOffset(addressOffsetFromParent / 4)
    {
#ifdef DEBUG
        for(std::size_t i = 0; i < _driverItemIDs._num_ids; ++i)
        {
            const auto& driverID = _driverItemIDs._ids[i];
            bool foundMatch = false;

            for(std::size_t j = 0; j < _ipCoreItemIDs._num_ids; ++j)
            {
                if (driverID._driverFrameworkType == _ipCoreItemIDs._ids[j]._driverFrameworkType)
                {
                    foundMatch = true;
                    break;
                }
            }
            
            if(not foundMatch)
            {
                std::cerr << __FILE__ << ":" << __LINE__ << ": error: No matching IpCore ID found for DriverFrameworkType "
                          << static_cast<std::underlying_type<DriverFrameworkType>::type>(driverID._driverFrameworkType) << "\n";
            }
        }
#endif /* DEBUG */
    }

    HapiItemBase::HapiItemBase()
    {
    }

    HapiItemBase::~HapiItemBase()
    {
        RemoveFromActiveHapiItems();
    }

    bool HapiItemBase::Initialize(std::weak_ptr<HapiCore> wpHapiCore)
    {
        _wpHapiCore = wpHapiCore;
        return true;
    }

    std::shared_ptr<HapiCore> HapiItemBase::GetHapiCore()
    {
        std::shared_ptr<HapiCore> spHapiCore(_wpHapiCore);
        return spHapiCore;
    }

    uint32_t HapiItemBase::ReadRegister(uint32_t registerIndex)
    {
        uint32_t value = 0;
        if (_spTunnel)
        {
            value = _spTunnel->ReadRegister(registerIndex + _encapsulatedRegisterOffset);
            LogRead(registerIndex, value);
        }
        return value;
    }

    void HapiItemBase::WriteRegister(uint32_t registerIndex, uint32_t value)
    {
        if (_spTunnel)
        {
            _spTunnel->WriteRegister(registerIndex + _encapsulatedRegisterOffset, value);
            LogWrite(registerIndex, value);
        }
    }

    uint32_t HapiReadRegister(void* hapi_item, uint32_t registerIndex)
    {
        HapiItemBase* pHapiItemBase = (HapiItemBase*)hapi_item;
        uint32_t value =  pHapiItemBase->ReadRegister(registerIndex);
        return value;
    }

    void HapiWriteRegister(void* hapi_item, uint32_t registerIndex, uint32_t registerValue)
    {
        HapiItemBase* pHapiItemBase = (HapiItemBase*)hapi_item;
        pHapiItemBase->WriteRegister(registerIndex, registerValue);
    }

    // Register individual register address for logging
    void HapiItemBase::LogRegisterWrites(uint32_t registerAddress)
    {
        _logIndividualRegisterAddresses.insert(registerAddress);
    }

    // Enable register read/write logging
    void HapiItemBase::LogRegisterWrites(bool state)
    {
        _logRegisterWrites = state;
    }

    void HapiItemBase::DumpRegisterReads()
    {
        std::cout.fill('0');
        for (auto& entry: _logReadValues)
        {
            std::cout << "0x" << std::hex << std::setw(8) << entry.first;
            std::cout << ",0x" << std::hex << std::setw(8) << entry.second;
            std::cout << std::endl;
        }
    }

    void HapiItemBase::LogRead(uint32_t registerIndex, uint32_t value)
    {
        registerIndex += _encapsulatedRegisterOffset;

        if (_logRegisterWrites or (_logIndividualRegisterAddresses.find(registerIndex) != _logIndividualRegisterAddresses.end()))
        {
            size_t regAddress = _registerPhysicalAddress + (registerIndex * 4);

            if (_logReadValues.find(regAddress) == _logReadValues.end())
                _logReadValues[regAddress] = value;
        }
    }

    void HapiItemBase::LogWrite(uint32_t registerIndex, uint32_t value)
    {
        registerIndex += _encapsulatedRegisterOffset;
        if (_logRegisterWrites or (_logIndividualRegisterAddresses.find(registerIndex) != _logIndividualRegisterAddresses.end()))
        {
            const auto flags{std::cout.flags()};

            size_t regAddress = _registerPhysicalAddress + (registerIndex * 4);
            std::cout.fill('0');
            std::cout << "0x" << std::hex << std::setw(8) << regAddress;
            std::cout << ",0x" << std::hex << std::setw(8) << value;
            std::cout << std::endl;

            _logReadValues[regAddress] = value;

            std::cout.flags(flags);
        }
    }

    void HapiItemBase::AddToActiveHapiItems(const HapiItemIdentifier& driverItemId)
    {
        std::lock_guard lock(_activeHapiItemsCS);

        auto i = _activeHapiItems.find(driverItemId);
        if (i != _activeHapiItems.end())
        {
            std::vector<HapiItemBase*>* currentItemsOfThisType = &i->second;
            currentItemsOfThisType->push_back(this);
        }
        else
        {
            _activeHapiItems[driverItemId] = std::vector<HapiItemBase*>();
            _activeHapiItems[driverItemId].push_back(this);
        }
    }    

    void HapiItemBase::RemoveFromActiveHapiItems()
    {
        std::lock_guard lock(_activeHapiItemsCS);

        for (auto& itemOfType : _activeHapiItems)
        {
            for (auto& pItem : itemOfType.second)
            {
                if (pItem == this)
                {
                    // Remove from the std::vector
                    auto pos = std::find(itemOfType.second.begin(), itemOfType.second.end(), this);
                    if (pos != itemOfType.second.end())
                        itemOfType.second.erase(pos);

                    // Remove the vector if it's empty
                    if (itemOfType.second.empty())
                    {
                        auto i = _activeHapiItems.find(itemOfType.first);
                        if (i != _activeHapiItems.end())
                            _activeHapiItems.erase(i);
                    }
                    return;
                }
            }
        }
    }

    std::weak_ptr<HapiItemBase> HapiItemBase::GetExistingItem(const DriverFrameworkType& frameworkType, HapiItemBase* pHapiItemBase, std::function<bool(HapiItemBase*)> checkFunction)
    {
        // Have we got one already ?
        std::lock_guard lock(_activeHapiItemsCS);

        if (not pHapiItemBase)
            return {};

        HapiItemIdentifier* driverItemId = pHapiItemBase->GetDriverItemIdentifier(frameworkType);

        if (not driverItemId)
            return {};

        auto i = _activeHapiItems.find(*driverItemId);

        if (i != _activeHapiItems.end())
        {
            std::vector<HapiItemBase*>* currentItemsOfThisType = &i->second;
            for (HapiItemBase* pExistingHapiItem : *currentItemsOfThisType)
            {
                // The core ID also needs to match, and also the
                // register offset (used for encapsulated cores)
                HapiItemIdentifier* ipCoreId = pExistingHapiItem->GetIpCoreItemIdentifier(frameworkType);
                HapiItemIdentifier* ipCoreIdNew = pHapiItemBase->GetIpCoreItemIdentifier(frameworkType);

                if (ipCoreId and ipCoreIdNew)
                {
                    if (((*ipCoreId) == (*ipCoreIdNew)) and
                        (pExistingHapiItem->GetEncapsulatedRegisterOffset() == pHapiItemBase->GetEncapsulatedRegisterOffset()))
                    {
                        if (checkFunction(pExistingHapiItem))
                            return pExistingHapiItem->shared_from_this();
                    }
                }
            }
        }

        return {};
    }

    bool HapiItemSharedMemoryBase::GetVirtualAddress(size_t& virtualAddress, size_t& size)
    {
        return _spTunnel ? _spTunnel->GetVirtualAddress(virtualAddress, size) : false;
    }

} // namespace Hapi