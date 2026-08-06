/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "Variant.h"
#include <cstring>
#include <cinttypes>

namespace
{

template<typename T>
const T& Identity(const T& v)
{
    return v;
}

} // namespace

Variant::Variant()
:    _vt(VariantType::UNKNOWN)
,    _value{}
,    _convertFromString(false)
{
}

Variant::Variant(int32_t value)
:    _vt(VariantType::INTEGER_32)
,    _convertFromString(false)
{
    _value._intValue = value;
}

Variant::Variant(int64_t value)
:    _vt(VariantType::INTEGER_64)
,    _convertFromString(false)
{
    _value._longLongValue = value;
}

Variant::Variant(uint32_t value)
:    _vt(VariantType::UNSIGNED_INTEGER_32)
,    _convertFromString(false)
{
    _value._uint32Value = value;
}

Variant::Variant(uint64_t value)
:    _vt(VariantType::UNSIGNED_INTEGER_64)
,    _convertFromString(false)
{
    _value._uint64Value = value;
}

Variant::Variant(float value)
:    _vt(VariantType::FLOAT_32)
,    _convertFromString(false)
{
    _value._floatValue = value;
}

Variant::Variant(float value, std::string format)
:    _vt(VariantType::FLOAT_32)
,    _convertFromString(false)
,    _floatStringFormat(std::move(format))
{
    _value._floatValue = value;
}

Variant::Variant(double value)
:    _vt(VariantType::FLOAT_64)
,    _convertFromString(false)
{
    _value._doubleValue = value;
}

Variant::Variant(double value, std::string format)
:    _vt(VariantType::FLOAT_64)
,    _convertFromString(false)
,    _floatStringFormat(std::move(format))
{
    _value._doubleValue = value;
}

Variant::Variant(bool value)
:    _vt(VariantType::BOOLEAN)
,    _convertFromString(false)
{
    _value._boolValue = value;
}

Variant::Variant(std::string value)
:    _vt(VariantType::STRING)
,    _convertFromString(false)
{
    _stringValue = std::move(value);
}

Variant::Variant(const char* value)
:	Variant(std::string{value})
{
}

Variant::Variant(const std::vector<int>& value)
:    _vt(VariantType::VARIANT_ARRAY)
,    _convertFromString(false)
{
    for (int v : value)
    {
        _variantArrayValue.emplace_back(int32_t{v});  // NOTE: assuming int has 32 bits
    }
}

Variant::Variant(const std::vector<std::string>& value, int startIndex)
:    _vt(VariantType::VARIANT_ARRAY)
,    _convertFromString(false)
{
    if (static_cast<size_t>(startIndex) >= value.size())
    {
        return;
    }

    for (size_t i = static_cast<size_t>(startIndex); i < value.size(); ++i)
    {
        _variantArrayValue.emplace_back(value[i]);
    }
}

Variant::Variant(const std::vector<Variant>& value)
:    _vt(VariantType::VARIANT_ARRAY)
,    _convertFromString(false)
{
    _variantArrayValue = value;
}

////

void Variant::ConvertFromString(bool state)
{
    _convertFromString = state;
}

Variant::operator int32_t() const
{
    if (_convertFromString)
    {
        int32_t value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }

    assert(_vt == VariantType::INTEGER_32);
    return _value._intValue;
}

Variant::operator std::vector<int>() const
{
    return ToVector<int>( [](const std::string& s){ return atoi(s.c_str()); } );
}

Variant::operator std::vector<std::string>() const
{
    return ToVector<std::string>(Identity<std::string>);
}

Variant::operator std::vector<Variant>() const
{
    std::vector<Variant> value = ToVector<Variant>(Identity<std::string>);

    if (_convertFromString)
    {
        for (auto& item : value)
            item.ConvertFromString(true);
    }

    return value;
}

Variant::operator int64_t() const
{
    if (_convertFromString)
    {
        int64_t value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }
    if (_vt != VariantType::INTEGER_64)
        *_pErrorStream << "ERROR: Variant cast to int64_t _vt " << (uint32_t)_vt << std::endl;

    assert(_vt == VariantType::INTEGER_64);
    return _value._longLongValue;
}

Variant::operator uint32_t() const
{
    if (_convertFromString)
    {
        uint32_t value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }

    assert(_vt == VariantType::UNSIGNED_INTEGER_32);
    return _value._uint32Value;
}

Variant::operator uint64_t() const
{
    if (_convertFromString)
    {
        uint64_t value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }

    assert(_vt == VariantType::UNSIGNED_INTEGER_64);
    return _value._uint64Value;
}

Variant::operator float() const
{
    if (_convertFromString)
    {
        float value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }

    assert(_vt == VariantType::FLOAT_32);
    return _value._floatValue;
}

Variant::operator double() const
{
    if (_convertFromString)
    {
        double value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }

    assert(_vt == VariantType::FLOAT_64);
    return _value._doubleValue;
}

Variant::operator bool() const
{
    if (_convertFromString)
    {
        bool value = 0;
        if (_vt == VariantType::STRING)
        {
            AtUtils::FromString(_stringValue, value);
            return value;
        }
    }

    assert(_vt == VariantType::BOOLEAN);
    return _value._boolValue;
}

Variant::operator std::string() const
{
    if (_vt == VariantType::STRING)
        return _stringValue;
    else if (_vt == VariantType::VARIANT_ARRAY)
    {
        std::string str{};
        const char* separator = "";
        for (auto& v : _variantArrayValue)
        {
            str += separator;
            str += static_cast<std::string>(v);
            separator = ",";
        }
        return str;
    }
    else if (_vt == VariantType::BOOLEAN)
        return _value._boolValue ? "true" : "false";
    else if (_vt == VariantType::FLOAT_64)
    {
        std::string str;
        if (_floatStringFormat.empty())
            str = AtUtils::FormatString("%f", _value._doubleValue);
        else
            str = AtUtils::FormatString(_floatStringFormat.c_str(), _value._doubleValue);
        return str;
    }
    else if (_vt == VariantType::FLOAT_32)
    {
        std::string str;

        if (_floatStringFormat.empty())
            str = AtUtils::FormatString("%f", _value._floatValue);
        else
            str = AtUtils::FormatString(_floatStringFormat.c_str(), _value._floatValue);
        return str;
    }
    else if (_vt == VariantType::UNSIGNED_INTEGER_64)
    {
        return AtUtils::FormatString("%" PRIu64, _value._uint64Value);
    }
    else if (_vt == VariantType::UNSIGNED_INTEGER_32)
    {
        return AtUtils::FormatString("%" PRIu32, _value._uint32Value);
    }
    else if (_vt == VariantType::INTEGER_64)
    {
        return AtUtils::FormatString("%lld", _value._longLongValue);
    }
    else if (_vt == VariantType::INTEGER_32)
    {
        return AtUtils::FormatString("%d", _value._intValue);
    }
    else
        return "";
}

// If new_value comes is as a string, it is converted to the destination
// type, i.e. if you have an Variant of type int and you assign it with
// a new value of (string)"5", then it becomes (int)5 rather than (string)"5"
void Variant::AssignFromString(const Variant& new_value)
{
    std::string str;
    if (new_value._vt == VariantType::STRING)
        str = new_value._stringValue;
    else
    {
        // It's not a string, copy without conversion
        *this = new_value;
        return;
    }

    if (_vt == VariantType::STRING)
        _stringValue = std::move(str);
    else if (_vt == VariantType::VARIANT_ARRAY)
    {
        Variant temp{new_value};
        temp.ConvertFromString(true);
        _variantArrayValue = temp;
    }
    else if (_vt == VariantType::BOOLEAN)
        AtUtils::FromString(str, _value._boolValue);
    else if (_vt == VariantType::FLOAT_64)
        AtUtils::FromString(str, _value._doubleValue);
    else if (_vt == VariantType::FLOAT_32)
        AtUtils::FromString(str, _value._floatValue);
    else if (_vt == VariantType::UNSIGNED_INTEGER_64)
        AtUtils::FromString(str, _value._uint64Value);
    else if (_vt == VariantType::UNSIGNED_INTEGER_32)
        AtUtils::FromString(str, _value._uint32Value);
    else if (_vt == VariantType::INTEGER_64)
        AtUtils::FromString(str, _value._longLongValue);
    else if (_vt == VariantType::INTEGER_32)
        AtUtils::FromString(str, _value._intValue);
}

bool Variant::operator==(const Variant& other) const
{
    if (_convertFromString && other._convertFromString)
        return _stringValue == other._stringValue;
    else if (_convertFromString)
    {
        Variant thisConverted{other};
        thisConverted.AssignFromString(*this);
        return thisConverted == other;
    }
    else if (other._convertFromString)
    {
        Variant otherConverted{*this};
        otherConverted.AssignFromString(other);
        return *this == otherConverted;
    }
    else if (_vt != other._vt)
        return false;
    else if (_vt == VariantType::INTEGER_32)
        return (_value._intValue == other._value._intValue);
    else if (_vt == VariantType::INTEGER_64)
        return (_value._longLongValue == other._value._longLongValue);
    else if (_vt == VariantType::UNSIGNED_INTEGER_32)
        return (_value._uint32Value == other._value._uint32Value);
    else if (_vt == VariantType::UNSIGNED_INTEGER_64)
        return (_value._uint64Value == other._value._uint64Value);
    else if (_vt == VariantType::FLOAT_32)
        return (_value._floatValue == other._value._floatValue);
    else if (_vt == VariantType::FLOAT_64)
        return (_value. _doubleValue== other._value._doubleValue);
    else if (_vt == VariantType::BOOLEAN)
        return (_value._boolValue == other._value._boolValue);
    else if (_vt == VariantType::STRING)
        return (_stringValue == other._stringValue);
    else if (_vt == VariantType::VARIANT_ARRAY)
        return (_variantArrayValue == other._variantArrayValue);
    else
        return false;
}

bool Variant::operator!=(const Variant& other) const
{
    bool equals = (*this == other);
    return !equals;
}

std::string Variant::ToString() const
{
    return static_cast<std::string>(*this);
}

void Variant::SetErrorStream(std::ostream& errorStream)
{
    _pErrorStream = &errorStream;
}

template<typename T, typename F>
std::vector<T> Variant::ToVector(F itemFromString) const
{
    std::vector<T> value;

    if (_convertFromString)
    {
        if (!_stringValue.empty())
        {
            size_t start = 0;
            while (true)
            {
                size_t comma = _stringValue.find(",", start);

                if (comma == std::string::npos)
                {
                    value.push_back(itemFromString(AtUtils::Mid(_stringValue, start)));
                    break;
                }
                else
                {
                    value.push_back(itemFromString(AtUtils::Mid(_stringValue, start, comma - start)));
                    start = comma + 1;
                }

            }
        }
    }
    else
    {
        assert(_vt == VariantType::VARIANT_ARRAY);

        for (auto& v : _variantArrayValue)
        {
            value.emplace_back(v);
        }
    }

    return value;
}
