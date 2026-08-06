/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspProfileVignetteCorrectionControls.h"
#include "UiElements.h"

#include <string>
#include <array>
#include <cmath>

VvpIspProfileVignetteCorrectionControls::VvpIspProfileVignetteCorrectionControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                                                                 const std::shared_ptr<WhiteBalanceController>& spWBController,
                                                                                 const std::shared_ptr<SwApi::Vc>& spVc,
                                                                                 const std::shared_ptr<SwApi::HistogramStats>& spHs)
: _spProfile(spProfile),
  _spWBController(spWBController),
  _spVc(spVc),
  _spHs(spHs)
{

}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileVignetteCorrectionControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Vignette Correction Calibration", GetSettingsSectionName());

    auto modeselectCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
    {
        _selectedMode = (MeshGenerationMode)selected._userItemData;
    };

    std::vector<UiEnumOption> modeVec = {
        {"Sampled Correction", MeshGenerationMode::SampleCorrect},
        {"DEMO: Amplify Vignette", MeshGenerationMode::DemoAmplifyVignette},
        {"DEMO: Horizontal Gradient", MeshGenerationMode::DemoHorizontalGradient},
        {"DEMO: Vertical Gradient", MeshGenerationMode::DemoVerticalGradient}
    };

    spContainer->AddEnumControl(
        "Select Mesh Mode", modeVec, modeselectCB, "vcModeSelectDropdown", 0);

    auto meshXDimCB = [this] (uint32_t clientID, int32_t& value)
    {
        _setXDim = value;
        _spGenerateButton->Enable((_setXDim * _setYDim) <= _spVc->GetMaxGainMeshPointsAllowed());
    };
    spContainer->AddSliderControl("Mesh X Dimension", _setXDim, 9, 33, meshXDimCB);

    auto meshYDimCB = [this] (uint32_t clientID, int32_t& value)
    {
        _setYDim = value;
        _spGenerateButton->Enable((_setXDim * _setYDim) <= _spVc->GetMaxGainMeshPointsAllowed());
    };
    spContainer->AddSliderControl("Mesh Y Dimension", _setYDim, 9, 33, meshYDimCB);


    auto genMeshCB = [this] (uint32_t clientID)
    {
        GenerateAndApplyVCMesh();
    };
    _spGenerateButton = spContainer->AddButtonControl("Generate Mesh", genMeshCB);

    return {std::move(spContainer)};
}

void VvpIspProfileVignetteCorrectionControls::GenerateAndApplyVCMesh()
{
    // First, generate a step mesh

    if (!_spVc->GetBypass())
    {
        _spVc->SetBypass(true);
    }

    SwApi::VcMeshUtils::VcStepMesh stepMesh;
    SwApi::VcMeshUtils::VcStepSampleCoordMesh sampleMesh;

    const auto [imageWidth, imageHeight] = _spVc->GetResolution();
    uint8_t pip = _spVc->GetPip();

    std::tie(stepMesh, sampleMesh) = SwApi::VcMeshUtils::GenerateStepMesh(imageWidth, imageHeight, pip, _setXDim, _setYDim);

    /*
    std::cout << "Step mesh:\n";
    for (int y = 0; y < _setYDim; y++)
    {
        for (int x = 0; x < _setXDim; x++)
        {
            std::cout << std::hex << stepMesh[(y*_setXDim) + x] << ",";
        }
        std::cout << "\n";
    }
    std::cout << std::dec;
    */

    SwApi::VcMeshUtils::VcCpFloatMesh floatMesh;

    // Depending on mode, this function does VERY different things
    switch (_selectedMode)
    {
        case MeshGenerationMode::SampleCorrect:
        {
            floatMesh = SampleScreenWithHS(imageWidth, imageHeight, std::move(sampleMesh));
            break;
        }
        case MeshGenerationMode::DemoAmplifyVignette:
        {
            // Vignette amplification is created by generated 2 sin waves that run for 1/2 a period and are clipped at the top
            auto clipSin = [this](float position)
            {
                float rad = position * (3.141592);
                float sin_val = sinf(rad) * 1.2;
                return (sin_val < 1) ? (sin_val) : (1);
            };

            for (uint32_t y = 0; y < _setYDim; y++)
            {
                for (uint32_t x = 0; x < _setXDim; x++)
                {
                    float sin_val = clipSin(sampleMesh[y]) * clipSin(sampleMesh[_setYDim + x]);
                    floatMesh.push_back(sin_val);
                }
            }
            break;
        }
        case MeshGenerationMode::DemoHorizontalGradient:
        {
            for (uint32_t y = 0; y < _setYDim; y++)
            {
                for (uint32_t x = 0; x < _setXDim; x++)
                {

                    floatMesh.push_back(sampleMesh[_setYDim + x] * sampleMesh[_setYDim + x]);
                }
            }
            break;
        }
        case MeshGenerationMode::DemoVerticalGradient:
        {
            for (uint32_t y = 0; y < _setYDim; y++)
            {
                for (uint32_t x = 0; x < _setXDim; x++)
                {
                    floatMesh.push_back(sampleMesh[y] * sampleMesh[y]);
                }
            }
            break;
        }
    }

    // For the demo, we use bayer stuff, which belies a different colour plane for each bayer channel

    SwApi::VcMeshUtils::VcCpMesh quantised_mesh = SwApi::VcMeshUtils::quantizeMeshToFixedPoint8i11f(floatMesh);

    _spProfile->LoadVCConfigIntoProfile(_setXDim, _setYDim, 
                                        imageWidth, imageHeight, 4,
                                        &quantised_mesh,
                                        &quantised_mesh,
                                        &quantised_mesh,
                                        &quantised_mesh,
                                        std::move(stepMesh));

    _spWBController->ApplyVCMeshes();
    usleep(20000);
    _spWBController->ApplyVCMeshes();

    if (_spVc->GetBypass())
    {
        _spVc->SetBypass(false);
    }

    if (_fpUpdateOverviewUiCB)
    {
        _fpUpdateOverviewUiCB();
    }
}


SwApi::VcMeshUtils::VcCpFloatMesh VvpIspProfileVignetteCorrectionControls::SampleScreenWithHS(uint16_t width, uint16_t height, SwApi::VcMeshUtils::VcStepSampleCoordMesh sampleMesh)
{
    uint16_t numXSamples = _setXDim * 2;
    uint16_t numYSamples = _setYDim * 2;

    uint16_t totalSamples = numXSamples * numYSamples;

    uint8_t numIterationsToAverageOver = 10;

    uint16_t xSampleWidth = width / numXSamples;
    uint16_t xSampleHeight = height / numYSamples;

    float overallMean = 1.0;
    float sampleGrid[numYSamples][numXSamples];

    _spVc->SetBypass(true);

    usleep(6000); // Wait for the bypass to be enabled

    std::cout << "Progress:\n";

    for (int y = 0; y < numYSamples; y++)
    {
        for (int x = 0; x < numXSamples; x++)
        {
            sampleGrid[y][x] = 0;

            SwApi::Hs::RegionOfInterest roi;
            roi.h_start = x * xSampleWidth;
            roi.h_end = ((x + 1) * xSampleWidth) - 1;
            roi.v_start = y * xSampleHeight;
            roi.v_end = ((y + 1) * xSampleHeight) - 1;
            _spHs->SetRegionOfInterest(roi);
            for (int i = 0; i < numIterationsToAverageOver; i++)
            {
                auto hsResults = _spHs->GetHistogram();

                if((hsResults == nullptr) || (hsResults->valid == false))
                {
                    i--;
                    continue;
                }

                sampleGrid[y][x] += hsResults->roi_luma_bins.GetMeanBin();
            }
            sampleGrid[y][x] /= numIterationsToAverageOver;
        }
        std::cout << ((float) ((y * numXSamples)) / totalSamples) * 100 << "%\n";
    }

    std::cout << "\nSampleGrid:\n";
    for (int y = 0; y < numYSamples; y++)
    {
        for (int x = 0; x < numXSamples; x++)
        {
            std::cout << sampleGrid[y][x] << ", ";
        }
        std::cout << "\n";
    }

    for (int y = 0; y < numYSamples; y++)
    {
        for (int x = 0; x < numXSamples; x++)
        {
            overallMean += sampleGrid[y][x];
        }
    }
    overallMean /= totalSamples;

    std::cout << "Full frame mean bin: " << overallMean << "\n";

    for (int y = 0; y < numYSamples; y++)
    {
        for (int x = 0; x < numXSamples; x++)
        {
            sampleGrid[y][x] = overallMean / sampleGrid[y][x];
        }
    }

    /*
        Now for some arduous geometry
    */

    float sampleMeshXOffset = (1.0) / (numXSamples * 2);
    float sampleMeshYOffset = (1.0) / (numYSamples * 2);
    float sampleMeshXGap = (1.0) / numXSamples;
    float sampleMeshYGap = (1.0) / numYSamples;

    float meshMinX = sampleMeshXOffset;
    float meshMinY = sampleMeshYOffset;

    float meshMaxX = sampleMeshXOffset + ((numXSamples - 1) * sampleMeshXGap);
    float meshMaxY = sampleMeshYOffset + ((numYSamples - 1) * sampleMeshYGap);


    std::cout << "sampleMeshXOffset: " << sampleMeshXOffset << "\n";
    std::cout << "sampleMeshYOffset: " << sampleMeshYOffset << "\n";
    std::cout << "sampleMeshXGap: " << sampleMeshXGap << "\n";
    std::cout << "sampleMeshYGap: " << sampleMeshYGap << "\n";
    std::cout << "meshMinX: " << meshMinX << "\n";
    std::cout << "meshMinY: " << meshMinY << "\n";
    std::cout << "meshMaxX: " << meshMaxX << "\n";
    std::cout << "meshMaxY: " << meshMaxY << "\n";

    SwApi::VcMeshUtils::VcCpFloatMesh floatMesh;

    for (uint32_t y = 0; y < _setYDim; y++)
    {
        for (uint32_t x = 0; x < _setXDim; x++)
        {
            float ySamplePos = sampleMesh[y];
            float xSamplePos = sampleMesh[_setYDim + x];

            std::cout << "Sampling: (" << xSamplePos << ", " << ySamplePos << ")\n";

            // TODO - proper extrapolation of the mesh. For now though, just clamp to mesh boundaries

            if (xSamplePos < meshMinX)
            {
                xSamplePos = meshMinX;
            } 
            else if (xSamplePos > meshMaxX)
            {
                xSamplePos = meshMaxX;
            }

            if (ySamplePos < meshMinY)
            {
                ySamplePos = meshMinY;
            } 
            else if (ySamplePos > meshMaxY)
            {
                ySamplePos = meshMaxY;
            }

            std::cout << "Clamped Sampling: (" << xSamplePos << ", " << ySamplePos << ")\n";

            uint16_t gridX = (xSamplePos - sampleMeshXOffset) / sampleMeshXGap;
            if (gridX == numXSamples - 1)
            {
                gridX -= 1;
            }
            float gridXF = (gridX * sampleMeshXGap) + sampleMeshXOffset;
            uint16_t gridY = (ySamplePos - sampleMeshYOffset) / sampleMeshYGap;
            if (gridY == numYSamples - 1)
            {
                gridY -= 1;
            }
            float gridYF = (gridY * sampleMeshYGap) + sampleMeshYOffset;
    
            float x_interp_val = (xSamplePos - gridXF) / (sampleMeshXGap);
            float x_top_interp = (sampleGrid[gridY][gridX] * x_interp_val) + ((1 - x_interp_val) * sampleGrid[gridY][gridX + 1]);
            float x_bot_interp = (sampleGrid[gridY + 1][gridX] * x_interp_val) + ((1 - x_interp_val) * sampleGrid[gridY + 1][gridX + 1]);

            float y_interp_val = (ySamplePos - gridYF) / (sampleMeshYGap);

            float final_interp = (x_top_interp * y_interp_val) + (x_bot_interp * (1 - y_interp_val));

            std::cout << "Final: " << final_interp << "\n";

            floatMesh.push_back(final_interp);

            std::cout << "Grid coords: (" << gridX << ", " << gridY << ")\n";
        }
    }

    // Simple blur to reduce any obvious issues.
    SwApi::VcMeshUtils::VcCpFloatMesh outMesh;
    float accum = 0;
    int count = 0;
    for (uint32_t y = 0; y < _setYDim; y++)
    {

        for (uint32_t x = 0; x < _setXDim; x++)
        {
            accum = 0;
            count = 0;
            
            for (int h = (int)y - 1; h < (int)y + 2; h++)
            {
                if (h < 0 || h >= (int)_setYDim) continue;
                for (int w = (int)x - 1; w < (int)x + 2; w++)
                {
                    if (w < 0 || w >= (int)_setXDim) continue;
                    accum += floatMesh[(h * _setXDim) + w];
                    count++;
                }
            }

            accum /= count;
            outMesh.push_back(accum);
        }
    }

    std::cout << "\noutMesh:\n";
    for (uint32_t y = 0; y < _setYDim; y++)
    {
        for (uint32_t x = 0; x < _setXDim; x++)
        {
            std::cout << outMesh[(y * _setXDim) + x] << ", ";
        }
        std::cout << "\n";
    }


    return outMesh;
}

void VvpIspProfileVignetteCorrectionControls::LoadFunctionPointerForUpdatingOverviewUI(std::function<void(void)> fpUpdateOverviewUiCB)
{
    _fpUpdateOverviewUiCB = std::move(fpUpdateOverviewUiCB);
}
