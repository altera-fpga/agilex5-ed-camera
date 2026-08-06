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
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <concepts>
#include <type_traits>
#include <functional>
#include <unordered_map>

namespace SwApi
{
    struct I2CBusInfo
    {
        uint32_t busID;
        std::string sysClassName;
    };


    class II2CBusEnumerator
    {
    public:
        virtual ~II2CBusEnumerator() = default;
        virtual std::vector<I2CBusInfo> EnumerateBuses() const = 0;
    };


    class II2CBusIo
    {
        public:
            virtual ~II2CBusIo() = default;
            virtual bool Read(uint16_t chipAddress, uint8_t* data, uint16_t size) = 0;
            virtual bool Write(uint16_t chipAddress, const uint8_t* data, uint16_t size) = 0;
            virtual uint32_t GetID() const = 0;
    };


    class I2CDeviceBase
    {
    public:
        I2CDeviceBase(std::weak_ptr<II2CBusIo> wpBus, uint16_t deviceAddress, const std::string& deviceName);
        virtual ~I2CDeviceBase() = default;

        // Raw i2c read/write interface
        bool Read(uint8_t* data, uint16_t size);
        bool Write(const uint8_t* data, uint16_t size);
        // Type agnostic register read/write interface for polymorphic access
        virtual bool ReadRegisterToU32(const uint32_t reg, uint32_t& value) = 0;
        virtual bool WriteRegisterFromU32(const uint32_t reg, const uint32_t value) = 0;
        uint32_t GetBusID();
        uint16_t GetAddress();
        std::string GetName();

    private:
        std::weak_ptr<II2CBusIo> _wpBus;
        uint16_t _deviceAddress;
        std::string _deviceName;
    };

    ///////////////////////////////////////////////////

    template <typename... Ts>
    constexpr auto EncodeDataBE(Ts... values)
    {
        static_assert(sizeof...(Ts) > 0, "At least one value is required");
        static_assert((std::is_integral_v<std::decay_t<Ts>> && ...), "All parameters must be integral");
        constexpr std::size_t N = (sizeof(std::decay_t<Ts>) + ... + 0);

        std::array<uint8_t, N> out{};
        std::size_t offset = 0;

        auto encodeOne = [&](auto v)
        {
            using ValueT = std::decay_t<decltype(v)>;
            using UnsignedT = std::make_unsigned_t<ValueT>;
            constexpr std::size_t M = sizeof(ValueT);

            UnsignedT value = static_cast<UnsignedT>(v);
            for (std::size_t i = 0; i < M; ++i)
            {
                const std::size_t shift = 8 * (M - 1 - i);
                out[offset + i] = static_cast<uint8_t>((value >> shift) & 0xffU);
            }
            offset += M;
        };

        (encodeOne(values), ...);

        return out;
    }    

    template <typename T>
    constexpr T DecodeDataBE(const std::array<uint8_t, sizeof(T)>& data)
    {
        static_assert(std::is_integral_v<T>, "T must be integral");
        constexpr std::size_t N = sizeof(T);

        T value = 0;
        for (std::size_t i = 0; i < N; ++i)
        {
            const std::size_t shift = 8 * (N - 1 - i);
            value |= static_cast<T>(data[i]) << shift;
        }

        return value;
    }

    template<std::size_t Bits>
    struct Reg
    {
        static_assert(Bits == 8 || Bits == 16, "Unsupported register address size");
        using Type = std::conditional_t<Bits == 8, uint8_t, uint16_t>;
    };

    template<typename T>
    concept RegWidth = std::same_as<T, Reg<8>> || std::same_as<T, Reg<16>>;

    template<std::size_t Bits>
    struct Val
    {
        static_assert(Bits == 8 || Bits == 16, "Unsupported register value size");
        using Type = std::conditional_t<Bits == 8, uint8_t, uint16_t>;
    };

    template<typename T>
    concept ValWidth = std::same_as<T, Val<8>> || std::same_as<T, Val<16>>;

    template<RegWidth RW, ValWidth VW>
    struct I2CDevice : public I2CDeviceBase
    {
        using RegT = typename RW::Type;
        using ValT = typename VW::Type;

        I2CDevice(std::weak_ptr<II2CBusIo> wpBus, uint16_t deviceAddress, const std::string& deviceName): 
            I2CDeviceBase{wpBus, deviceAddress, deviceName}
        {}

        virtual ~I2CDevice() = default;
        
        bool ReadRegister(const RegT& reg, ValT& value)
        {
            bool ret = false;

            const auto regAddr = EncodeDataBE(reg);

            if(Write(regAddr.data(), regAddr.size()))
            {
                constexpr std::size_t N = sizeof(ValT);
                std::array<uint8_t, N> raw_value{};

                if(Read(raw_value.data(), raw_value.size()))
                {
                    value = DecodeDataBE<ValT>(raw_value);
                    ret = true;
                }
            }

            return ret;
        }

        bool WriteRegister(const RegT& reg, const ValT& value)
        {
            const auto data = EncodeDataBE(reg, value);
            return Write(data.data(), data.size());
        }

        virtual bool ReadRegisterToU32(const uint32_t reg, uint32_t& value) override
        {
            RegT regTyped = static_cast<RegT>(reg);
            ValT valueTyped;
            bool success = ReadRegister(regTyped, valueTyped);

            if(success)
                value = static_cast<uint32_t>(valueTyped);

            return success;
        }

        virtual bool WriteRegisterFromU32(const uint32_t reg, const uint32_t value) override
        {
            RegT regTyped = static_cast<RegT>(reg);
            ValT valueTyped = static_cast<ValT>(value);
            return WriteRegister(regTyped, valueTyped);
        }
    };

    // Helper to check if a type is an I2CDevice specialization
    template<typename T>
    struct IsI2CDeviceType : std::false_type {};

    template<RegWidth RW, ValWidth VW>
    struct IsI2CDeviceType<I2CDevice<RW, VW>> : std::true_type {};

    template<typename T>
    concept I2CDeviceType = IsI2CDeviceType<std::remove_cvref_t<T>>::value;


    class I2CBus
    {
    public:
        I2CBus(uint32_t busID, const std::string& sysClassName);
        I2CBus(uint32_t busID, const std::string& sysClassName, std::shared_ptr<II2CBusIo> spBusIo);

        uint32_t GetID() const { return _busID; }
        std::string GetSysClassName() const { return _sysClassName; }

        template<I2CDeviceType DeviceT>
        std::shared_ptr<DeviceT> AddDevice(uint16_t deviceAddress, const std::string& deviceName)
        {
            // Only keep 1 device per address
            for(auto& spDevice : _devices){
                if (spDevice->GetAddress() == deviceAddress)
                    return std::dynamic_pointer_cast<DeviceT>(spDevice);
            }

            auto spDevice = std::make_shared<DeviceT>(_spBusIo, deviceAddress, deviceName);
            _devices.push_back(spDevice);
            return spDevice;
        }

        std::vector<std::shared_ptr<I2CDeviceBase>> GetDevices() ;

    private:
        uint32_t _busID;
        std::string _sysClassName;
        std::shared_ptr<II2CBusIo> _spBusIo;
        std::vector<std::shared_ptr<I2CDeviceBase>> _devices;
    };
    
    class I2C
    {
    public:
        static std::shared_ptr<I2C> Create();

        I2C(const std::shared_ptr<II2CBusEnumerator>& spBusEnumerator,
            std::function<std::shared_ptr<II2CBusIo>(uint32_t)> busIoFactory = nullptr);

        std::vector<uint32_t> GetBusIDs() const;
        std::shared_ptr<I2CBus> GetBus(uint32_t busID);
        std::vector<std::shared_ptr<I2CBus>> GetBuses();

    private:
        std::unordered_map<uint32_t, std::shared_ptr<I2CBus>> _buses;
        std::vector<uint32_t> _busIDs;
    };

} // namespace SwApi
