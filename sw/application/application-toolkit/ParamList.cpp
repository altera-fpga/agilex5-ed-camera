/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "ParamList.h"
#include "CommonUiLayer.h"
#include "AtUtils.h"

ParamList::ParamList(std::string full_string, uint32_t clientID)
:	_clientID(clientID)
,	_chainedDialog(false)
{
    while (true)
    {
        size_t pos = full_string.find(",");
        if (pos == std::string::npos)
        {
            if (full_string != "")
                AddParam(std::move(full_string));
            return;
        }

        AddParam(AtUtils::Left(full_string, pos));
        full_string = AtUtils::Mid(full_string, pos + 1);

        if (full_string == "")
        {
            push_back("");
            return;
        }
    }
}

void ParamList::AddParam(std::string parameter)
{
    AtUtils::Replace(parameter, "&#39;", "'");
    push_back(parameter);
}

bool ParamList::CheckNumParams(size_t n)
{
    size_t count = size();
    return (n == count);
}

std::string ParamList::GetParam(size_t n)
{
    if (n < size())
        return at(n);
    else
        return "";
}

void ParamList::SetBaseCommand(std::string& base_name)
{
    _base_command = base_name;
}

std::string ParamList::ExtractBaseCommand()
{
    if (!_base_command.empty())
        return _base_command;

    std::string base_command;
    if (size() > 0)
    {
        base_command = front();
        erase(begin());
    }

    return base_command;
}

void ParamList::Dump()
{
    for (size_t i = 0; i < size(); i++)
        std::cout << "[" << i << "] '" << at(i) << "'\n";
}

std::string ParamList::GetCSV()
{
    std::string csv_string;
    for (size_t i = 0, n = size(); i < n; i++)
    {
        csv_string += at(i);
        if (i < (n - 1))
            csv_string += ",";
    }

    return csv_string;
}

void ParamList::SetJsonData(std::shared_ptr<AtUtils::IJson>& command_doc,
                           AtUtils::IJsonObjectPtr data)
{
    // Since ParamList can be delegated across threads, we need
    // to keep the pugi::xml_document so that _data doesn't go out
    // of scope
    _command_doc = std::move(command_doc);
    _data = std::move(data);
}

AtUtils::IJsonObjectPtr ParamList::GetXmlData()
{
    return _data;
}

uint32_t ParamList::GetClientID()
{
    return _clientID;
}

///////////////////////////////////////////////////////////////////////////////

ONamedParamList::ONamedParamList(AtUtils::IJsonObjectPtr paramsNode, uint32_t clientID)
:	_clientID(clientID)
,	_paramsNode(paramsNode)
{
    for (size_t i = 0, n = paramsNode->GetNumMembers(); i < n; i++)
    {
        auto spMember = paramsNode->GetMember(i);
        if (spMember)
        {
            std::string name = spMember->GetName();
            std::string value;
            auto spValue = spMember->GetValue();
            AtUtils::JsonValueVariant valueVariant = spValue->GetValue();

            // Variants are: std::string, int32_t, uint32_t, double, bool
            if (std::holds_alternative<std::string>(valueVariant))
            {
                value = std::get<std::string>(valueVariant);
            }
            else if (std::holds_alternative<int32_t>(valueVariant))
            {
                auto containedValue = std::get<int32_t>(valueVariant);
                value = std::to_string(containedValue);
            }
            else if (std::holds_alternative<uint32_t>(valueVariant))
            {
                auto containedValue = std::get<uint32_t>(valueVariant);
                value = std::to_string(containedValue);
            }
            else if (std::holds_alternative<double>(valueVariant))
            {
                auto containedValue = std::get<double>(valueVariant);
                value = std::to_string(containedValue);
            }
            else if (std::holds_alternative<bool>(valueVariant))
            {
                auto containedValue = std::get<bool>(valueVariant);
                value = std::to_string(containedValue);
            }
            else
                break;

            (*this)[name] = std::move(value);
        }
    }
}

ONamedParamList::ONamedParamList()
:	_clientID(0)
{
}

bool ONamedParamList::CheckNumParams(size_t n)
{
    return (n == size());
}

std::string ONamedParamList::GetParam(const char* name)
{
    std::string key_string(name);
    std::string value;
    AtUtils::Lookup(*this, std::move(key_string), value);
    return value;
}

void ONamedParamList::SetBaseCommand(std::string& baseName)
{
    _baseCommand = baseName;
}

uint32_t ONamedParamList::GetClientID()
{
    return _clientID;
}

std::vector<NamedParamPair> ONamedParamList::GetParamPairs()
{
    std::vector<NamedParamPair> pairs;

    for (auto i = begin(); i != end(); i++)
        pairs.push_back(NamedParamPair(i->first, i->second));

    return pairs;
}

void ONamedParamList::Add(std::string name, std::string value)
{
    (*this)[name] = std::move(value);
}

void ONamedParamList::Add(std::string name, const char* value)
{
    (*this)[name] = value;
}

void ONamedParamList::Add(std::string name, int value)
{
    (*this)[name] = AtUtils::ToString(value);
}

void ONamedParamList::Add(std::string name, uint32_t value)
{
    (*this)[name] = AtUtils::ToString(value);
}

void ONamedParamList::Add(std::string name, double value)
{
    (*this)[name] = AtUtils::ToString(value);
}

void ONamedParamList::Add(std::string name, bool value)
{
    (*this)[name] = AtUtils::ToString(value);
}

