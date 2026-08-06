/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AppToolkitStringUtils.h"
#include <cstdarg>
#include <cstring>
#include <cinttypes>

namespace AtUtils
{
#ifdef __linux__
    static int _vscprintf(const char* format, va_list argptr)
    {
        // Create a local copy of va_list
        // as it might be altered in vfwprintf
        std::va_list ap_local{};
        va_copy(ap_local, argptr);
        int ret = (vsnprintf(0, 0, format, ap_local));
        va_end(ap_local);

        return ret;
    }
#endif

    std::string FormatString(const char* format, ...)
    {
        if (!format)
            return "";

        va_list args;
        va_start(args, format);
        size_t formattedLength = _vscprintf(format, args);
        char* buffer = new char[formattedLength + 1];
        std::string formatted_string;
        if (buffer)
        {
#ifdef __linux__
            vsprintf(buffer, format, args);
#else
            vsprintf_s(buffer, formattedLength + 1, format, args);
#endif
            formatted_string = buffer;
        }
        va_end(args);
        delete[] buffer;
        return formatted_string;
    }

    template<>
    bool FromString(const std::string& stringValue, bool& value)
    {
        if ((stringValue == "true") || (stringValue == "1"))
        {
            value = true;
            return true;
        }
        else if ((stringValue == "false") || (stringValue == "0"))
        {
            value = false;
            return true;
        }

        value = false;
        return false;
    }

    std::string ToString(bool value)
    {
        return value ? "true" : "false";
    }

    std::string ToString(float value)
    {
        return FormatString("%f", value);
    }

    std::string ToString(double value, const char* format_str)
    {
        return FormatString(format_str, value);
    }

    std::string ToString(uint32_t value)
    {
        return FormatString("%" PRIu32, value);
    }

    std::string ToString(int32_t value)
    {
        return FormatString("%" PRIi32, value);
    }

    std::string ToString(uint64_t value)
    {
        return FormatString("%" PRIu64, value);
    }

    std::string ToString(int64_t value)
    {
        return FormatString("%" PRIi64, value);
    }

    void MakeLower(std::string& stringValue)
    {
        for (auto& c : stringValue)
            c = std::tolower(c);
    }

    void MakeUpper(std::string& stringValue)
    {
        for (auto& c : stringValue)
            c = std::toupper(c);
    }

    std::string Left(const std::string& stringValue, size_t nCharacters)
    {
        if (stringValue.length() <= nCharacters)
            return stringValue;

        std::string result = stringValue.substr(0, nCharacters);
        return result;
    }

    std::string Mid(const std::string& stringValue, size_t position, size_t nCharacters)
    {
        size_t length = stringValue.length();

        if (position >= length)
            return "";

        if ((position + nCharacters) > length)
            nCharacters = length - position;

        std::string result = stringValue.substr(position, nCharacters);
        return result;
    }

    std::string Mid(const std::string& stringValue, size_t position)
    {
        size_t length = stringValue.length();

        if (position >= length)
            return "";

        size_t nCharacters = length - position;

        std::string result = stringValue.substr(position, nCharacters);
        return result;
    }

    std::string Right(const std::string& stringValue, size_t nCharacters)
    {
        if (stringValue.length() <= nCharacters)
            return stringValue;

        size_t start_pos = stringValue.length() - nCharacters;
        std::string result = stringValue.substr(start_pos, nCharacters);
        return result;
    }

    bool Replace(std::string& stringValue, const char* findString, const char* replaceString)
    {
        if (!findString || !replaceString)
        {
            return false;
        }

        bool replaced = false;
        std::string searchString(findString);

        if (searchString.empty()) // Prevent infinite loop
        {
            return false;
        }

        size_t search_str_len = searchString.length();
        size_t replace_str_len = strlen(replaceString);
        size_t start_pos = 0;
        while (true)
        {
            size_t pos = stringValue.find(searchString, start_pos);
            if (pos == std::string::npos)
                return replaced;

            stringValue.replace(pos, search_str_len, replaceString);
            start_pos = pos + replace_str_len;	// start searching after replacement position or we get lockup!
            replaced = true;
        }
    }

    void TrimLeft(std::string& stringValue)
    {
        size_t spaceCount = 0;

        for (auto c : stringValue)
        {
            if (isspace(c))
                spaceCount++;
            else
                break;
        }

        stringValue = Mid(stringValue, spaceCount);
    }

    void TrimRight(std::string& stringValue)
    {
        size_t len = stringValue.length();
        size_t spaceCount = 0;

        if (len == 0)
            return;

        for (size_t i = 0; i < len; i++)
        {
            if (isspace(stringValue.at(len - i - 1)))
                spaceCount++;
            else
                break;
        }

        stringValue = Left(stringValue, len - spaceCount);
    }

    void MakeSafeXml(std::string& stringValue)
    {
        Replace(stringValue, "&", "&amp;");
        Replace(stringValue, "<", "&lt;");
        Replace(stringValue, ">", "&gt;");
        Replace(stringValue, "\"", "&quot;");
        Replace(stringValue, "'", "&apos;");
        Replace(stringValue, "\n", "");
    }

    void CopyString(char* destination, size_t destinationLength, const char* source)
    {
        size_t i = 0;
        for (; i < destinationLength; i++)
        {
            destination[i] = source[i];
            if (destination[i] == 0)
                return;
        }
    }

    void CopyMemoryBuffer(void* destination, size_t destinationLength, const void* source, size_t sourceLength)
    {
        size_t copyLength = (sourceLength < destinationLength) ? sourceLength : destinationLength;
        uint8_t* pDestination = (uint8_t*)destination;
        uint8_t *pSource = (uint8_t*)source;
        for (size_t i = 0; i < copyLength; i++)
            pDestination[i] = pSource[i];
    }

    void MemorySet(void* destination, size_t destinationLength, int value)
    {
        uint8_t* pDestination = (uint8_t*)destination;
        for (size_t i = 0; i < destinationLength; i++)
            pDestination[i] = (uint8_t)value;
    }

    std::wstring ToStringW(const std::string& inputString)
    {
        std::wstring outputString;
        outputString.resize(inputString.length());
        for (size_t i = 0, end = inputString.length(); i < end; i++)
        {
            wchar_t wchar = inputString[i];
            outputString[i] = wchar;
        }

        return outputString;
    }

} // namespace AtUtils
