/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <memory>
#include <variant>
#include <string>
#include <filesystem>
#include <vector>
#include "AppToolkitStringUtils.h"

namespace AtUtils
{

    class IJson;
    class IJsonValue;
    class IJsonObject;
    class IJsonArray;
    class IJsonMember;

    using IJsonPtr = std::shared_ptr<IJson>;
    using IJsonValuePtr = std::shared_ptr<IJsonValue>;
    using IJsonMemberPtr = std::shared_ptr<IJsonMember>;
    using IJsonObjectPtr = std::shared_ptr<IJsonObject>;
    using IJsonArrayPtr = std::shared_ptr<IJsonArray>;
    using JsonValueVariant = std::variant<const char*, std::string, int32_t, uint32_t, double, bool>;

    class IJson
    {
    public:
        static IJsonPtr Create(std::istream& fromStream);
        static IJsonPtr Create(std::filesystem::path filePath);
        static IJsonPtr Create(const std::string& fromString);
        static IJsonPtr Create();

        virtual bool Save(std::filesystem::path filePath) = 0;
        virtual std::string ToString() = 0;
        virtual IJsonObjectPtr Parse() = 0;
        virtual IJsonObjectPtr RootObject() = 0;
    };

    class IJsonMember
    {
    public:
        virtual const std::string& GetName() = 0;
        virtual IJsonValuePtr GetValue() = 0;
    };

    class IJsonValue
    {
    public:
        virtual IJsonArrayPtr GetArray() = 0;
        virtual IJsonObjectPtr GetObject() = 0;
        virtual JsonValueVariant GetValue() = 0;
        virtual IJsonValuePtr At(size_t index) = 0;
        virtual IJsonObjectPtr AddObject() = 0;
        virtual IJsonArrayPtr AddArray() = 0;
        virtual void AddValue(JsonValueVariant variantValue) = 0;
        virtual void SetStringIsObject() = 0;

        template<typename T> T GetValue()
        {
            T value{};
            JsonValueVariant valueVariant = GetValue();

            try
            {
                value = std::get<T>(valueVariant);
            }
            catch (...)
            {
                std::string jsonError = "JSON error: incorrect value type";
                throw(jsonError);
            }

            return value;
        }
    };

    class IJsonObject
    {
    public:
        virtual IJsonObjectPtr AddObject(const char* name) = 0;
        virtual IJsonArrayPtr AddArray(const char* name) = 0;
        virtual IJsonValuePtr GetValue(const char* name) = 0;
        virtual IJsonObjectPtr GetObject(const char* name) = 0;
        virtual IJsonArrayPtr GetArray(const char* name) = 0;

        template<typename T> T GetValue(const char* name)
        {
            T value{};
            auto spJsonValueItem = GetValue(name);

            if (spJsonValueItem)
            {
                try
                {
                    value = spJsonValueItem->GetValue<T>();
                }
                catch (...)
                {
                    std::string jsonError = FormatString("JSON error: incorrect value type for '%s'", name);
                    throw(jsonError);
                }
            }
            else
            {
                std::string jsonError = FormatString("JSON error: no value named '%s'", name);
                throw(jsonError);
            }
            return value;
        }

        virtual IJsonValuePtr AddValue(const char* name, JsonValueVariant value) = 0;
        virtual std::vector<IJsonObjectPtr> GetAllChildObjects() = 0;
        virtual size_t GetNumMembers() = 0;
        virtual IJsonMemberPtr GetMember(size_t index) = 0;
        virtual IJsonMemberPtr GetMember(const char* name) = 0;
    };

    class IJsonArray
    {
    public:
        virtual size_t Size() = 0;
        virtual IJsonValuePtr operator[](size_t index) = 0;
        virtual IJsonValuePtr At(size_t index) = 0;
        virtual IJsonValuePtr AddElement() = 0;
    };

} // namespace AtUtils