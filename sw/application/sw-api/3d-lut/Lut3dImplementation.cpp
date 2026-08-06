/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Lut3dImplementation.h"
#include "LutContainer.h"
#include "intel_vvp_3d_lut.h"

#include <cmath>

namespace SwApi
{
    std::shared_ptr<ILut3d> ILut3d::Create(Hapi::Vvp3dLutPtr sp3dLut)
    {
        auto spLut3d = std::make_shared<Lut3d::Lut3dImplementation>(sp3dLut);
        return spLut3d;
    }

    namespace Lut3d
    {

        Lut3dImplementation::Lut3dImplementation(Hapi::Vvp3dLutPtr spVvp3dLut)
        :   _spVvp3dLut(spVvp3dLut)
        ,   _enabled(false)
        ,   _activeBuffer(0)
        ,   _lutNames(2)
        ,   _validBuffers(0)
        {
            intel_vvp_3d_lut_enable(_spVvp3dLut->GetInstance(), false);
        }

        void Lut3dImplementation::SetEnable(bool enable)
        {
            intel_vvp_3d_lut_enable(_spVvp3dLut->GetInstance(), enable);
            _enabled = enable;
        }

        bool Lut3dImplementation::GetEnable()
        {
            return _enabled;
        }

        uint8_t Lut3dImplementation::GetNumBuffers()
        {
            return intel_vvp_3d_lut_get_double_buffered(_spVvp3dLut->GetInstance()) ? 2 : 1;
        }

        bool Lut3dImplementation::SetActiveBuffer(const uint8_t bufferId)
        {
            if(kIntelVvp3dLutOk == intel_vvp_3d_lut_buffer_select(_spVvp3dLut->GetInstance(), bufferId))
                _activeBuffer = bufferId;
            else
                return false;

            return true;
        }

        uint8_t Lut3dImplementation::GetActiveBuffer()
        {
            return _activeBuffer;
        }

        uint8_t Lut3dImplementation::GetLutDimension()
        {
            return intel_vvp_3d_lut_get_dimension(_spVvp3dLut->GetInstance());
        }

        bool Lut3dImplementation::LoadLutFromCubeFile(const std::string& lutName,
                                                      const std::filesystem::path& cubeFile, 
                                                      bool resizeIfNeeded)
        {
            // Load to the back buffer
            const uint8_t nextBuffer = (_activeBuffer + 1) % GetNumBuffers();
            size_t firmwareDimension = static_cast<size_t>(GetLutDimension());
            LutContainer lutContainer(cubeFile);

            if (lutContainer.GetDimension() == 0)
            {
                return false;
            }
            else if ((lutContainer.GetDimension() != firmwareDimension) &&
                     (!resizeIfNeeded || !lutContainer.Resize(firmwareDimension)))
            {
                return false;
            }

            intel_vvp_3d_lut_instance *pInstance = _spVvp3dLut->GetInstance();
            
            size_t lutDepth = static_cast<size_t>(intel_vvp_3d_lut_get_lut_depth(pInstance));
            lutDepth = (1 << lutDepth) - 1;
            float fLutDepth = static_cast<float>(lutDepth);
            LutContainer::TableEntry tableEntry;

            for (size_t b = 0; b < firmwareDimension; b++)
            {
                for (size_t g = 0; g < firmwareDimension; g++)
                {
                    for (size_t r = 0; r < firmwareDimension; r++)
                    {
                        lutContainer.GetTableEntry(r, g, b, tableEntry);
                        intel_vvp_3d_lut_load(pInstance, r, g, b, nextBuffer,
                                              static_cast<uint16_t>(std::round(fLutDepth * tableEntry[0])),
                                              static_cast<uint16_t>(std::round(fLutDepth * tableEntry[1])),
                                              static_cast<uint16_t>(std::round(fLutDepth * tableEntry[2])), 0);
                    }
                }
            }

            // Auto switch the buffer            
            SetActiveBuffer(nextBuffer);
            _lutNames[nextBuffer] = lutName;

            _validBuffers = std::min((_validBuffers + 1), (size_t)GetNumBuffers());

            return true;
        }

        size_t Lut3dImplementation::GetNumValidBuffers()
        {
            return _validBuffers;
        }

        std::string Lut3dImplementation::GetLutName(size_t index)
        {
            if (index < _lutNames.size())
                return _lutNames[index];
            else
                return "";
        }
    } // namespace Lut3d
} // namespace SwApi