/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvp3dLut.h"
#include <filesystem>

namespace SwApi
{
    class ILut3d
    {
    public:
        static std::shared_ptr<ILut3d> Create(Hapi::Vvp3dLutPtr sp3dLut);
        virtual ~ILut3d() {}

        virtual void SetEnable(bool enable) = 0;
        virtual bool GetEnable() = 0;
        virtual uint8_t GetNumBuffers() = 0;
        virtual bool SetActiveBuffer(const uint8_t bufferId) = 0;
        virtual uint8_t GetActiveBuffer() = 0;
        virtual uint8_t GetLutDimension() = 0;
        virtual bool LoadLutFromCubeFile(const std::string& lutName, const std::filesystem::path& cubeFile, bool resizeIfNeeded = true) = 0;
        virtual size_t GetNumValidBuffers() = 0;
        virtual std::string GetLutName(size_t index) = 0;
    };
} // namespace SwApi