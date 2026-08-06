/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "Hapi.h"
#include <iostream>
#include <filesystem>

namespace Hapi
{

    class HapiCore : public IHapi, public std::enable_shared_from_this<HapiCore>
    {
    public:
        HapiCore(DriverFrameworkType driverFrameworkType);
        ~HapiCore();

        void SetLogCallback(ILogCallback* pLogCallback) override;
        DriverFrameworkType GetDriverFrameworkType() override { return _driverFrameworkType; }

    protected:
        bool _isWindows;
        DriverFrameworkType _driverFrameworkType;

    public:
        static ILogCallback* _pLogCallback;

        static uint64_t ReadValueFromFile(const std::filesystem::path& path);
        static std::string ReadStringFromFile(const std::filesystem::path& path);
    };

} // namespace Hapi
