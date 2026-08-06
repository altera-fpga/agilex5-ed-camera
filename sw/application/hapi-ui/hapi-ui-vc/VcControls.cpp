/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VcControls.h"
#include "UiElements.h"
#include "VcMeshUtils.h"
#include "intel_vvp_vc.h"

#include "IspCommon.h"

#include <filesystem>
#include <fstream>
#include <cmath>

#ifdef __linux__
#include <unistd.h>
#endif

#include <ranges>
#include <iterator>


using namespace SwApi;
using namespace VcMeshUtils;

VcControls::VcControls(const std::string& name, const std::shared_ptr<SwApi::Vc>& spVc, bool enableDebugUi)
    : _name(name)
    , _spVc(spVc)
    , _enableDebugUi(enableDebugUi)
{
}

std::string contentsOf(std::ifstream& f)
{
    std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return str;
}

std::vector<std::shared_ptr<UiControlContainer>> VcControls::AddUiElements()
{
    if (not _spVc) 
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    // region Bypass toggle button
    auto bypassCB = [this](uint32_t clientID, bool& val) { _spVc->SetBypass(val); };
    _spBypassControl = spContainer->AddBoolControl("Bypass", bypassCB, "BypassVC", _spVc->GetBypass());
    // endregion

    if (_enableDebugUi) {
        // region CFA Selector

        auto cfaCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void {
            auto chosenCfaType = static_cast<TCfaPhase>(selected._userItemData);
            _spVc->SetCfaPhase(chosenCfaType);
        };

        std::vector<UiEnumOption> cfaTypeOptions = { { "RGGB", static_cast<uint32_t>(TCfaPhase::RGGB) },
                                                     { "GRBG", static_cast<uint32_t>(TCfaPhase::GRBG) },
                                                     { "GBRG", static_cast<uint32_t>(TCfaPhase::GBRG) },
                                                     { "BGGR", static_cast<uint32_t>(TCfaPhase::BGGR) } };
        spContainer->AddEnumControl("Bayer pattern type",
                                    cfaTypeOptions,
                                    cfaCB,
                                    "CfaPhase",
                                    (uint32_t)_spVc->GetCfaPhase());

        // endregion
    }

    auto radiusCB = [&, this](uint32_t clientId, float& val) 
    {
        _radialDistance = val;
        GenerateAndApplyNewMesh();
    };

    _spRadialSlider = spContainer->AddSliderControl("Radial Vignette", 0.0f, 1.0f, radiusCB, "VcRadiusSlider", 0.0);


    auto lensStrengthCB = [this](uint32_t clientId, float& val) 
    {
        _coronaStrength = val;
        GenerateAndApplyNewMesh();
    };

    _spCoronaSlider = spContainer->AddSliderControl("Lens Corona", 0.0f, 1.0f, lensStrengthCB, "VcLensStrengthSlider", 0.0);

    auto invControlCB = [this](uint32_t clientId, bool& setting) 
    {
        _invertMesh = setting;
        GenerateAndApplyNewMesh();
    };
    _spInvertBool = spContainer->AddBoolControl("Invert", false, invControlCB);

    auto applyUnityFloatMeshCb = [this](uint32_t clientId) 
    {
        uint32_t meshWidth = _spVc->GetHorizontalNumBlocks() + 1;
        uint32_t meshHeight = _spVc->GetVerticalNumBlocks() + 1;
        VcCpMesh currentMesh = quantizeMeshToFixedPoint8i11f(GenerateUnityFMesh(meshWidth, meshHeight));

         const uint32_t numCp = _spVc->GetPerColorGainEnable() ? 4 : 1;

        // Upload the current mesh.
        for(uint32_t i = 0; i < numCp; i++) 
        {
            if (!_spVc->UploadMeshCpLut(i, currentMesh.data()))
            {
                std::cout << "[VC] Failed to upload mesh index " << i << "\n";
                return;
            }
        }  

        _radialDistance = 0.0f;
        _coronaStrength = 0.0f;
        _invertMesh = false;

        _spRadialSlider->UpdateValue(0.0f);
        _spCoronaSlider->UpdateValue(0.0f);
        _spInvertBool->UpdateValue(false);
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", applyUnityFloatMeshCb, "Reset", "OJL/Images/Reset.png" }
    });

    return {std::move(spContainer)};
}


void VcControls::GenerateAndApplyNewMesh()
{
    const uint32_t meshWidth = _spVc->GetHorizontalNumBlocks() + 1;
    const uint32_t meshHeight = _spVc->GetVerticalNumBlocks() + 1;

    const uint32_t midX = meshWidth / 2;
    const uint32_t midY = meshHeight / 2;

    const float maxDist = sqrtf(static_cast<float>(midX * midX + midY * midY));
    const float cleanRadius = maxDist / 2.0f;
    const float radialDistanceReciprocal = 1.0f - _radialDistance;
    const float maxDarkness = radialDistanceReciprocal * radialDistanceReciprocal * radialDistanceReciprocal * radialDistanceReciprocal;

    const std::size_t mesh_size = meshHeight * meshWidth;

    std::vector<std::pair<float,float>> mesh;
    mesh.reserve(mesh_size);

    const float distOffset = 1.0f - _radialDistance;

    float maxDiffFromUnity = 0.0f;
    float maxDiffScale = 1.0f;

    for(uint32_t y = 0; y < meshHeight; y++)
    {
        for(uint32_t x = 0; x < meshWidth; x++)
        {
            // Idea - for each point, work out the distance from the middle.
            // We do a scale of 0 - > dist - > maxDist.
            // This scale is offset by (1.0 - _radialDistance)
            //
            const int32_t dx = static_cast<int32_t>(midX) - static_cast<int32_t>(x);
            const int32_t dy = static_cast<int32_t>(midY) - static_cast<int32_t>(y);
            const float dist = sqrtf(static_cast<float>(dx * dx + dy * dy));

            const float distPower = dist / maxDist;

            float scaleValue = 1.0f;

            if (distPower > distOffset)
            {
                const float scaledDistPower = std::min((distPower - distOffset) / _radialDistance, 1.0f);
                scaleValue = (1.0f - scaledDistPower) + (scaledDistPower * maxDarkness);

                if(1.0f > scaleValue)
                {
                    const float diffFromUnity = 1.0f - scaleValue;
                    maxDiffFromUnity = std::max(diffFromUnity, maxDiffFromUnity);

                    const float diffScale = 1.0f / scaleValue;
                    maxDiffScale = std::max(diffScale, maxDiffScale);
                }
            }

            float corona_mesh_value = 1.0f;

            if(cleanRadius > dist)
            {
                const float dr_ratio = dist / cleanRadius;
                corona_mesh_value = (dr_ratio + ((1.0f - dr_ratio) * (1.0f + (3.0f * _coronaStrength))));
            }

            mesh.emplace_back(std::min(scaleValue, 1.0f), corona_mesh_value);
        }
    }

    maxDiffScale = std::min(maxDiffScale, 1.5f);

    VcCpFloatMesh combinedMesh;
    combinedMesh.reserve(mesh_size);

    auto combine_func = [maxDiffScale, maxDiffFromUnity](const std::pair<float, float>& v)->float {
        return (v.first * maxDiffScale + maxDiffFromUnity) * v.second;
    };

    std::ranges::transform(mesh, std::back_inserter(combinedMesh), combine_func);    

    if(!_invertMesh)
    {
        const auto max_val = *(std::ranges::max_element(combinedMesh));
        std::ranges::transform(combinedMesh, combinedMesh.begin(), [max_val](float v){ return max_val / v; });
    }

    VcCpMesh quantizedMesh = quantizeMeshToFixedPoint8i11f(combinedMesh);

    const uint32_t numCp = _spVc->GetPerColorGainEnable() ? 4 : 1;
    
    for(uint32_t i = 0; i < numCp; i++)
    {
        _spVc->UploadMeshCpLut(i, quantizedMesh.data());
    }
}
