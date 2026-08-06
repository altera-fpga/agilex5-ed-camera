/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "WhiteBalance.h"

#include "CoeffData.h"
#include "CoeffGen.h"

#include <cmath>
#include <linux/prctl.h>
#include <sys/prctl.h>

using namespace SwApi;

static const SwApi::WbsResultFormat _currentFormat = WbsResultFormat::RG_GB;

static const uint32_t baseWbcScale = 2048;



WhiteBalanceController::WhiteBalanceController(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                           const std::shared_ptr<SwApi::Blc>& spBlackLevelCorrect,
                           const std::shared_ptr<SwApi::Anr>& spAdaptiveNoiseReduction,
                           const std::shared_ptr<SwApi::Vc>& spVignetteCorrection,
                           const std::shared_ptr<SwApi::Wbs>& spWhiteBalanceStats,
                           const std::shared_ptr<SwitchRouter>& spWbsSwitchRouter,
                           const std::shared_ptr<SwApi::Wbc>& spWhiteBalanceCorrection,
                           const std::shared_ptr<SwApi::Ccm>& spColourCorrectionMatrix,
                           const char* const name)
: SwUtils::Thread(name),
  _spProfile(std::move(spProfile)),
  _awbMode(AWBMode::Automatic),
  _tint(0.0f),
  _tintStrength(1.0f),
  _spBlackLevelCorrect(std::move(spBlackLevelCorrect)),
  _spAdaptiveNoiseReduction(std::move(spAdaptiveNoiseReduction)),
  _spVignetteCorrection(std::move(spVignetteCorrection)),
  _spWhiteBalanceStats(std::move(spWhiteBalanceStats)),
  _spWbsSwitchRouter(std::move(spWbsSwitchRouter)),
  _spWhiteBalanceCorrection(std::move(spWhiteBalanceCorrection)),
  _spColourCorrectionMatrix(std::move(spColourCorrectionMatrix)),
  _spMultiChannelWhiteBalance(nullptr),
  _multiChannelWhiteBalanceHandle(0),
  _gain(1.0f),
  _roi{0,0,0,0}
{
    _measuredTempSamples.resize(_noMeasuredTempSamplesToAverage);
    StartThread();
}

WhiteBalanceController::~WhiteBalanceController()
{
    StopThread();
}

void WhiteBalanceController::SetMultiChannelWhiteBalance(const IMultiChannelWhiteBalancePtr& spMultiChannelWhiteBalance, uint32_t handle)
{
    _spMultiChannelWhiteBalance = spMultiChannelWhiteBalance;
    _multiChannelWhiteBalanceHandle = handle;
}

void WhiteBalanceController::SetAWBMode(AWBMode awbMode)
{
    _awbMode = awbMode;
    switch(awbMode)
    {
        case AWBMode::Disabled:
        {
            DeactivateCcmModifiers();

            GetCcm()->ApplyAWBTintMatrix(vvp::ccm::GenerateIdentityMatrix());
            break;
        }
        case AWBMode::ChooseTemperatureKelvin:
        {
            UpdateTint();
            break;
        }
        case AWBMode::ChooseLigthing:
        {
            UpdateTint();
            break;
        }
        case AWBMode::Automatic:
        {
            _noMeasuredTempSamplesToAverage = 10;
            UpdateTint();
            break;
        }
        case AWBMode::CustomPreset:
        {
            _noMeasuredTempSamplesToAverage = 4;
            UpdateTint();
            break;
        }
    }
}

void WhiteBalanceController::RunThread()
{
    prctl(PR_SET_NAME, _threadName.c_str(),0,0,0);
    while (!_shutdownEvent.IsSignalled())
    {
        AutoWhiteBalanceUpdateFunc();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void WhiteBalanceController::SetTint(float tint)
{
    _tint = tint;
    UpdateTint();
}

void WhiteBalanceController::SetTintStrength(float tintStrength)
{
    _tintStrength = tintStrength;
    auto ccm = GetCcm();
    if (ccm)
    {
        ccm->SetAWBInterpFactor(_tintStrength);
    }
}

void WhiteBalanceController::SetTemperature(uint16_t temp)
{
    _sceneTemp = temp;
    _targetTemp = temp;
    ApplyWhiteBalance(_sceneTemp, _targetTemp, TCfaPhase::RGGB);
}

void WhiteBalanceController::UpdateTint()
{
    auto ccm = GetCcm();
    if (ccm)
    {
        auto tintMatrix = vvp::ccm::GenerateIdentityMatrix();
        vvp::ccm::ModifyMatrixByTint(tintMatrix, _targetTemp, _tint);
        ccm->ApplyAWBTintMatrix(tintMatrix);
    }
}

void WhiteBalanceController::AutoWhiteBalanceUpdateFunc()
{
    static AWBMode awbModePrev = AWBMode::Disabled;

    switch (_awbMode)
    {
        default:
        case AWBMode::Disabled:
        case AWBMode::ChooseTemperatureKelvin:
        case AWBMode::ChooseLigthing:
        case AWBMode::CustomPreset:
        {
            break;
        }
       case AWBMode::Automatic:
        {
            // When White Balance mode changes to automatic
            // reset previous measure to make sure
            // White Balance controller updated immediatly
            if(awbModePrev != AWBMode::Automatic)
                _sceneTemp = 0;


            // This is SceneDetection, but with the target temp locked to the detected scene temp
            uint16_t sceneTemp = MesureCurrentSceneTemperature(_tempSampleCircularPointer == 0);

            if (sceneTemp == 0)
            {
                sceneTemp = 5700;
            }

            auto round_to_nearest = []<typename T>(const T v, const T n)->T {
                const T a = n ? ((v / n) * n) : v;  // Previous multiple
                const T b = a + n;                  // Next multiple
                return (v - a >= b - v) ? b : a;    // Return the closest
            };

            // Round to nearest 100 to try to smooth out fluctuations
            _measuredTempSamples[_tempSampleCircularPointer++] = round_to_nearest(sceneTemp, uint16_t{100});
            
            if (_tempSampleCircularPointer >= _noMeasuredTempSamplesToAverage)
            {
                _tempSampleCircularPointer = 0;
            }

            uint32_t avgTemp = 0;
            
            for (int i = 0; i < _noMeasuredTempSamplesToAverage; i++)
            {
                avgTemp += _measuredTempSamples[i];
            }
            
            avgTemp /= _noMeasuredTempSamplesToAverage;
            avgTemp = round_to_nearest(avgTemp, uint32_t{100});

            if(_spMultiChannelWhiteBalance != nullptr)
            {
                _spMultiChannelWhiteBalance->BalanceTemp(_multiChannelWhiteBalanceHandle, avgTemp);
            }

            if(avgTemp != _sceneTemp)
            {
                _sceneTemp = (uint16_t)avgTemp;
                _targetTemp = (uint16_t)avgTemp;

                ApplyWhiteBalance(_sceneTemp, _targetTemp, TCfaPhase::RGGB);
            }

            break;
        }
    }

    awbModePrev = _awbMode;
}


TCfaPhase WhiteBalanceController::GetWbcCfaPhase()
{
    return _spWhiteBalanceCorrection->GetCfaPhase();
}

void WhiteBalanceController::SetGain(float gain)
{
    _gain = gain;
}

void WhiteBalanceController::SetRoi(SwApi::WbsRoI roi)
{
    _roi = roi;
}

void WhiteBalanceController::ApplyBlackLevel()
{
    AutoWhiteBalanceGainTableEntry currentEntry;

    if (_spProfile->IsGainRepresentableInTable(_gain) == EntryRepresentationStatus::Invalid)
    {
        return;
    }

    currentEntry = _spProfile->GetInterpolatedTableEntryForGain(_gain);

    _spBlackLevelCorrect->SetPedestalsAndScalers(currentEntry._BlcPedestals, currentEntry._BlcScalars);

    if(_blackLevelCb)
        _blackLevelCb({
            currentEntry._BlcPedestals[0],
            currentEntry._BlcPedestals[1],
            currentEntry._BlcPedestals[2],
            currentEntry._BlcPedestals[3],
    });

    if (_spAdaptiveNoiseReduction)
    {
        auto lut = _spAdaptiveNoiseReduction->ApplyLuts(currentEntry._CombinedNoise, currentEntry._DarkNoise);
        (void)lut; // unused
    }
}

void WhiteBalanceController::DeactivateCcmModifiers()
{
    _spColourCorrectionMatrix->ApplyAWBMatrix(vvp::ccm::GenerateIdentityMatrix());
}

std::vector<float> WhiteBalanceController::CalculateRGBScalarsToMatchTempToTarget(uint16_t currentTemp, uint16_t targetTemp)
{
    std::vector<float> rgbScalars = {0, 0, 0};
    if (_spProfile->IsColourTempRepresentableInTable(currentTemp) == EntryRepresentationStatus::Invalid)
    {
        return rgbScalars;
    }

    auto assumedTempCoeffs = _spProfile->GetInterpolatedTableEntryForTemperature(currentTemp);
    auto assumedTempCoeffGenData = vvp::ccm::GetColorCoefficientsForTemperature((float)currentTemp);
    (void)assumedTempCoeffGenData; //unused

    return CalculateRGBScalarsToCorrectSceneWhiteLevel(assumedTempCoeffs._WbsRatio, targetTemp);
}

std::vector<float> WhiteBalanceController::CalculateRGBScalarsToCorrectSceneWhiteLevel(SwApi::NormalisedWbsRegion wbsRatio, uint16_t targetTemp)
{
    std::vector<float> rgbScalars = {0, 0, 0};

    // Step 2. Get the target temp coefficient data
    auto targetTempCoeffGenData = vvp::ccm::GetColorCoefficientsForTemperature((float)targetTemp);

    // Step 3. Scale current coeffs to match the target coefficients

    #define PRINT_VAR(x) std::cout << #x << ": " << x << "\n"

    // New ratio discovery
    // First, balance to true white

    // Red and blue added together when scaled must match green

    float requiredMatch = 0.33f;

    float channelDiff[3];

    float maxChanDevianceFromAverage = 1.0f;

    if (wbsRatio.red_strength > wbsRatio.green_strength &&
        wbsRatio.red_strength > wbsRatio.blue_strength)
    {
        channelDiff[0] = 1.0f;
        channelDiff[1] = requiredMatch / wbsRatio.green_strength;
        channelDiff[2] = requiredMatch / wbsRatio.blue_strength;
    }
    else if (wbsRatio.green_strength > wbsRatio.red_strength &&
             wbsRatio.green_strength > wbsRatio.blue_strength)
    {
        channelDiff[0] = requiredMatch / wbsRatio.red_strength;
        channelDiff[1] = 1.0f;
        channelDiff[2] = requiredMatch / wbsRatio.blue_strength;
    }
    else
    {
        channelDiff[0] = requiredMatch / wbsRatio.red_strength;
        channelDiff[1] = requiredMatch / wbsRatio.green_strength;
        channelDiff[2] = 1.0f;
    }

    maxChanDevianceFromAverage = requiredMatch / 0.33f;
    (void)maxChanDevianceFromAverage; // unused

    float ratioScalars[3];

    ratioScalars[0] = targetTempCoeffGenData.colour_chan_percent[0] / 0.33f;
    ratioScalars[1] = targetTempCoeffGenData.colour_chan_percent[1] / 0.33f;
    ratioScalars[2] = targetTempCoeffGenData.colour_chan_percent[2] / 0.33f;

    float newRatio[3];
    float total;

    float totalDiff = 1.0f;

    float threshold = 0.001f;

    for (int i = 0; i < 50; i++)
    {
        newRatio[0] = 0.33f * ratioScalars[0];
        newRatio[1] = 0.33f * ratioScalars[1];
        newRatio[2] = 0.33f * ratioScalars[2];

        total = newRatio[0] + newRatio[1] + newRatio[2];

        newRatio[0] /= total;
        newRatio[1] /= total;
        newRatio[2] /= total;

        totalDiff = 0.0f;
        totalDiff += pow((targetTempCoeffGenData.colour_chan_percent[0] - newRatio[0]), 2);
        totalDiff += pow((targetTempCoeffGenData.colour_chan_percent[1] - newRatio[1]), 2);
        totalDiff += pow((targetTempCoeffGenData.colour_chan_percent[2] - newRatio[2]), 2);
        totalDiff = sqrt(totalDiff);

        if (newRatio[0] > 0.0f) ratioScalars[0] *= targetTempCoeffGenData.colour_chan_percent[0] / newRatio[0];
        if (newRatio[1] > 0.0f) ratioScalars[1] *= targetTempCoeffGenData.colour_chan_percent[1] / newRatio[1];
        if (newRatio[2] > 0.0f) ratioScalars[2] *= targetTempCoeffGenData.colour_chan_percent[2] / newRatio[2];

        if (totalDiff < threshold)
        {
            break;
        }
    }

    rgbScalars[0] = channelDiff[0] * ratioScalars[0];
    rgbScalars[1] = channelDiff[1] * ratioScalars[1];
    rgbScalars[2] = channelDiff[2] * ratioScalars[2];

    return rgbScalars;
}

std::vector<uint32_t> WhiteBalanceController::CfaAlignedRGBScale(float red, float green, float blue, TCfaPhase cfaPhase)
{
    std::vector<uint32_t> wbcParams;

    switch (cfaPhase)
    {
        case TCfaPhase::RGGB:
        {
            wbcParams.push_back(baseWbcScale * red);
            wbcParams.push_back(baseWbcScale * green);
            wbcParams.push_back(wbcParams[1]);
            wbcParams.push_back(baseWbcScale * blue);
            break;
        }
        case TCfaPhase::GRBG:
        {
            wbcParams.push_back(baseWbcScale * green);
            wbcParams.push_back(baseWbcScale * red);
            wbcParams.push_back(baseWbcScale * blue);
            wbcParams.push_back(wbcParams[0]);
            break;
        }
        case TCfaPhase::GBRG:
        {
            wbcParams.push_back(baseWbcScale * green);
            wbcParams.push_back(baseWbcScale * blue);
            wbcParams.push_back(baseWbcScale * red);
            wbcParams.push_back(wbcParams[0]);
            break;
        }
        case TCfaPhase::BGGR:
        {
            wbcParams.push_back(baseWbcScale * blue);
            wbcParams.push_back(baseWbcScale * green);
            wbcParams.push_back(wbcParams[1]);
            wbcParams.push_back(baseWbcScale * red);
            break;
        }
    }

   return wbcParams;
}

void WhiteBalanceController::ApplyWhiteBalance()
{
    ApplyWhiteBalance(_currentTemp, _targetTemp, _cfaPhase);
}

void WhiteBalanceController::ApplyWhiteBalance(uint16_t currentTemp, uint16_t targetTemp, TCfaPhase cfaPhase)
{
    _currentTemp = currentTemp;
    _targetTemp = targetTemp;
    _cfaPhase = cfaPhase;

    if (!_spProfile->IsValid())
    {
        return;
    }

    if (_spProfile->IsColourTempRepresentableInTable(currentTemp) == EntryRepresentationStatus::Invalid)
    {
        return;
    }

    if(_awbMode == AWBMode::Disabled)
        return;

    uint32_t wbcParams[4];
    vvp::ccm::FloatCoefficients ccmMatrix = vvp::ccm::GenerateIdentityMatrix();

    // If we're correcting to the exact same temp (as in fully automatic setting) then we just need to load the params from
    // the table entry.
    // If not, then we need to recalculate the coefficients
    if (currentTemp == targetTemp)
    {
        auto currentTempData = _spProfile->GetInterpolatedTableEntryForTemperature(currentTemp);
        auto wbcCoeffs = CfaAlignedRGBScale(currentTempData._ChanScalars[0],
                                            currentTempData._ChanScalars[1],
                                            currentTempData._ChanScalars[2], cfaPhase);
        for (int i = 0; i < 4; i++)
        {
            wbcParams[i] = wbcCoeffs[i];
        }

        if (currentTempData._HasCcm)
        {
            ccmMatrix.A0 = currentTempData._CcmCoeffs[0][0];
            ccmMatrix.A1 = currentTempData._CcmCoeffs[0][1];
            ccmMatrix.A2 = currentTempData._CcmCoeffs[0][2];

            ccmMatrix.B0 = currentTempData._CcmCoeffs[1][0];
            ccmMatrix.B1 = currentTempData._CcmCoeffs[1][1];
            ccmMatrix.B2 = currentTempData._CcmCoeffs[1][2];

            ccmMatrix.C0 = currentTempData._CcmCoeffs[2][0];
            ccmMatrix.C1 = currentTempData._CcmCoeffs[2][1];
            ccmMatrix.C2 = currentTempData._CcmCoeffs[2][2];
        }
    }
    else
    {
        auto scalars = CalculateRGBScalarsToMatchTempToTarget(currentTemp, targetTemp);
        auto wbcCoeffs = CfaAlignedRGBScale(scalars[0], scalars[1], scalars[2], cfaPhase);
        for (int i = 0; i < 4; i++)
        {
            wbcParams[i] = wbcCoeffs[i];
        }
    }
    _spWhiteBalanceCorrection->SetAllColorScalers(wbcParams);

    _spColourCorrectionMatrix->ApplyAWBMatrix(ccmMatrix);
}

void WhiteBalanceController::ApplyVCMeshes()
{
    if (!_spProfile->HasVCMesh())
    {
        // No VC mesh, don't bother.
        return;
    }

    uint8_t meshXDim = _spProfile->GetHorizVCMeshPoints();
    uint8_t meshYDim = _spProfile->GetVertiVCMeshPoints();
    uint8_t numColPlanes = _spProfile->GetVCNumColorPlanes();
    std::vector<uint32_t> stepMesh = _spProfile->GetVCStepMesh();

    const auto [currentResX, currentResY] = _spVignetteCorrection->GetResolution();

    if (currentResX != _spProfile->GetVCStepMeshHorizRes() ||
        currentResY != _spProfile->GetVCStepMeshVertiRes())
    {
        std::cout << "Step mesh resolution does not match the current resolution, regenerating for current config\n";
        SwApi::VcMeshUtils::VcStepSampleCoordMesh sampleMesh;

        std::tie(stepMesh, sampleMesh) = SwApi::VcMeshUtils::GenerateStepMesh(currentResX, currentResY,
                                                                              _spProfile->GetPip(), meshXDim, meshYDim);
    }

    for(uint32_t i = 0; i < numColPlanes; i++)
    {
        _spVignetteCorrection->UploadMeshCpLut(i, _spProfile->GetVCCpMesh(i).data());
    }

    _spVignetteCorrection->UploadStepLut(stepMesh.data());

    _spVignetteCorrection->SetUpVCForMesh(meshXDim, meshYDim, UpdatePolicy::Sync());
}

void WhiteBalanceController::ResetWBSToDefault()
{
    _spWhiteBalanceStats->SetRoI(_roi, UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetCfaPhase(GetWbcCfaPhase(), UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetResultFormat(_currentFormat, UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetCfaX0Ranges(4.0f, 0.25f, UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetCfaX1Ranges(4.0f, 0.25f, UpdatePolicy::Async());
}

uint16_t WhiteBalanceController::GetCurrentSceneTemperature()
{
    return _sceneTemp;
}

uint16_t WhiteBalanceController::MesureCurrentSceneTemperature(bool applyUpdateToWbs)
{
    if (!_spProfile->IsValid())
    {
        return 0;
    }

    if (applyUpdateToWbs)
    {
        ResetWBSToDefault();
    }

    // Calculate the maximum number of ratios expected per zone
    auto roi = _spWhiteBalanceStats->GetRoI();
    uint32_t maxZoneRatios = ((roi.h_end - roi.h_start) * (roi.v_end - roi.v_start)) / (2 * 2 * 7 * 7);
    // Minimum number of ratios per zone allowed to count the statistics
    uint32_t minZoneRatios = std::max(10u, maxZoneRatios / 10);

    auto results = _spWhiteBalanceStats->ReadResultTable();

    SwApi::NormalisedWbsRegion overallNormalisedResult = {0};
    uint32_t zoneCount = 0;

    for (int y = 0; y < 7; y++)
    {
        for (int x = 0; x < 7; x++)
        {
            // Check for zones that did not discard too many ratios
            if (results.raw[y][x].num_pixels_accumulated > minZoneRatios)
            {
                zoneCount += 1;

                overallNormalisedResult.red_strength += results.normalised[y][x].red_strength;
                overallNormalisedResult.green_strength += results.normalised[y][x].green_strength;
                overallNormalisedResult.blue_strength += results.normalised[y][x].blue_strength;
            }
        }
    }

    if (zoneCount == 0)
    {
        // No useful zones found in the statistics
        if (_print_info)
        {
            std::cout << "[White balance statistics]: No valid statistics found. Defaulting to 5700K.\n";
        }
        _print_info = false;
        return 5700;
    }
    else
    {
        _print_info = true;
    }

    overallNormalisedResult.red_strength /= zoneCount;
    overallNormalisedResult.green_strength /= zoneCount;
    overallNormalisedResult.blue_strength /= zoneCount;

    float totalStrengths = overallNormalisedResult.red_strength + overallNormalisedResult.green_strength + overallNormalisedResult.blue_strength;

    overallNormalisedResult.red_strength /= totalStrengths;
    overallNormalisedResult.green_strength /= totalStrengths;
    overallNormalisedResult.blue_strength /= totalStrengths;

    // Next step, go through the colour recordings and work out which has the minimum distance to the overallNormalisedResult

    int closest = 0;
    float minimumClosestDist = 100;
    auto listOfTemps = _spProfile->GetListOfTemps();
    for (int i = 0; i < (int)listOfTemps.size(); i++)
    {

        auto entry = _spProfile->GetInterpolatedTableEntryForTemperature(listOfTemps[i]);
        float redDist = entry._WbsRatio.red_strength - overallNormalisedResult.red_strength;
        float greenDist = entry._WbsRatio.green_strength - overallNormalisedResult.green_strength;
        float blueDist = entry._WbsRatio.blue_strength - overallNormalisedResult.blue_strength;
        float distance = (redDist * redDist) + (greenDist * greenDist) + (blueDist * blueDist);

        distance = sqrt(distance);

        if (distance < minimumClosestDist)
        {
            minimumClosestDist = distance;
            closest = i;
        }
    }

    // Now we have the closest one, check the temps either side of it to see which is closest out of the two.
    // This will give us the interval where the best value lies somewhere.
    // We then scan through that interval to find the temperature of minimum delta, and hope for the best.


    int minIndex = (closest > 0) ? (closest - 1) : (0);
    int maxIndex = (closest < ((int)listOfTemps.size() - 1)) ? (closest + 1) : (closest);

    uint16_t minTemp = listOfTemps[minIndex] / 100;
    minTemp *= 100; // We don't need any values under the 100s
    uint16_t maxTemp = listOfTemps[maxIndex] / 100;
    maxTemp *= 100;

    uint16_t closestTemp = 0;

    minimumClosestDist = 100; // Reset the minimum distance store

    for (uint16_t tempItr = minTemp; tempItr < maxTemp; tempItr += 100)
    {
        if (_spProfile->IsColourTempRepresentableInTable(tempItr) == EntryRepresentationStatus::Invalid)
        {
            continue;
        }

        auto entry = _spProfile->GetInterpolatedTableEntryForTemperature(tempItr);
        float redDist = entry._WbsRatio.red_strength - overallNormalisedResult.red_strength;
        float greenDist = entry._WbsRatio.green_strength - overallNormalisedResult.green_strength;
        float blueDist = entry._WbsRatio.blue_strength - overallNormalisedResult.blue_strength;
        float distance = (redDist * redDist) + (greenDist * greenDist) + (blueDist * blueDist);

        distance = sqrt(distance);

        if (distance < minimumClosestDist)
        {
            minimumClosestDist = distance;
            closestTemp = tempItr;
        }
    }

    return closestTemp;
}



SwApi::WbsResults WhiteBalanceController::ReadWBSAndGenerateCoeffsForTemp(uint16_t temp, TCfaPhase cfaPhase, float* wbcScalars, uint32_t* wbcCoeffs, uint32_t* bestX, uint32_t* bestY)
{
    // New logic - This needs to read the baseline colour correction, talk to the colour calculator to
    // work out what the colour ratios should be, and then correct the ratios to match it to that point.
    if (_spWbsSwitchRouter)
    {
        for (int i = 0; i < 5; i++)
        {
            _spWbsSwitchRouter->SetWBSInput(SwitchRouterInput::PreWBC);
            usleep(30000);
            _spWhiteBalanceStats->ReadResultTable(); // Do a handshake
        }
    }
    _spWhiteBalanceStats->SetCfaPhase(GetWbcCfaPhase(), UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetResultFormat(_currentFormat, UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetCfaX0Ranges(4.0f, 0.25f, UpdatePolicy::DeferCommit());
    _spWhiteBalanceStats->SetCfaX1Ranges(4.0f, 0.25f, UpdatePolicy::Sync());
    _spWhiteBalanceStats->ReadResultTable(); // Do a handshake
    SwApi::WbsResults results = _spWhiteBalanceStats->ReadResultTable();
    // TODO: Accumulate cells together to get the best one, but for now, just use the
    // central cell.

    *bestX = 3;
    *bestY = 3;

    // Baseline correction recording:
    // - First, make it "white" by balancing the channels so that they're all equal
    // - Second, find the "current temp" and work out what the percentages *should* be
    // - Third, apply that modification to the channelScalars


    SwApi::NormalisedWbsRegion& bestRegion = results.normalised[*bestY][*bestX];

    vvp::ccm::ColorCoefficients coeffs = vvp::ccm::GetColorCoefficientsForTemperature((float)temp);

    auto channelScalars = CalculateRGBScalarsToCorrectSceneWhiteLevel(bestRegion, temp);

    // With the switch router in place, we can do a post-check pass
    if (_spWbsSwitchRouter)
    {
        for (int i = 0 ; i < 6; i++)
        {
            auto wbcCoeffVec = CfaAlignedRGBScale(channelScalars[0], channelScalars[1], channelScalars[2], cfaPhase);
            for (int i = 0; i < 4; i++)
            {
                wbcCoeffs[i] = wbcCoeffVec[i];
            }
            _spWhiteBalanceCorrection->SetAllColorScalers(wbcCoeffs, UpdatePolicy::Sync());

            usleep(30000);

            for (int i = 0; i < 5; i++)
            {
                _spWbsSwitchRouter->SetWBSInput(SwitchRouterInput::PostWBC);
                usleep(30000);
                _spWhiteBalanceStats->ReadResultTable(); // Do a handshake
            }
            usleep(30000);
            _spWhiteBalanceStats->ReadResultTable(); // Do a handshake
            _spWhiteBalanceStats->ReadResultTable(); // Do a handshake
            SwApi::WbsResults newResults = _spWhiteBalanceStats->ReadResultTable();

            // Green is almost always the dominant channel, but what if it isn't?
            float newRedScalar = channelScalars[0] * (coeffs.colour_chan_percent[0] / newResults.normalised[*bestY][*bestX].red_strength);
            float newBlueScalar = channelScalars[2] * (coeffs.colour_chan_percent[2] / newResults.normalised[*bestY][*bestX].blue_strength);
            channelScalars[0] += (newRedScalar - channelScalars[0]) * 0.85f;
            channelScalars[2] += (newBlueScalar - channelScalars[2]) * 0.85f;
        }
    }

    if (wbcScalars)
    {
        for (int i = 0; i < 3; i++)
        {
            wbcScalars[i] = channelScalars[i];
        }
    }

    // Now create the WBC parameters
    if (wbcCoeffs)
    {
        auto wbcCoeffVec = CfaAlignedRGBScale(channelScalars[0], channelScalars[1], channelScalars[2], cfaPhase);
        for (int i = 0; i < 4; i++)
        {
            wbcCoeffs[i] = wbcCoeffVec[i];
        }
    }

    return results;
}

void WhiteBalanceController::ReadWBSAndUpdateProfile(TCfaPhase cfaPhase, uint16_t temp, uint32_t *params)
{
    float chanScalars[3];
    uint32_t wbcParams[4];
    uint32_t bestX;
    uint32_t bestY;
    auto results = ReadWBSAndGenerateCoeffsForTemp(temp, cfaPhase, chanScalars, wbcParams, &bestX, &bestY);
    SwApi::NormalisedWbsRegion& bestRegion = results.normalised[bestY][bestX];

    if (params)
    {
        std::memcpy(params, wbcParams, sizeof(uint32_t) * 4);
    }

    auto tempTableEntry = _spProfile->FindOrCreateEntryByColourTemp(temp);

    tempTableEntry->_HasRaw = true;
    tempTableEntry->_Raw = results.raw[bestY][bestX];
    tempTableEntry->_RawFormat = _currentFormat;
    tempTableEntry->_RawDecBits = _spWhiteBalanceStats->GetPrecisionBits();
    tempTableEntry->_WbsRatio = bestRegion;

    std::memcpy(tempTableEntry->_ChanScalars, chanScalars, sizeof(float) * 3);
}

