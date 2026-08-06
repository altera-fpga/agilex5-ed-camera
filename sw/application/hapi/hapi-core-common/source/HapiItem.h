/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <stdint.h>
#include <functional>
#include <set>
#include <atomic>
#include <cstring>
#include <vector>
#include <memory>
#include "IpCapabilityTypes.h"
#include "HapiCore.h"

namespace Hapi
{
    class UioDevice;

    class HapiItemTunnel
    {
    public:
        virtual uint32_t ReadRegister(uint32_t registerIndex) = 0;
        virtual void WriteRegister(uint32_t registerIndex, uint32_t value) = 0;
        virtual bool GetVirtualAddress(size_t& virtualAddress, size_t& addressSpan) { return false; }
    };

    class HapiItemIdentifier
    {
    public:
        static constexpr size_t maxUioNameLength = 63;
        static constexpr size_t guidLength = 36;

        struct OcsID
        {
            // OCS ID fields
            FPGA_CAPABILITY _ocsCapabilityID = FPGA_CAPABILITY::NONE;
            uint32_t		_capabilityIndex = 0;
            int32_t			_capabilityUniqueID = 0;
            uint32_t		_capabilityAssociatedID = 0;

            bool operator==(const OcsID& other) const
            {
                // Only compare OCS cap ID, since the other fields
                // have may not have been set yet
                return (_ocsCapabilityID == other._ocsCapabilityID);          
            }
        };

        struct UioID
        {
            // UIO ID fields
            char        _uioName[maxUioNameLength + 1] = {};
            uint32_t    _uioDeviceIndex = 0;

            bool operator==(const UioID& other) const
            {
                for (size_t i{0}; i < maxUioNameLength; i++)
                {
                    if (_uioName[i] != other._uioName[i])
                        return false;
                }
                return true;
            }            
        };

        struct DflID
        {
            // DFL guid
            char _UUID[guidLength + 1] = {};

            bool operator==(const DflID& other) const
            {
                for (size_t i{0}; i < guidLength; i++)
                {
                    if (_UUID[i] != other._UUID[i])
                        return false;
                }
                return true;
            }
        };

        struct MmaID
        {
            // Used during development only
            size_t _mmaBasePhysicalAddress = 0;
            size_t _mmaAddressSpan = 0;

            bool operator==(const MmaID& other) const
            {
                return ((_mmaBasePhysicalAddress == other._mmaBasePhysicalAddress) and
                        (_mmaAddressSpan == other._mmaAddressSpan) and
                        (_mmaAddressSpan != 0));
            }            
        };
    
        constexpr HapiItemIdentifier()
        {
        }

        constexpr HapiItemIdentifier(const char* idString)
        :   _driverFrameworkType(DriverFrameworkType::DFL)
        {
            size_t inputLen = std::strlen(idString);
            if (inputLen == 36)
            {
                for (size_t i{0}; (i < 36) and (i < inputLen); i++)
                    _dflID._UUID[i] = idString[i];
            }
            else
            {
                _driverFrameworkType = DriverFrameworkType::UIO;
                for (size_t i{0}; (i < 63) and (i < inputLen); i++)
                    _uioID._uioName[i] = idString[i];
            }
        }

        constexpr HapiItemIdentifier(FPGA_CAPABILITY ocsCapabilityID)
        :   _driverFrameworkType(DriverFrameworkType::OCS)
        {
            _ocsID._ocsCapabilityID = ocsCapabilityID;
        }

        constexpr HapiItemIdentifier(size_t basePhysicalAddress, size_t addressSpan)
        :   _driverFrameworkType(DriverFrameworkType::MMA)
        {
            _mmaID._mmaBasePhysicalAddress = basePhysicalAddress;
            _mmaID._mmaAddressSpan = addressSpan;
        }

        bool operator==(const HapiItemIdentifier& other) const
        {
            if (_driverFrameworkType == DriverFrameworkType::OCS)
            {
                return (_ocsID == other._ocsID);
            }
            else if (_driverFrameworkType == DriverFrameworkType::UIO)
            {
                return (_uioID == other._uioID);
            }
            else if (_driverFrameworkType == DriverFrameworkType::DFL)
            {
                return (_dflID == other._dflID);
            }
            else if (_driverFrameworkType == DriverFrameworkType::MMA)
            {
                return (_mmaID == other._mmaID);
            }
            else
            {
                return false;
            }
        }

        DriverFrameworkType _driverFrameworkType = DriverFrameworkType::DFL;

        // Depending on _driverFrameworkType, one of these is valid:
        OcsID   _ocsID;
        UioID   _uioID;
        DflID   _dflID;
        MmaID   _mmaID;

        static constexpr std::size_t _NUM_FRAMEWORKS = 4;
    };

    struct HapiItemIdentifierList
    {
        constexpr HapiItemIdentifierList(std::initializer_list<HapiItemIdentifier> item_ids):
            _ids{}, _num_ids{std::min(HapiItemIdentifier::_NUM_FRAMEWORKS, item_ids.size())}
        {
            for(std::size_t i = 0; i < _num_ids; ++i)
                _ids[i] = *(item_ids.begin() + i);
        }        

        HapiItemIdentifier _ids[HapiItemIdentifier::_NUM_FRAMEWORKS];
        std::size_t _num_ids;
    }; 

    class HapiItemBase : public std::enable_shared_from_this<HapiItemBase>
    {
        friend class IHapi;
        friend class HapiCore;
        friend class OcsHapiCore;
        friend class OcsUioHapiCore;
        friend class UioHapiCore;
        friend class MmaHapiCore;
        friend class DflHapiCore;

    public:
        HapiItemBase();

        HapiItemBase(const HapiItemIdentifierList& driverItemIDs,
                     const HapiItemIdentifierList& coreItemIDs,
                     size_t addressOffsetFromParent = 0);

        virtual ~HapiItemBase();

        // Usually the Driver ID is the same as the IP Core ID, but if an IP core
        // has another core encapsulated at a fixed offset in its register map
        // (eg HDMI Tx may have an I2C 'sub' core at a fixed offset), then
        // The Driver ID lets you know which driver instance this HAPI item is
        // using (say HDMITxI2C or whatever), and the IpCore ID (say HDMITx)
        // is the ID that is used to find the register map. You would
        // construct your HapiItem with the IpCore ID and a addressOffsetFromParent,
        // The offset should be defined in the header for the IpCore driver
        HapiItemIdentifier* GetIpCoreItemIdentifier(const DriverFrameworkType driverFrameworkType)
        {
            HapiItemIdentifier* pId{nullptr};

            for(std::size_t i = 0; i < _ipCoreItemIDs._num_ids; ++i)
                if(_ipCoreItemIDs._ids[i]._driverFrameworkType == driverFrameworkType)
                    pId = &(_ipCoreItemIDs._ids[i]);

            return pId;
        }

        HapiItemIdentifier* GetDriverItemIdentifier(const DriverFrameworkType driverFrameworkType)
        {
            HapiItemIdentifier* pId{nullptr};

            for(std::size_t i = 0; i < _driverItemIDs._num_ids; ++i)
                if(_driverItemIDs._ids[i]._driverFrameworkType == driverFrameworkType)
                    pId = &(_driverItemIDs._ids[i]);

            return pId;
        }

        size_t GetEncapsulatedRegisterOffset() { return _encapsulatedRegisterOffset; }

        // You generally won't override Initialize but may want to if you want the object to be
        // created without a capability for example
        virtual bool Initialize(std::weak_ptr<HapiCore> wpHapiCore);

        // You must override this to call the C instance initializer function
        virtual bool InitializeInstance() = 0;

        uint32_t ReadRegister(uint32_t registerIndex);
        void WriteRegister(uint32_t registerIndex, uint32_t value);
        std::shared_ptr<HapiCore> GetHapiCore();
        void LogRegisterWrites(bool state);
        void LogRegisterWrites(uint32_t registerAddress);
        void DumpRegisterReads();

    protected:
        void LogRead(uint32_t registerIndex, uint32_t value);
        void LogWrite(uint32_t registerIndex, uint32_t value);

        std::weak_ptr<HapiCore>		_wpHapiCore;
        size_t					    _registerPhysicalAddress = 0;
        bool						_logRegisterWrites = false;
        std::set<uint32_t>			_logIndividualRegisterAddresses;
        std::unordered_map<uint32_t, uint32_t> _logReadValues;
        HapiItemIdentifierList          _driverItemIDs{};
        HapiItemIdentifierList          _ipCoreItemIDs{};        
        size_t                      _encapsulatedRegisterOffset = 0;
        std::shared_ptr<HapiItemTunnel> _spTunnel; // eg OcsHapiItemTunnel

    private:
        void AddToActiveHapiItems(const HapiItemIdentifier& driverItemId);
        void RemoveFromActiveHapiItems();
        static std::weak_ptr<HapiItemBase> GetExistingItem(const DriverFrameworkType& frameworkType, HapiItemBase* pCheckItem, std::function<bool(HapiItemBase*)> checkFunction);
    };
   

    template <typename INIT_RETURN_TYPE, class INSTANCE, INIT_RETURN_TYPE (*INIT_FUNCTION)(INSTANCE*, void*), HapiItemIdentifierList ITEM_IDS>
    class HapiItem : public HapiItemBase
    {
    public:
        // The driver and IP core ID's are typically the same
        HapiItem()
        :   HapiItemBase(ITEM_IDS, ITEM_IDS)
        ,   _initFunction(INIT_FUNCTION)
        {
        }

        // For an IP core that is embedded in the address space of another.
        // parentIpCoreID is used to identify the base register map, and can be
        // a different ID to the ID of this item. e.g. parentIpCoreID could
        // be the ID for HDMI 2.0 Tx, and this ID is I2C. Rather that finding
        // the register map for this ID, the parentIpCoreID is found and the
        // addressOffsetFromParent is added to read/writes
        // HapiItem(HapiItemIdentifier parentIpCoreID, size_t addressOffsetFromParent)
        // :   HapiItemBase(ITEM_ID, parentIpCoreID, addressOffsetFromParent)
        HapiItem(HapiItemIdentifierList parentIpCoreIDs, size_t addressOffsetFromParent)
        :   HapiItemBase(ITEM_IDS, parentIpCoreIDs, addressOffsetFromParent)
        ,   _initFunction(INIT_FUNCTION)
        {
        }

        bool InitializeInstance() override
        {
            // Select the overloaded CallInitializeFunction by the
            // return type of the init function
            return CallInitializeFunction((INIT_RETURN_TYPE*)nullptr);
        }

        INSTANCE* GetInstance() { const HapiItem* const a = this; return (a != nullptr) ? &_instance : nullptr; }

    protected:
        INSTANCE _instance;
    
    private:
        // CallInitializeFunction are overloaded functions to handle the driver init functions
        // that can have different return types
        bool CallInitializeFunction(void*)
        {
            if (!_initFunction)
                return true;

            _initFunction(&this->_instance, this);
            return true;
        }

        bool CallInitializeFunction(uint8_t*)
        {
            if (!_initFunction)
                return true;

            uint8_t r = _initFunction(&this->_instance, this);
            return (r == 0);
        }

        bool CallInitializeFunction(int32_t*)
        {
            if (!_initFunction)
                return true;

            int32_t r = _initFunction(&this->_instance, this);
            return (r == 0);
        }

        bool CallInitializeFunction(bool*)
        {
            if (!_initFunction)
                return true;

            return _initFunction(&this->_instance, this);
        }

        INIT_RETURN_TYPE (*_initFunction)(INSTANCE*, void*) = nullptr;
    };   

    class HapiItemSharedMemoryBase : public HapiItemBase
    {
    public:
        // The driver and IP core ID's are typically the same
        HapiItemSharedMemoryBase(HapiItemIdentifierList hapiItemIDs)
        :   HapiItemBase(hapiItemIDs, hapiItemIDs, 0)
        {
        }

        // For an IP core that is embedded in the address space of another.
        // parentIpCoreID is used to identify the base register map, and can be
        // a different ID to the ID of this item. e.g. parentIpCoreID could
        // be the ID for HDMI 2.0 Tx, and this ID is I2C. Rather that finding
        // the register map for this ID, the parentIpCoreID is found and the
        // addressOffsetFromParent is added to read/writes
        HapiItemSharedMemoryBase(HapiItemIdentifierList hapiItemIDs,
                                 HapiItemIdentifierList parentIpCoreIDs,
                                 size_t addressOffsetFromParent)
        :   HapiItemBase(hapiItemIDs, parentIpCoreIDs, addressOffsetFromParent)
        {
        }

        bool InitializeInstance() override
        {
            // No instance to initialize for shared memory items
            return true;
        }

        bool GetVirtualAddress(size_t& pVirtualAddress, size_t& size);
    };

    template <HapiItemIdentifierList ITEM_IDS>
    class HapiItemSharedMemory : public HapiItemSharedMemoryBase
    {
    public:
        // The driver and IP core ID's are typically the same
        HapiItemSharedMemory()
        :   HapiItemSharedMemoryBase(ITEM_IDS)
        {
        }

        // For an IP core that is embedded in the address space of another.
        // parentIpCoreID is used to identify the base register map, and can be
        // a different ID to the ID of this item. e.g. parentIpCoreID could
        // be the ID for HDMI 2.0 Tx, and this ID is I2C. Rather that finding
        // the register map for this ID, the parentIpCoreID is found and the
        // addressOffsetFromParent is added to read/writes
        HapiItemSharedMemory(HapiItemIdentifierList parentIpCoreIDs, size_t addressOffsetFromParent)
        :   HapiItemSharedMemoryBase(ITEM_IDS, parentIpCoreIDs, addressOffsetFromParent)
        {
        }
    };

    // The class T will be a base class of the implementation which will
    // also be derived from HapiItemBase. Coding the classes this way
    // means the I* interfaces don't need to depened on the members of HapiItemBase
    template<class T, typename... ARGS>
    std::shared_ptr<T> IHapi::CreateByIndex(uint32_t index, ARGS... args)
    {
        std::shared_ptr<T> spResult(new T(args...));
        HapiItemBase* pHapiItemBase = dynamic_cast<HapiItemBase*>(spResult.get());

        bool OcsItem = (GetDriverFrameworkType() == DriverFrameworkType::OCS);

        // Have we got one already ?
        std::function<bool(HapiItemBase*)> checkFunction;

        if (OcsItem)
        {
            checkFunction = [index] (HapiItemBase* pItem)
            {
                const auto& ipCoreItemIDs = pItem->_ipCoreItemIDs;

                for(std::size_t i = 0; i < ipCoreItemIDs._num_ids; ++i)
                {
                    if (ipCoreItemIDs._ids[i]._driverFrameworkType == DriverFrameworkType::OCS)
                        // Check that the capability index of the core ID item matches,
                        // and the driver ID + offset match
                        return (ipCoreItemIDs._ids[i]._ocsID._capabilityIndex == index);
                }

                return false;                
            };
        }
        else
        {
            // UIO item
            checkFunction = [index] (HapiItemBase* pItem)
            {
                const auto& ipCoreItemIDs = pItem->_ipCoreItemIDs._ids;
                const std::size_t num_ids = pItem->_ipCoreItemIDs._num_ids;                

                for(std::size_t i = 0; i < num_ids; ++i)
                    if(ipCoreItemIDs[i]._driverFrameworkType == DriverFrameworkType::UIO)
                        return ipCoreItemIDs[i]._uioID._uioDeviceIndex == index;

                return false;
            };
        }

        std::shared_ptr<HapiItemBase> spExistingItem(HapiItemBase::GetExistingItem(GetDriverFrameworkType(), pHapiItemBase, checkFunction).lock());

        if (spExistingItem)
            return std::dynamic_pointer_cast<T>(spExistingItem);

        if (!InitializeByIndex(pHapiItemBase, index))
            spResult.reset();

        return spResult;
    }

    template<class T, typename... ARGS>
    std::shared_ptr<T> IHapi::Create(ARGS... args)
    {
        return CreateByIndex<T>(0, args...);
    }

    template<class T, typename... ARGS>
    std::shared_ptr<T> IHapi::CreateByUniqueID(uint32_t uniqueID, ARGS... args)
    {
        std::shared_ptr<T> spResult(new T(args...));
        HapiItemBase* pHapiItemBase = dynamic_cast<HapiItemBase*>(spResult.get());

        bool OcsItem = (GetDriverFrameworkType() == DriverFrameworkType::OCS);

        // uniqueID only supported by OCS
        if (not OcsItem)
            return {};

        // Have we got one already ?
        auto checkFunction = [uniqueID] (HapiItemBase* pItem)
        {
            const auto& ipCoreItemIDs = pItem->_ipCoreItemIDs._ids;
            const std::size_t num_ids = pItem->_ipCoreItemIDs._num_ids;

            for(std::size_t i = 0; i < num_ids; ++i)
                if(ipCoreItemIDs[i]._ocsID._ocsCapabilityID != FPGA_CAPABILITY::NONE)
                    return (ipCoreItemIDs[i]._ocsID._capabilityUniqueID == (int32_t)uniqueID);

            return false;
        };

        std::shared_ptr<HapiItemBase> spExistingItem(HapiItemBase::GetExistingItem(GetDriverFrameworkType(), pHapiItemBase, checkFunction).lock());
        if (spExistingItem)
            return std::dynamic_pointer_cast<T>(spExistingItem);

        if (!InitializeByUniqueID(pHapiItemBase, uniqueID))
            spResult.reset();

        return spResult;
    }

    template<class T, typename... ARGS>
    std::shared_ptr<T> IHapi::CreateByAssociatedID(uint32_t associatedID, ARGS... args)
    {
        std::shared_ptr<T> spResult(new T(args...));
        HapiItemBase* pHapiItemBase = dynamic_cast<HapiItemBase*>(spResult.get());

        bool OcsItem = (GetDriverFrameworkType() == DriverFrameworkType::OCS);

        // associatedID only supported by OCS
        if (not OcsItem)
            return {};

        // Have we got one already ?
        auto checkFunction = [associatedID](HapiItemBase* pItem)
        {
            const auto& ipCoreItemIDs = pItem->_ipCoreItemIDs._ids;
            const std::size_t num_ids = pItem->_ipCoreItemIDs._num_ids;

            for(std::size_t i = 0; i < num_ids; ++i)
                if(ipCoreItemIDs[i]._ocsID._ocsCapabilityID != FPGA_CAPABILITY::NONE)
                    return ipCoreItemIDs[i]._ocsID._capabilityAssociatedID == associatedID;

            return false;
        };

        std::shared_ptr<HapiItemBase> spExistingItem(HapiItemBase::GetExistingItem(pHapiItemBase, checkFunction).lock());
        if (spExistingItem)
            return std::dynamic_pointer_cast<T>(spExistingItem);

        if (!InitializeByAssociatedID(pHapiItemBase, associatedID))
            spResult.reset();

        return spResult;
    }

} // namespace Hapi

extern "C" uint32_t HapiReadRegister(void* pHapiItem, uint32_t registerIndex);
extern "C" void HapiWriteRegister(void* pHapiItem, uint32_t registerIndex, uint32_t registerValue);
