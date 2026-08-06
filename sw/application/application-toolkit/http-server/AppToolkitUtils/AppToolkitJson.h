/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "AppToolkitIJson.h"
#include <vector>

namespace AtUtils
{

    class JsonString;
    class JsonValue;
    class Json;
    class JsonMember;
    class JsonArray;
    class JsonObject;
    class JsonNumber;

    using JsonValuePtr = std::shared_ptr<JsonValue>;
    using JsonMemberPtr = std::shared_ptr<JsonMember>;
    using JsonArrayPtr = std::shared_ptr<JsonArray>;
    using JsonObjectPtr = std::shared_ptr<JsonObject>;
    using JsonNumberPtr = std::shared_ptr<JsonNumber>;

    class JsonBase
    {
    public:
        JsonBase(Json* pJsonParser);
        virtual void Save(std::vector<char>& data) = 0;

    protected:
        virtual bool Parse() = 0;

        bool MatchChar(char ch);
        bool PeekNext(char& ch);
        bool GetNext(char& ch);
        bool Backup(size_t nChars = 1);
        bool BackupToStart(const char* start);
        const char* GetPosition();
        std::string GetString(const char* start, const char* end);
        bool MatchString(std::string pString);
        bool ParseWhitespace();
        void AddString(std::vector<char>& data, std::string string, bool addQuotes = false);
        void AddIndentation(std::vector<char>& data);
        void Indent(int32_t delta);
        void MakeStdStrings(JsonValueVariant& variantValue);

        Json* _pJsonParser;
    };

    class JsonMember : public IJsonMember
    {
    public:
        JsonMember(std::string spName,
                   JsonValuePtr& spValue);

        // IJsonMember implementation
        const std::string& GetName() override   { return _spName; }
        IJsonValuePtr GetValue() override       { return _spValue; }

        std::string _spName;
        IJsonValuePtr _spValue;
    };

    class JsonValue : public JsonBase, public IJsonValue
    {
    public:
        JsonValue(Json* pJsonParser);
        JsonValue(Json* pJsonParser, JsonValueVariant variantValue);
        JsonValue(Json* pJsonParser, JsonObjectPtr spObject);
        JsonValue(Json* pJsonParser, JsonArrayPtr spArray);

        bool Parse() override;
        void Save(std::vector<char>& data) override;

        // IJsonValue interface
        IJsonArrayPtr GetArray() override { return std::dynamic_pointer_cast<IJsonArray>(_spArray); }
        IJsonObjectPtr GetObject() override { return std::dynamic_pointer_cast<IJsonObject>(_spObject); }
        JsonValueVariant GetValue() override;
        IJsonValuePtr At(size_t index) override;
        IJsonObjectPtr AddObject() override;
        IJsonArrayPtr AddArray() override;
        void AddValue(JsonValueVariant variantValue) override;
        void SetStringIsObject() override;

        JsonNumberPtr GetNumber() { return _spNumber; }

    private:
        JsonValueVariant _value;
        JsonObjectPtr _spObject;
        JsonArrayPtr _spArray;
        JsonNumberPtr _spNumber;
        bool _stringIsObject = false;
    };

    class JsonNumber : public JsonBase
    {
    public:
        JsonNumber(Json* pJsonParser);
        JsonNumber(Json* pJsonParser, double value);
        JsonNumber(Json* pJsonParser, int32_t value);
        JsonNumber(Json* pJsonParser, uint32_t value);
        bool Parse() override;
        void Save(std::vector<char>& data) override;

        // // double and int32_t supported for T
        // template <typename T> T Get();
        std::string GetValueString();

        JsonValueVariant GetValue() { return _value; }

    private:
        bool ParseFraction();
        bool ParseExponent();

        std::string _valueString;
        JsonValueVariant _value;
        bool _isFloatingPoint;
        bool _isSigned;
    };

    class JsonObject : public JsonBase, public IJsonObject
    {
        friend class Json;

    public:
        JsonObject(Json* pJsonParser);
        bool Parse() override;
        void Save(std::vector<char>& data) override;

        // IJsonObject interface
        IJsonObjectPtr AddObject(const char* name) override;
        IJsonArrayPtr AddArray(const char* name) override;
        IJsonValuePtr GetValue(const char* name) override;
        IJsonObjectPtr GetObject(const char* name) override;
        IJsonArrayPtr GetArray(const char* name) override;
        IJsonValuePtr AddValue(const char* name, JsonValueVariant value) override;
        std::vector<IJsonObjectPtr> GetAllChildObjects() override;
        size_t GetNumMembers() override { return _members.size(); }
        IJsonMemberPtr GetMember(size_t index) override;
        IJsonMemberPtr GetMember(const char* name) override;

    private:
        std::vector<IJsonMemberPtr> _members;
        bool _isRootObject = false;
    };

    class JsonArray : public JsonBase, public IJsonArray
    {
    public:
        JsonArray(Json* pJsonParser);
        bool Parse() override;
        void Save(std::vector<char>& data) override;

        // IJsonArray interface
        size_t Size() override { return _values.size(); }
        IJsonValuePtr operator[](size_t index) override { return At(index); }
        IJsonValuePtr At(size_t index) override;
        IJsonValuePtr AddElement() override;

    private:
        std::vector<JsonValuePtr> _values;
    };

    class JsonString : public JsonBase
    {
    public:
        JsonString(Json* pJsonParser);
        bool Parse() override;
        void Save(std::vector<char>& data) override;
        std::string Get() { return _value; }

    private:
        std::string _value;
    };

    class JsonWhitespace : public JsonBase
    {
    public:
        JsonWhitespace(Json* pJsonParser);
        void Save(std::vector<char>& data) override {};
        bool Parse() override;
    };

    class Json : public IJson
    {
        friend class JsonBase;

    public:
        Json(const std::string& fromString);
        Json(std::istream& inputStream);
        Json();

        // IJson interface
        bool Save(std::filesystem::path filePath) override;
		std::string ToString() override;
        IJsonObjectPtr Parse() override;
        IJsonObjectPtr RootObject() override;

    private:
        bool PeekNext(char& ch);
        bool GetNext(char& ch);
        bool Backup(size_t nChars);
        bool BackupToStart(const char* start);
        bool PreprocessComments(std::vector<char>& fullData);
        const char* GetPosition() { return _pStart; }
        void AddIndentation(std::vector<char>& data);
        void Indent(int32_t delta);

        std::vector<uint8_t> _data;
        const char* _pActualStart = nullptr;
        const char* _pStart = nullptr;
        const char* _pEnd = nullptr;
        JsonObjectPtr _rootObject;
        int32_t _indentation = 0;
		bool _useTabs = true;
    };

} // namespace AtUtils