/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <stdint.h>
#include <memory>
#include "AppToolkitIJson.h"

class ParamList : public std::vector<std::string>
{
public:
    ParamList(std::string full_string, uint32_t clientId);
    bool CheckNumParams(size_t n);
    std::string GetParam(size_t n);
    std::string ExtractBaseCommand();
    void Dump();
    void SetBaseCommand(std::string& base_command);
    void SetJsonData(std::shared_ptr<AtUtils::IJson>& command_doc, AtUtils::IJsonObjectPtr data);
    AtUtils::IJsonObjectPtr GetXmlData();
    uint32_t GetClientID();
    bool ChainedDialogShown()            { return _chainedDialog; }
    void SetChainedDialog(bool state)    { _chainedDialog = state; }
    std::string GetCSV();

private:
    void AddParam(std::string parameter);

    std::string            _base_command;
    std::shared_ptr<AtUtils::IJson>    _command_doc; // Keep document alive so _data doesn't go out of scope
    AtUtils::IJsonObjectPtr _data;
    uint32_t            _clientID; // Identifies the source of the command
    bool                _chainedDialog;
};

using ParamListPtr = std::shared_ptr<ParamList>;

class NamedParamPair
{
public:
    NamedParamPair(std::string name, std::string value) : _name(std::move(name)), _value(std::move(value)) {}
    std::string _name;
    std::string _value;
};

class ONamedParamList : public std::unordered_map<std::string, std::string, std::hash<std::string>>
{
public:
    ONamedParamList(AtUtils::IJsonObjectPtr paramsObject, uint32_t clientID);
    ONamedParamList();
    bool CheckNumParams(size_t n);
    std::string GetParam(const char* name);
    void SetBaseCommand(std::string& baseCommand);
    uint32_t GetClientID();

    std::vector<NamedParamPair> GetParamPairs();
    void Add(std::string name, std::string value);
    void Add(std::string name, const char* value);
    void Add(std::string name, int value);
    void Add(std::string name, uint32_t value);
    void Add(std::string name, double value);
    void Add(std::string name, bool value);

private:
    std::string _baseCommand;
    uint32_t    _clientID; // Identifies the source of the command
    AtUtils::IJsonObjectPtr _paramsNode;
};
