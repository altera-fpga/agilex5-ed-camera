/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AppToolkitJson.h"
#include "AppToolkitStringUtils.h"
#include <fstream>

namespace AtUtils
{
    IJsonPtr IJson::Create(std::istream& fromStream)
    {
        auto spJson = std::make_shared<Json>(fromStream);
        return std::dynamic_pointer_cast<IJson>(spJson);        
    }
    
    IJsonPtr IJson::Create(std::filesystem::path filePath)
    {
        std::fstream inputStream;
        std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary;
        inputStream.open(filePath, mode);
        auto spJson = std::make_shared<Json>(inputStream);
        return std::dynamic_pointer_cast<IJson>(spJson);
    }

    IJsonPtr IJson::Create(const std::string& fromString)
    {
        auto spJson = std::make_shared<Json>(fromString);
        return std::dynamic_pointer_cast<IJson>(spJson);
    }

    IJsonPtr IJson::Create()
    {
        auto spJson = std::make_shared<Json>();
        return std::dynamic_pointer_cast<IJson>(spJson);
    }

    Json::Json()
    {
        _rootObject = std::make_shared<JsonObject>(this);
        _rootObject->_isRootObject = true;
    }

    Json::Json(std::istream& inputStream)
    {
        _rootObject = std::make_shared<JsonObject>(this);
        _rootObject->_isRootObject = true;

        if (inputStream.good())
        {
            // Get size of data in the stream
            inputStream.seekg(0, inputStream.end);
            int dataSize = inputStream.tellg();
            inputStream.seekg(0, inputStream.beg);

            std::vector<char> fullData(dataSize);
            inputStream.read(fullData.data(), dataSize);

            PreprocessComments(fullData);

            _pActualStart = (char*)_data.data();
            _pStart = _pActualStart;
            _pEnd = _pStart + _data.size();
        }
    }    

    Json::Json(const std::string& fromString)
    {
        _rootObject = std::make_shared<JsonObject>(this);
        _rootObject->_isRootObject = true;
		std::vector<char> fullData(fromString.begin(), fromString.end());

		PreprocessComments(fullData);

		_pActualStart = (char*)_data.data();
		_pStart = _pActualStart;
		_pEnd = _pStart + _data.size();
    }

    IJsonObjectPtr Json::RootObject()
    {
        return std::dynamic_pointer_cast<IJsonObject>(_rootObject);
    }

    bool Json::PreprocessComments(std::vector<char>& fullData)
    {
        bool foundComment = false;
        bool inComment = false;
        int slashCount = 0;
        for (uint8_t ch : fullData)
        {
            if (ch == '/')
            {
                slashCount++;
                if (slashCount >= 2)
                {
                    foundComment = true;
                    inComment = true;
                }
            }
            else
            {
                if ((slashCount == 1) && !inComment)
                    _data.push_back('/');

                slashCount = 0;
                if ((ch == '\r') || (ch == '\n'))
                    inComment = false;

                if (!inComment)
                    _data.push_back(ch);
            }
        }

        return foundComment;
    }

    IJsonObjectPtr Json::Parse()
    {
        if (_rootObject->Parse())
        {
            return std::dynamic_pointer_cast<IJsonObject>(_rootObject);
        }
        else
        {
            _rootObject.reset();
            return nullptr;
        }
    }

    bool Json::PeekNext(char& ch)
    {
        if (_pStart < _pEnd)
        {
            ch = _pStart[0];
            return true;
        }

        return false;
    }

    bool Json::GetNext(char& ch)
    {
        if (_pStart < _pEnd)
        {
            ch = _pStart[0];
            _pStart++;
            return true;
        }

        return false;
    }

    bool Json::Backup(size_t nChars)
    {
        for (size_t i = 0; i < nChars; i++)
        {
            if (_pStart > _pActualStart)
                _pStart--;
            else
                return false;
        }

        return true;
    }

    bool Json::BackupToStart(const char* start)
    {
        size_t nChars = _pStart - start;
        Backup(nChars);

        // Always false
        return false;
    }

    void Json::AddIndentation(std::vector<char>& data)
    {
        data.push_back('\n');
        for (int32_t i = 0; i < _indentation; i++)
        {
			if (_useTabs)
				data.push_back('\t');
			else
			{
            	for (int32_t s = 0; s < 4; s++)
                	data.push_back(' ');
			}
        }
    }

    void Json::Indent(int32_t delta)
    {
        _indentation += delta;
    }

    bool Json::Save(std::filesystem::path filePath)
    {
        if (!_rootObject)
            return false;

        std::vector<char> data;
        _rootObject->Save(data);

        std::fstream outputStream;
        std::ios_base::openmode mode = std::ios_base::out | std::ios_base::binary;
        outputStream.open(filePath, mode);
        if (outputStream.good())
        {
            outputStream.write(data.data(), data.size());
        }

        return true;
    }

	std::string Json::ToString()
    {
        if (!_rootObject)
            return "";

        std::vector<char> buffer;
        _rootObject->Save(buffer);
		buffer.push_back(0);

		std::string jsonString(buffer.data());
        return jsonString;
    }

    ///////////////////////////////////////////////////////////////////////////////

    JsonBase::JsonBase(Json* pJsonParser)
        : _pJsonParser(pJsonParser)
    {
    }

    bool JsonBase::MatchChar(char value)
    {
        char nextChar;
        if (!GetNext(nextChar))
            return false;

        if (value == nextChar)
            return true;

        Backup();
        return false;
    }

    bool JsonBase::PeekNext(char& value)
    {
        return _pJsonParser->PeekNext(value);
    }

    bool JsonBase::GetNext(char& value)
    {
        return _pJsonParser->GetNext(value);
    }

    bool JsonBase::Backup(size_t nChars)
    {
        return _pJsonParser->Backup(nChars);
    }

    bool JsonBase::BackupToStart(const char* start)
    {
        return _pJsonParser->BackupToStart(start);
    }

    const char* JsonBase::GetPosition()
    {
        return _pJsonParser->GetPosition();
    }

    void JsonBase::AddIndentation(std::vector<char>& data)
    {
        return _pJsonParser->AddIndentation(data);
    }

    void JsonBase::Indent(int32_t delta)
    {
        return _pJsonParser->Indent(delta);
    }

    std::string JsonBase::GetString(const char* start, const char* end)
    {
        std::string result;
        for (const char* scan = start; scan < end; scan++)
        {
            result += scan[0];
        }

        return result;
    }

    bool JsonBase::MatchString(std::string string)
    {
        uint32_t nConsumed = 0;
        for (char ch : string)
        {
            char nextChar;
            if (!GetNext(nextChar))
                return false;

            nConsumed++;
            if (ch != nextChar)
            {
                Backup(nConsumed);
                return false;
            }
        }

        return true;
    }

    bool JsonBase::ParseWhitespace()
    {
        auto spWhitespace = std::make_shared<JsonWhitespace>(_pJsonParser);
        return spWhitespace->Parse();
    }

    void JsonBase::AddString(std::vector<char>& data, std::string string, bool addQuotes)
    {
        if (addQuotes)
            data.push_back('"');

        for (char ch : string)
            data.push_back(ch);

        if (addQuotes)
            data.push_back('"');
    }

    void JsonBase::MakeStdStrings(JsonValueVariant& variantValue)
    {
        // Convert char* to std::string so we don't lose the string
        // on the stack
        if (std::holds_alternative<const char*>(variantValue))
        {
            const char* value = std::get<const char*>(variantValue);
            std::string strValue(value);
            variantValue = strValue;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////

    JsonObject::JsonObject(Json* pJsonParser)
        : JsonBase(pJsonParser)
    {
    }

    bool JsonObject::Parse()
    {
        if (!MatchChar('{'))
            return false;

        if (!ParseWhitespace())
            return false;

        if (MatchChar('}'))
            return true;

        while (true)
        {
            auto spString = std::make_shared<JsonString>(_pJsonParser);
            if (!spString->Parse())
                return false;

            if (!ParseWhitespace())
                return false;

            if (!MatchChar(':'))
                return false;

            auto spValue = std::make_shared<JsonValue>(_pJsonParser);
            if (!spValue->Parse())
                return false;

            auto spNewMember = std::make_shared<JsonMember>(spString->Get(), spValue);
            _members.push_back(spNewMember);

            if (MatchChar('}'))
                return true;

            if (!MatchChar(','))
                return false;

            if (!ParseWhitespace())
                return false;
        }

        return true;
    }

    IJsonMemberPtr JsonObject::GetMember(size_t index)
    {
        if (index < _members.size())
            return _members[index];
        else
            return nullptr;
    }

    IJsonMemberPtr JsonObject::GetMember(const char* name)
    {
        for (auto& spMember : _members)
        {
            if (spMember->GetName() == name)
                return spMember;
        }

        return nullptr;
    }

    IJsonValuePtr JsonObject::GetValue(const char* name)
    {
        for (auto& spMember : _members)
        {
            if (spMember->GetName() == name)
                return spMember->GetValue();
        }

        return nullptr;
    }

    IJsonValuePtr JsonObject::AddValue(const char* name, JsonValueVariant value)
    {
        MakeStdStrings(value);
        JsonValuePtr spJsonValue = std::make_shared<JsonValue>(_pJsonParser, value);
        auto spNewMember = std::make_shared<JsonMember>(name, spJsonValue);
        _members.push_back(spNewMember);
        return spJsonValue; //std::dynamic_pointer_cast<IJsonValuePtr>(spJsonValue);
    }

    IJsonObjectPtr JsonObject::AddObject(const char* name)
    {
        auto spObject = std::make_shared<JsonObject>(_pJsonParser);
        auto spJsonValue = std::make_shared<JsonValue>(_pJsonParser, spObject);
        auto spNewMember = std::make_shared<JsonMember>(name, spJsonValue);
        _members.push_back(spNewMember);
        return std::dynamic_pointer_cast<IJsonObject>(spObject);
    }

    IJsonArrayPtr JsonObject::AddArray(const char* name)
    {
        auto spArray = std::make_shared<JsonArray>(_pJsonParser);
        auto spJsonValue = std::make_shared<JsonValue>(_pJsonParser, spArray);
        auto spNewMember = std::make_shared<JsonMember>(name, spJsonValue);
        _members.push_back(spNewMember);
        return std::dynamic_pointer_cast<IJsonArray>(spArray);
    }

    std::vector<IJsonObjectPtr> JsonObject::GetAllChildObjects()
    {
        std::vector<IJsonObjectPtr> childObjects;

        for (auto& spMember : _members)
        {
            auto spValue = spMember->GetValue();
            auto spObject = spValue->GetObject();
            if (spObject)
                childObjects.push_back(std::move(spObject));
        }

        return childObjects;
    }

    IJsonObjectPtr JsonObject::GetObject(const char* name)
    {
        auto spValue = GetValue(name);
        if (spValue)
            return spValue->GetObject();
        else
            return nullptr;
    }

    IJsonArrayPtr JsonObject::GetArray(const char* name)
    {
        auto spValue = GetValue(name);
        if (spValue)
            return spValue->GetArray();
        else
            return nullptr;
    }

    void JsonObject::Save(std::vector<char>& data)
    {
        if (!_isRootObject)
            AddIndentation(data);

        AddString(data, "{");
        Indent(1);

        bool first = true;
        for (auto member : _members)
        {
            if (!first)
                AddString(data, ",");

            first = false;

            AddIndentation(data);
            AddString(data, member->GetName(), true);
            AddString(data, ": ");

            auto pValue = std::dynamic_pointer_cast<JsonValue>(member->GetValue());
            if (pValue)
                pValue->Save(data);
        }

        Indent(-1);
        AddIndentation(data);
        AddString(data, "}");
    }

    ///////////////////////////////////////////////////////////////////////////////

    JsonMember::JsonMember(std::string spName,
                           std::shared_ptr<JsonValue>& spValue)
        : _spName(std::move(spName))
        , _spValue(spValue)
    {
    }

    JsonArray::JsonArray(Json* pJsonParser)
        : JsonBase(pJsonParser)
    {
    }

    bool JsonArray::Parse()
    {
        if (!MatchChar('['))
            return false;

        if (!ParseWhitespace())
            return false;

        if (MatchChar(']'))
            return true;

        while (true)
        {
            auto spValue = std::make_shared<JsonValue>(_pJsonParser);
            if (!spValue->Parse())
                return false;

            _values.push_back(std::move(spValue));

            if (MatchChar(']'))
                return true;

            if (!MatchChar(','))
                return false;
        }

        return true;
    }

    IJsonValuePtr JsonArray::At(size_t index)
    {
        if (index < _values.size())
            return std::dynamic_pointer_cast<IJsonValue>(_values[index]);
        else
            return nullptr;
    }

    IJsonValuePtr JsonArray::AddElement()
    {
        auto spValue = std::make_shared<JsonValue>(_pJsonParser);
        _values.push_back(spValue);
        return std::dynamic_pointer_cast<IJsonValue>(spValue);
    }

    void JsonArray::Save(std::vector<char>& data)
    {
        if (_values.empty())
        {
            AddString(data, "[]");
            return;
        }

        AddIndentation(data);
        AddString(data, "[");
        Indent(1);

        bool first = true;
        for (auto value : _values)
        {
            if (!first)
                AddString(data, ",");

            first = false;

            if (!value->GetObject() && !value->GetArray())
                AddIndentation(data);

            value->Save(data);
        }

        Indent(-1);
        AddIndentation(data);
        AddString(data, "]");
    }

    ///////////////////////////////////////////////////////////////////////////////

    JsonValue::JsonValue(Json* pJsonParser)
    :   JsonBase(pJsonParser)
    ,   _value{}
    {
    }

    JsonValue::JsonValue(Json* pJsonParser, JsonValueVariant variantValue)
    :   JsonBase(pJsonParser)
    ,   _value{}
    {
        if (std::holds_alternative<double>(variantValue))
        {
            double value = std::get<double>(variantValue);
            _spNumber = std::make_shared<JsonNumber>(_pJsonParser, value);
        }
        else if (std::holds_alternative<int32_t>(variantValue))
        {
            int32_t value = std::get<int32_t>(variantValue);
            _spNumber = std::make_shared<JsonNumber>(_pJsonParser, value);
        }
        else if (std::holds_alternative<uint32_t>(variantValue))
        {
            uint32_t value = std::get<uint32_t>(variantValue);
            _spNumber = std::make_shared<JsonNumber>(_pJsonParser, value);
        }

        _value = variantValue;
    }

    JsonValue::JsonValue(Json* pJsonParser, JsonObjectPtr spObject)
    :   JsonBase(pJsonParser)
    ,   _value{}
    ,   _spObject(std::move(spObject))
    {
    }

    JsonValue::JsonValue(Json* pJsonParser, JsonArrayPtr spArray)
    :   JsonBase(pJsonParser)
    ,   _value{}
    ,   _spArray(std::move(spArray))
    {
    }

    bool JsonValue::Parse()
    {
        if (!ParseWhitespace())
            return false;

        if (MatchString("null"))
        {
            _value = "";
            return ParseWhitespace();
        }

        if (MatchString("true"))
        {
            _value = true;
            return ParseWhitespace();
        }

        if (MatchString("false"))
        {
            _value = false;
            return ParseWhitespace();
        }

        auto spString = std::make_shared<JsonString>(_pJsonParser);
        if (spString->Parse())
        {
            _value = spString->Get();
            return ParseWhitespace();
        }

        auto spObject = std::make_shared<JsonObject>(_pJsonParser);
        if (spObject->Parse())
        {
            _spObject = std::move(spObject);
            return ParseWhitespace();
        }

        auto spArray = std::make_shared<JsonArray>(_pJsonParser);
        if (spArray->Parse())
        {
            _spArray = std::move(spArray);
            return ParseWhitespace();
        }

        auto spNumber = std::make_shared<JsonNumber>(_pJsonParser);
        if (spNumber->Parse())
        {
            _spNumber = std::move(spNumber);
            _value = _spNumber->GetValue();
            return ParseWhitespace();
        }

        return false;
    }

    IJsonValuePtr JsonValue::At(size_t index)
    {
        if (_spArray)
        {
            return std::dynamic_pointer_cast<IJsonValue>(_spArray->At(index));
        }
        else
            return nullptr;
    }

    IJsonObjectPtr JsonValue::AddObject()
    {
        _spObject = std::make_shared<JsonObject>(_pJsonParser);
        _spArray.reset();
        _spNumber.reset();
        return std::dynamic_pointer_cast<IJsonObject>(_spObject);
    }

    IJsonArrayPtr JsonValue::AddArray()
    {
        _spArray = std::make_shared<JsonArray>(_pJsonParser);
        _spObject.reset();
        _spNumber.reset();
        return std::dynamic_pointer_cast<IJsonArray>(_spArray);
    }

    void JsonValue::AddValue(JsonValueVariant variantValue)
    {
        MakeStdStrings(variantValue);

        _spArray.reset();
        _spObject.reset();
        _spNumber.reset();

        if (std::holds_alternative<double>(variantValue))
        {
            double value = std::get<double>(variantValue);
            _spNumber = std::make_shared<JsonNumber>(_pJsonParser, value);
        }
        else if (std::holds_alternative<int32_t>(variantValue))
        {
            int32_t value = std::get<int32_t>(variantValue);
            _spNumber = std::make_shared<JsonNumber>(_pJsonParser, value);
        }
        else if (std::holds_alternative<uint32_t>(variantValue))
        {
            uint32_t value = std::get<uint32_t>(variantValue);
            _spNumber = std::make_shared<JsonNumber>(_pJsonParser, value);
        }

        _value = variantValue;
    }

    void JsonValue::SetStringIsObject()
    {
        _stringIsObject = true;
    }

    void JsonValue::Save(std::vector<char>& data)
    {
        if (_spObject)
            _spObject->Save(data);
        else if (_spArray)
            _spArray->Save(data);
        else if (_spNumber)
            _spNumber->Save(data);
        else
        {
            if (std::holds_alternative<bool>(_value))
            {
                bool value = std::get<bool>(_value);
                AddString(data, value ? "true" : "false");
            }
            else if (std::holds_alternative<std::string>(_value))
            {
                std::string value = std::get<std::string>(_value);
                AddString(data, std::move(value), !_stringIsObject);
            }
        }
    }

    JsonValueVariant JsonValue::GetValue()
    {
        return _value;
    }

    ///////////////////////////////////////////////////////////////////////////////

    JsonString::JsonString(Json* pJsonParser)
        : JsonBase(pJsonParser)
    {
    }

    bool JsonString::Parse()
    {
        if (!MatchChar('"'))
            return false;

        if (MatchChar('"'))
            return true;

        while (true)
        {
            char nextChar = ' ';
            if (!GetNext(nextChar))
                return false;

            if (nextChar == '"')
                return true;

            if (nextChar == '\\')
            {
                if (!GetNext(nextChar))
                    return false;

                bool validEscapeChar = false;

                if ((nextChar == '"') ||
                    (nextChar == '\\') ||
                    (nextChar == '/') ||
                    (nextChar == 'b') ||
                    (nextChar == 'f') ||
                    (nextChar == 'n') ||
                    (nextChar == 'r') ||
                    (nextChar == 't'))
                {
                    validEscapeChar = true;
                    _value += '\\';
                    _value += nextChar;
                }
                else if (nextChar == 'u')
                {
                    _value += '\\';
                    _value += nextChar;
                    char hexDigits[4];
                    for (uint32_t i : { 0, 1, 2, 3 })
                    {
                        if (!GetNext(hexDigits[i]))
                            return false;
                        _value += hexDigits[i];
                    }
                }
                (void)validEscapeChar; // unused
            }
            else
            {
                _value += nextChar;
            }
        }
    }

    void JsonString::Save(std::vector<char>& data)
    {
        // TODO
    }


    ///////////////////////////////////////////////////////////////////////////////

    JsonWhitespace::JsonWhitespace(Json* pJsonParser)
        : JsonBase(pJsonParser)
    {
    }

    bool JsonWhitespace::Parse()
    {
        char nextChar;

        do
        {
            if (!GetNext(nextChar))
                return true;
        } while ((nextChar == ' ') || (nextChar == '\n') || (nextChar == '\r') || (nextChar == '\t'));

        Backup();
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////

    JsonNumber::JsonNumber(Json* pJsonParser)
    :   JsonBase(pJsonParser)
    ,   _isFloatingPoint(false)
    ,   _isSigned(false)
    {
    }

    JsonNumber::JsonNumber(Json* pJsonParser, double value)
    :   JsonBase(pJsonParser)
    ,   _value(value)
    ,   _isFloatingPoint(true)
    ,   _isSigned(true)
    {
        _valueString = std::to_string(value);

        _valueString.erase(_valueString.find_last_not_of('0') + 1, std::string::npos);
		if (Right(_valueString, 1) == ".")
			_valueString += "0";
    }

    JsonNumber::JsonNumber(Json* pJsonParser, int32_t value)
    :   JsonBase(pJsonParser)
    ,   _value(value)
    ,   _isFloatingPoint(false)
    ,   _isSigned(true)
    {
        _valueString = std::to_string(value);
    }

    JsonNumber::JsonNumber(Json* pJsonParser, uint32_t value)
    :   JsonBase(pJsonParser)
    ,   _value(value)
    ,   _isFloatingPoint(false)
    ,   _isSigned(false)
    {
        _valueString = std::to_string(value);
    }

    bool JsonNumber::ParseFraction()
    {
        if (!MatchChar('.'))
            return false;

        _isFloatingPoint = true;
        _isSigned = true;

        char nextChar = ' ';
        if (!GetNext(nextChar))
            return false;
        if ((nextChar < '0') || (nextChar > '9'))
            return false;

        while (true)
        {
            if (!GetNext(nextChar))
                return true;
            if ((nextChar < '0') || (nextChar > '9'))
            {
                Backup();
                return true;
            }
        }
    }

    bool JsonNumber::ParseExponent()
    {
        bool matchE = MatchChar('e');
        if (!matchE)
            matchE = MatchChar('E');

        if (!matchE)
            return false;

        _isFloatingPoint = true;
        _isSigned = true;

        bool matchSign = MatchChar('-');
        if (!matchSign)
            matchSign = MatchChar('+');

        char nextChar = ' ';
        if (!GetNext(nextChar))
            return false;
        if ((nextChar < '0') || (nextChar > '9'))
            return false;

        while (true)
        {
            if (!GetNext(nextChar))
                return true;
            if ((nextChar < '0') || (nextChar > '9'))
            {
                Backup();
                return true;
            }
        }
    }

    bool JsonNumber::Parse()
    {
        const char* startPosition = GetPosition();
        if (MatchChar('-'))
            _isSigned = true;

        bool match0 = MatchChar('0');

        if (!match0)
        {
            char nextChar = ' ';
            if (!GetNext(nextChar))
            {
                return BackupToStart(startPosition);
            }

            if ((nextChar < '1') || (nextChar > '9'))
            {
                return BackupToStart(startPosition);
            }

            if (!GetNext(nextChar))
                return false;

            bool numeratorEnd = false;
            if ((nextChar < '0') || (nextChar > '9'))
            {
                Backup();
                numeratorEnd = true;
            }

            while (!numeratorEnd)
            {
                if (!GetNext(nextChar))
                    return true;
                if ((nextChar < '0') || (nextChar > '9'))
                {
                    Backup();
                    numeratorEnd = true;
                }
            }
        }

        ParseFraction();
        ParseExponent();

        const char* endPosition = GetPosition();
        _valueString = GetString(startPosition, endPosition);

        if (_isFloatingPoint)
            _value = std::stod(_valueString);
        else if (_isSigned)
            _value = std::stoi(_valueString);
        else
            _value = static_cast<uint32_t>(std::stoul(_valueString));

        return true;
    }

    std::string JsonNumber::GetValueString()
    {
        return _valueString;
    }

    void JsonNumber::Save(std::vector<char>& data)
    {
        AddString(data, _valueString);
    }

} // namespace AtUtils
