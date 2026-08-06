/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "ILut3d.h"
#include <vector>

namespace SwApi
{
    namespace Lut3d
    {
        class Lut3dImplementation : public SwApi::ILut3d
        {
        public:
            Lut3dImplementation( Hapi::Vvp3dLutPtr spVvp3dLut);            

            void SetEnable(bool enable) override;
            bool GetEnable() override;
            uint8_t GetNumBuffers() override;
            bool SetActiveBuffer(const uint8_t bufferId) override;
            uint8_t GetActiveBuffer() override;
            uint8_t GetLutDimension() override;
            bool LoadLutFromCubeFile(const std::string& lutName, const std::filesystem::path& cubeFile, bool resizeIfNeeded = true) override;
            size_t GetNumValidBuffers() override;
            std::string GetLutName(size_t index) override;

        private:
            Hapi::Vvp3dLutPtr _spVvp3dLut;
            bool _enabled;
            uint8_t _activeBuffer;
            std::vector<std::string> _lutNames;
            std::size_t _validBuffers;
        };

    } // namespace Lut3d
} // namespace SwApi