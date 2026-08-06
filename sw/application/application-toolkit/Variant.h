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
#include <vector>
#include <iostream>

class Variant
{
public:
    enum class VariantType : uint32_t
    {
        UNKNOWN,
        INTEGER_32,
        INTEGER_64,
        UNSIGNED_INTEGER_32,
        UNSIGNED_INTEGER_64,
        FLOAT_32,
        FLOAT_64,
        BOOLEAN,
        STRING,
        VARIANT_ARRAY
    };

    Variant();
    Variant(int32_t);
    Variant(uint32_t);
    Variant(int64_t);
    Variant(uint64_t);
    Variant(float);
    Variant(float, std::string format);
    Variant(double);
    Variant(double, std::string format);
    Variant(bool);
    Variant(std::string);
    Variant(const char*); // Required to avoid unwanted conversions to bool when using string literals
    Variant(const std::vector<int>&);
    Variant(const std::vector<std::string>& compositeValues, int startIndex = 0);
    Variant(const std::vector<Variant>& compositeValues);

    Variant(const Variant& other) = default;

    operator int32_t() const;
    operator int64_t() const;
    operator uint32_t() const;
    operator uint64_t() const;
    operator float() const;
    operator double() const;
    operator bool() const;
    operator std::vector<int>() const;
    operator std::vector<std::string>() const;
    operator std::vector<Variant>() const;

    bool operator==(const Variant& other) const;
    bool operator!=(const Variant& other) const;
    Variant& operator=(const Variant& other) = default;

    // These will convert the numeric/boolean types to a string for you
    operator std::string() const;

    void AssignFromString(const Variant& new_value);
    void ConvertFromString(bool state);
    bool IsString() const  { return (_vt == VariantType::STRING); }
    bool IsBool() const    { return (_vt == VariantType::BOOLEAN); }
    bool IsFloat32() const { return (_vt == VariantType::FLOAT_32); }
    bool IsValid() const   { return (_vt != VariantType::UNKNOWN); }
    std::string ToString() const;

    void SetErrorStream(std::ostream& errorStream);  // NOTE: stores stream by pointer. Mainly used for testing.

private:
    template<typename T, typename F>
    std::vector<T> ToVector(F fromString) const;

    VariantType    _vt;

    union ValueUnion
    {
        int32_t  _intValue;
        int64_t  _longLongValue;
        uint32_t _uint32Value;
        uint64_t _uint64Value;
        float    _floatValue;
        double   _doubleValue;
        bool     _boolValue;
    } _value;

    std::vector<Variant> _variantArrayValue;
    std::string _stringValue;
    bool        _convertFromString;
    std::string _floatStringFormat;
    std::ostream* _pErrorStream = &std::cout;
};
