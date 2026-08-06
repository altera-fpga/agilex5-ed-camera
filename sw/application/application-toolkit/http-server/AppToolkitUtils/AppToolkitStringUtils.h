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
#include <string>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <type_traits>
#include <limits>

#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace AtUtils
{
    namespace Impl
    {
        template<typename T> inline constexpr bool TYPE_IS_SUPPORTED = false;

        template<typename T>
        auto WideFromString(const std::string& stringValue)
        {
            if constexpr(std::is_integral_v<T>)
            {
                if constexpr(std::is_unsigned_v<T>)
                {
                    auto uintValue = std::stoull(stringValue, nullptr, 0);
                    return uintValue;
                }
                else
                {
                    auto intValue = std::stoll(stringValue, nullptr, 0);
                    return intValue;
                }
            }
            else if constexpr(std::is_floating_point_v<T>)
            {
                auto ldValue = std::stold(stringValue, nullptr);
                return ldValue;
            }
            else
            {
                static_assert(TYPE_IS_SUPPORTED<T>, "Unsupported type"); // Must depend on the type otherwise it always fails
            }
        }
    }

    std::string FormatString(const char* format, ...);

    template<typename T>
    bool FromString(const std::string& stringValue, T& value)
    {
        try
        {
            auto converted = Impl::WideFromString<T>(stringValue);
#ifdef REDUNDANT_CHECK
            // value of type T can never exceed its numeric limits
            if ( (converted > std::numeric_limits<T>::max()) ||
                 (converted < std::numeric_limits<T>::lowest()) )
            {
                throw std::out_of_range{"after conversion"};
            }
#endif

            value = static_cast<T>(converted);
        }
        catch(std::exception&)
        {
            value = 0;
            return false;
        }

        return true;
    }

    template<>
    bool FromString<bool>(const std::string& stringValue, bool& value);

	template<typename T>
    T FromString(const std::string& stringValue)
	{
		T value{};
		FromString<T>(stringValue, value);
		return value;
	}

    std::string ToString(bool value);
    std::string ToString(float value);
    std::string ToString(double value, const char* formatString = "%0.6f");
    std::string ToString(uint32_t value);
    std::string ToString(int32_t value);
    std::string ToString(uint64_t value);
    std::string ToString(int64_t value);
    std::wstring ToStringW(const std::string& inputString);

    void MakeLower(std::string& stringValue);
    void MakeUpper(std::string& stringValue);
    std::string Left(const std::string& stringValue, size_t nCharacters);
    std::string Mid(const std::string& stringValue, size_t position, size_t nCharacters);
    std::string Mid(const std::string& stringValue, size_t position);
    std::string Right(const std::string& stringValue, size_t nCharacters);
    bool Replace(std::string& stringValue, const char* findString, const char* replaceString);
    void TrimLeft(std::string& stringValue);
    void TrimRight(std::string& stringValue);
    void MakeSafeXml(std::string& stringValue);
    void CopyString(char* destination, size_t destinationLength, const char* source);
    void CopyMemoryBuffer(void* destination, size_t destinationLength, const void* source, size_t sourceLength);
    void MemorySet(void* destination, size_t destinationLength, int value);
}
