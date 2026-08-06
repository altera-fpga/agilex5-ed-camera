/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "HapiCore.h"
#include "HapiItem.h"
#include <filesystem>
#include <map>
#include <fstream>

namespace Hapi
{
    ///////////////////////////////////////////////////////////////////////////
    //std::map<DriverFrameworkType, std::weak_ptr<IHapi>> IHapi::_wpHapiCoreInstances;
    std::multimap<DriverFrameworkType, std::weak_ptr<IHapi>> IHapi::_wpHapiCoreInstances;

    IHapi::~IHapi()
    {
    }

    ///////////////////////////////////////////////////////////////////////////

    HapiCore::HapiCore(DriverFrameworkType driverFrameworkType)
    :   _driverFrameworkType(driverFrameworkType)
    {
#ifdef WIN32
        _isWindows = true;
#else
        _isWindows = false;
#endif
    }

    HapiCore::~HapiCore()
    {
    }

    ILogCallback* HapiCore::_pLogCallback = nullptr;

    void HapiCore::SetLogCallback(ILogCallback* pLogCallback)
    {
        _pLogCallback = pLogCallback;
    }

    uint64_t HapiCore::ReadValueFromFile(const std::filesystem::path& path)
    {
        std::string line = ReadStringFromFile(path);
        return std::stoull(line, nullptr, 16);
    }

    std::string HapiCore::ReadStringFromFile(const std::filesystem::path& path)
    {
        std::ifstream inputStream(path);
        if (inputStream.good())
        {
            std::string line;
            std::getline(inputStream, line);
            return line;
        }
        return "";
    }


} // namespace Hapi

extern "C" uint32_t HapiReadRegister(void* hapi_item, uint32_t registerIndex)
{
    Hapi::HapiItemBase* pHapiItemBase = (Hapi::HapiItemBase*)hapi_item;
    uint32_t value = pHapiItemBase->ReadRegister(registerIndex);
    return value;
}

extern "C" void HapiWriteRegister(void* hapi_item, uint32_t registerIndex, uint32_t registerValue)
{
    Hapi::HapiItemBase* pHapiItemBase = (Hapi::HapiItemBase*)hapi_item;
    pHapiItemBase->WriteRegister(registerIndex, registerValue);
}