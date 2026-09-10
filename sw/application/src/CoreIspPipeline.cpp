/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CoreIspPipeline.h"
#include "VvpIspDemo.h"
#include "Logging.h"

#include "BlsUtils.h"

using namespace SwApi;
using namespace VideoPipeline;

CoreIspPipeline::CoreIspPipeline(uint32_t pipeline_index, const std::shared_ptr<Hapi::IHapi>& spHapi, const uint32_t coreIspBaseUID)
    : _pipeline_index(pipeline_index)
    , _spHapi(spHapi)
    , _coreIspBaseUID(coreIspBaseUID)
    , _activeCamera{nullptr}
{
}

bool CoreIspPipeline::InitCoreIspPipeline()
{
    bool rc = true;

    bool blsFound = InitBls();
    if (!blsFound)
    {
        ERR << "Failed to initialise Bls " << std::to_string(_pipeline_index) << ".\n";
    }

    bool clipperFound = InitClipper();
    if (!clipperFound)
    {
        ERR << "Failed to initialise Clipper " << std::to_string(_pipeline_index) << ".\n";
    }

    bool dpcFound = InitDpc();
    if (!dpcFound)
    {
        ERR << "Failed to initialise Dpc " << std::to_string(_pipeline_index) << ".\n";
    }

    bool anrFound = InitAnr();
    if (!anrFound)
    {
      ERR << "Failed to initialise Anr " << std::to_string(_pipeline_index) << ".\n";
    }

    bool blcFound = InitBlc();
    if (!blcFound)
    {
        ERR << "Failed to initialise Blc " << std::to_string(_pipeline_index) << ".\n";
    }

    bool vcFound = InitVc();
    if (!vcFound)
    {
        ERR << "Failed to initialise Vc " << std::to_string(_pipeline_index) << ".\n";
    }

    bool routerSwitchFound = InitRouterSwitch();
    if (!routerSwitchFound)
    {
        INFO << "Router switch not detected " << std::to_string(_pipeline_index) << ".\n";
    }

    bool wbsFound = InitWbs();
    if (!wbsFound)
    {
        ERR << "Failed to initialise Wbs " << std::to_string(_pipeline_index) << ".\n";
    }

    bool wbcFound = InitWbc();
    if (!wbcFound)
    {
        ERR << "Failed to initialise Wbc " << std::to_string(_pipeline_index) << ".\n";
    }

    bool demosaicFound = InitDemosaic();
    if (!demosaicFound)
    {
        ERR << "Failed to initialise Demosaic " << std::to_string(_pipeline_index) << ".\n";
    }

    bool hsFound = InitHs();
    if (!hsFound)
    {
        ERR << "Failed to initialise Histogram Statistics " << std::to_string(_pipeline_index) << ".\n";
    }
    
    bool ccmFound = InitCcm();
    if (!ccmFound)
    {
        ERR << "Failed to initialise Ccm " << std::to_string(_pipeline_index) << ".\n";
    }

    bool roiFound = InitROI();
    if (!roiFound)
    {
        ERR << "Failed to initialise ROI " << std::to_string(_pipeline_index) << ".\n";
    }

    bool scalersFound = InitScalers();
    if (!scalersFound)
    {
        INFO << "no scalers found " << std::to_string(_pipeline_index) << ".\n";
    }

    bool warpFound = InitWarp();
    if(!warpFound)
    {
        WARN << "Failed to initialise Warp " << std::to_string(_pipeline_index) << ". Falling back to initialise Vfb\n";

        bool vfbFound = InitVfb();

        if (!vfbFound)
        {
            WARN << "Failed to initialise Vfb " << std::to_string(_pipeline_index) << ".\n";
        }
    }

    bool profileInitialised = InitSensorProfiling();
    if (!profileInitialised)
    {
        ERR << "Failed to initialise sensor profile subsystem " << std::to_string(_pipeline_index) << ".\n";
    }

    auto coreIspPipelineInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_coreIspInputCore->_inputCores.size() == 1 && _coreIspInputCore->_outputCores.size() == 1);

            TRACE << "[CoreIspPipeline IN " << std::to_string(_pipeline_index) << "]:\t" << LOG_IN(inputDetails) << '\n' << std::flush;

            _coreIspInputCore->_currentInputVideoDetails = inputDetails;
            _coreIspInputCore->_currentOutputVideoDetails = inputDetails;
            _coreIspInputCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _coreIspInputCore = PipelineCore::Create(nullptr, coreIspPipelineInputHandler);
    _coreIspInputCore->_currentInputVideoDetails = VideoStandard();
    _coreIspInputCore->_currentOutputVideoDetails = VideoStandard();

    _coreIspOutputCore = _postFbCore;

    return rc;
}

void CoreIspPipeline::SetInputCfaPhase(TCfaPhase inputCfaPhase)
{
    _inputCfaPhase = inputCfaPhase;
}

void CoreIspPipeline::ResetClipper(uint32_t imageWidth, uint32_t imageHeight)
{
    if(_spClipper)
    {
        if (_spClipper->GetMode() == ClipperMode::OffsetClipping)
        {
            _spClipper->SetClipTo(
                OffsetClipSetting{.top    = 0,
                                .left   = 0,
                                .bottom = 0,
                                .right  = 0}
            );
        }
        else if (_spClipper->GetMode() == ClipperMode::RectangleClipping)
        {
            _spClipper->SetClipTo(
                RectangleClipSetting{.topOffset  = 0,
                                    .leftOffset = 0,
                                    .clipWidth  = (uint16_t)imageWidth,
                                    .clipHeight = (uint16_t)imageHeight}
            );
        }
    }
}

void CoreIspPipeline::UpdateClipper(uint32_t& imageWidth, uint32_t& imageHeight)
{
    // If we're not resetting the clipper, then we need to pass whatever is in the clipper to warp
    // Calculate output resolution for the clipper.
    switch (_spClipper->GetMode())
    {
        default:
        case ClipperMode::OffsetClipping:
        {
            OffsetClipSetting setting = _spClipper->GetOffsetClipSetting();
            imageWidth = imageWidth - setting.left - setting.right;
            imageHeight = imageHeight - setting.bottom - setting.top;
            break;
        }
        case ClipperMode::RectangleClipping:
        {
            RectangleClipSetting setting = _spClipper->GetRectangleClipSetting();
            imageWidth = setting.clipWidth;
            imageHeight = setting.clipHeight;
            break;
        }
    }
}


void CoreIspPipeline::SetActiveCamera(ICameraPtr pCamera)
{
    _activeCamera = std::move(pCamera);

    if(_activeCamera)
    {
        const auto analogueGain = _activeCamera->GetAnalogueGain();

        if(_spWBController)
        {
            _spWBController->SetGain(analogueGain);
        }
    }

    if(_spAutoExposure)
        _spAutoExposure->SetCamera(_activeCamera);
}

void CoreIspPipeline::InsertAdditionalCore(std::shared_ptr<VideoPipeline::PipelineCore> additionalCore)
{
    _coreIspStack.push(additionalCore);
}


bool CoreIspPipeline::InitBls()
{
    const uint32_t BLS_UID = _coreIspBaseUID;
    auto spVvpBlsInst = _spHapi->CreateByUniqueID<Hapi::VvpBls>(BLS_UID);
    if (!spVvpBlsInst) return false;

    _spBls = Bls::Create(spVvpBlsInst);
    if (!_spBls) return false;

    _spBls->SetCfaPhase(_inputCfaPhase);
    _spBls->SetOpticalBlackRegion(0,0,100,100);

    // Pipeline core

    auto blsCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_blsCore->_inputCores.size() == 1 && _blsCore->_outputCores.size() == 1);

            TRACE << "[BLS " << std::to_string(_pipeline_index) << "]: " << LOG_IN(inputDetails) << '\n' << std::flush;

            _blsCore->_currentInputVideoDetails = inputDetails;
            _spBls->SetResolution(inputDetails._width, inputDetails._height);

            _spBls->SetCfaPhase(_inputCfaPhase);

            // Sanity check that the optical black region still makes sense for the current input.
            // e.g. user set the OB region for 4K UHD, but then switched to a 1080p source.
            SwApi::BlackRegion opticalBlackRegion = GetBls()->GetCombinedOpticalBlackRegion();

            bool isObRegionValid = (opticalBlackRegion.v_end <= inputDetails._height) &&
                                   (opticalBlackRegion.h_end <= inputDetails._width);

            if (!isObRegionValid)
            {
                SwApi::BlackRegion fixedObRegion = opticalBlackRegion;

                fixedObRegion.h_end = inputDetails._width;
                fixedObRegion.v_end = inputDetails._height;

                WARN << "[BLS " << std::to_string(_pipeline_index) << "]: The currently set optical black region is outside the bounds of "
                        "input video.\n"
                     << "\t\t" << LOG_V(inputDetails) << "\n"
                     << "\t\t" << LOG_V(opticalBlackRegion) << "\n"
                     << "\t\tClipping black region to within input range:\n"
                     << "\t\t" << LOG_V(fixedObRegion) << "\n";

                GetBls()->SetCombinedOpticalBlackRegion(fixedObRegion);
            }

            _blsCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _blsCore = PipelineCore::Create(_spBls, blsCorePipelineHandler);

    _coreIspStack.push(_blsCore);

    return true;
}

bool CoreIspPipeline::InitClipper()
{
    auto clipperSwitchUID = _coreIspBaseUID;

    auto spClipperSwitchInst = _spHapi->CreateByUniqueID<Hapi::VvpSwitch>(clipperSwitchUID);
    if (!spClipperSwitchInst)
    {
        ERR << "Failed to create Vfw Clipper HAPI VvpSwitch with unique ID "
            << clipperSwitchUID << "\n";
        return false;
    }

    _spVfwClipperSwitch = SwApi::ISwitch::Create(spClipperSwitchInst);
    if (!_spVfwClipperSwitch)
    {
        ERR << "Failed to create Vfw Clipper ISwitch with unique ID "
            << clipperSwitchUID << "\n";
        return false;
    }
    _spVfwClipperSwitch->ConnectOnly(0, 0);

    const uint32_t CLIPPER_UID = _coreIspBaseUID;
    auto spVvpClipperInst = _spHapi->CreateByUniqueID<Hapi::VvpClipper>(CLIPPER_UID);
    if (!spVvpClipperInst)
    {
        return false;
    }

    _spClipper = IClipper::Create(spVvpClipperInst, 1920, 1080);
    if (!_spClipper)
    {
        return false;
    }

    constexpr auto w=1920;
    constexpr auto h=1080;

    _spClipper->SetInputDimensions(w,h);

    if (_spClipper->GetMode() == ClipperMode::OffsetClipping)
    {
        _spClipper->SetClipTo(
            OffsetClipSetting{.top = 0, .left=0, .bottom=0, .right=0}
        );

    }
    else if (_spClipper->GetMode() == ClipperMode::RectangleClipping)
    {
        INFO << "Clipper has rect clipping on\n";
        _spClipper->SetClipTo(
            RectangleClipSetting{.topOffset = 0, .leftOffset = 0, .clipWidth=w, .clipHeight=h}
        );
    }
    else
    {
        ERR << "Clipper has INVALID clipping on\n";
        return false;
    }

    TRACE << "Clipper initialised. Clipper mode is " << _spClipper->GetMode() << ".\n" << std::flush;

    // Pipeline core

    auto clipperCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_clipperCore->_inputCores.size() == 1 &&
                   _clipperCore->_outputCores.size() == 1);

            _spClipper->SetInputDimensions(inputDetails._width, inputDetails._height);

            ImageConfig clippedImageDetails = inputDetails;

            // Calculate output resolution for the clipper.
            switch (_spClipper->GetMode())
            {
                case ClipperMode::OffsetClipping:
                {
                    OffsetClipSetting setting = _spClipper->GetOffsetClipSetting();
                    TRACE << "[Clipper " << std::to_string(_pipeline_index) << "]: Setting is: " << LOG_V(setting) << "\n" << std::flush;
                    clippedImageDetails._height
                        = inputDetails._height - setting.bottom - setting.top;
                    clippedImageDetails._width
                        = inputDetails._width - setting.left - setting.right;
                    break;
                }
                case ClipperMode::RectangleClipping:
                {
                    RectangleClipSetting setting = _spClipper->GetRectangleClipSetting();
                    TRACE << "[Clipper " << std::to_string(_pipeline_index) << "]: Rectangle Setting is: " << LOG_V(setting) << "\n" << std::flush;
                    clippedImageDetails._height = setting.clipWidth;
                    clippedImageDetails._width = setting.clipHeight;
                    break;
                }
                case ClipperMode::InvalidClipping:
                {
                    ASSERT(false, "VVP Clipper had invalid clipping set. Why?");
                }
            }

            TRACE << "[Clipper " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, clippedImageDetails) << '\n';

            _clipperCore->_currentInputVideoDetails = inputDetails;
            _clipperCore->_currentOutputVideoDetails = clippedImageDetails;
            _clipperCore->_outputCores[0]->ApplyInputVideoInfo(clippedImageDetails);

            return clippedImageDetails;
        };
    _clipperCore = PipelineCore::Create(_spClipper, clipperCorePipelineHandler);

    _coreIspStack.push(_clipperCore);

    return true;
}

bool CoreIspPipeline::InitDpc()
{
    const uint32_t DPC_UID = _coreIspBaseUID;
    auto spVvpDpcInst = _spHapi->CreateByUniqueID<Hapi::VvpDpc>(DPC_UID);
    if (!spVvpDpcInst)
    {
        ERR << "HAPI could !get access to the DPC. Maybe check the capability structure?\n";
        return false;
    }

    _spDpc = IDpc::Create(spVvpDpcInst, 1920, 1080);
    if (!_spDpc)
    {
        ERR << "Error while setting up the DPC (the DPC instance was found, but the driver failed to start.)\n";
        return false;
    }

    _spDpc->SetCfaPhase(_inputCfaPhase) || WARN << "DPC Failed to set CFA phase\n";
    _spDpc->SetSensitivityLevel(DpcSensitivityLevel::VeryWeak) || WARN << "DPC Failed to set sensitivity level.\n";

    // Pipeline core

    auto dpcCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_dpcCore->_inputCores.size() == 1 && _dpcCore->_outputCores.size() == 1);

            TRACE << "[DPC " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

            _spDpc->SetOutputWidth(inputDetails._width);
            _spDpc->SetOutputHeight(inputDetails._height);

            _spDpc->SetCfaPhase(_inputCfaPhase);

            _dpcCore->_currentInputVideoDetails = inputDetails;
            _dpcCore->_currentOutputVideoDetails = inputDetails;
            _dpcCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _dpcCore = PipelineCore::Create(_spDpc, dpcCorePipelineHandler);

    _coreIspStack.push(_dpcCore);

    return true;
}

bool CoreIspPipeline::InitAnr()
{
    const uint32_t ANR_UID = _coreIspBaseUID;
    auto spVvpAnrInst = _spHapi->CreateByUniqueID<Hapi::VvpAnr>(ANR_UID);
    if (!spVvpAnrInst)
    {
        return false;
    }

    _spAnr = SwApi::Anr::Create(spVvpAnrInst, 1920u, 1080u);
    if (!_spAnr)
    {
        return false;
    }

    _spAnr->SetBypass(false, UpdatePolicy::DeferCommit());
    _spAnr->ApplyUnityLuts();

    // Pipeline core

    auto anrCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_anrCore->_inputCores.size() == 1 && _anrCore->_outputCores.size() == 1);

            TRACE << "[ANR " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

            //_spAnr->WriteRangeLut();
            _spAnr->SetResolution(inputDetails._width, inputDetails._height);

            //_spAnr->WriteSpaceLut();
            //_spAnr->SetAlphaBlending(_spAnr->GetAlphaBlending());
            //_spAnr->UpdateConfig();

            _anrCore->_currentInputVideoDetails = inputDetails;
            _anrCore->_currentOutputVideoDetails = inputDetails;
            _anrCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _anrCore = PipelineCore::Create(_spAnr, anrCorePipelineHandler);

    _coreIspStack.push(_anrCore);

    return true;
}

bool CoreIspPipeline::InitBlc()
{
    const uint32_t BLC_UID = _coreIspBaseUID;
    auto spVvpBlcInst = _spHapi->CreateByUniqueID<Hapi::VvpBlc>(BLC_UID);
    if (!spVvpBlcInst)
    {
        ERR << "[BLC " << std::to_string(_pipeline_index) << "]:Failed to grab a hold of the VVP BLC IP Core. Is it present?\n";
        return false;
    }

    _spBlc = Blc::Create(spVvpBlcInst, 1920, 1080);
    if (!_spBlc)
    {
        return false;
    }

    _spBlc->SetCfaPhase(_inputCfaPhase, UpdatePolicy::DeferCommit()) || ERR << "[BLC " << std::to_string(_pipeline_index) << "]:Failed to set BLC CfaPhase to RGGB.\n";
    _spBlc->SetClipZero(true, UpdatePolicy::DeferCommit());
    _spBlc->SetBypass(true, UpdatePolicy::Async()) || ERR << "[BLC " << std::to_string(_pipeline_index) << "]:Failed to set bypass to true.\n";

    // Pipeline core

    auto blcCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_blcCore->_inputCores.size() == 1 && _blcCore->_outputCores.size() == 1);

            TRACE << "[BLC " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

            _spBlc->SetResolution(inputDetails._width, inputDetails._height);
            _spBlc->SetCfaPhase(_inputCfaPhase, UpdatePolicy::Async());

            _blcCore->_currentInputVideoDetails = inputDetails;
            _blcCore->_currentOutputVideoDetails = inputDetails;
            _blcCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _blcCore = PipelineCore::Create(_spBlc, blcCorePipelineHandler);

    _coreIspStack.push(_blcCore);

    return true;
}

bool CoreIspPipeline::InitVc()
{
    const uint32_t VC_UID = _coreIspBaseUID;
    auto spVvpVcInst = _spHapi->CreateByUniqueID<Hapi::VvpVc>(VC_UID);
    if (!spVvpVcInst)
    {
        ERR << "[VC " << std::to_string(_pipeline_index) << "]:Failed to grab a hold of the VVP VC IP Core. Is it present?\n";
        return false;
    }

    _spVc = Vc::Create(spVvpVcInst, 1920u, 1080u);
    if (!_spVc)
    {
        return false;
    }

    _spVc->SetBypass(true, UpdatePolicy::DeferCommit());
    _spVc->SetCfaPhase(_inputCfaPhase, UpdatePolicy::DeferCommit());

    // Limited to 16:9 ratios for now
    static constexpr uint16_t meshX = 35u;
    static constexpr uint16_t meshY = 21u;

    _spVc->SetUpVCForMesh(meshX, meshY, UpdatePolicy::Async());

    // Apply a unity mesh
    auto currentMesh = SwApi::VcMeshUtils::quantizeMeshToFixedPoint8i11f(SwApi::VcMeshUtils::GenerateUnityFMesh(meshX, meshY));

    // Upload the current mesh.
    const uint32_t numCp = _spVc->GetPerColorGainEnable() ? 4 : 1;

    for(uint32_t i = 0; i < numCp; i++)
    {
        if (!_spVc->UploadMeshCpLut(i, currentMesh.data()))
        {
            std::cout << "[VC " << std::to_string(_pipeline_index) << " Init] Failed to upload mesh index " << i << "\n";
        }
    }


    // Pipeline core
    auto vcCorePipelineHandler = [this](const ImageConfig& inputDetails) -> ImageConfig {
        ASSERT(_vcCore->_inputCores.size() == 1 && _vcCore->_outputCores.size() == 1);

        TRACE << "[VC " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

        const auto [width, height] = _spVc->GetResolution();

        if (width != inputDetails._width || height != inputDetails._height)
        {
            bool needsNewStepMesh = !_spVc->CheckIfResolutionFitsExistingStepMeshConfig(inputDetails._width, inputDetails._height);

            _spVc->SetResolution(inputDetails._width, inputDetails._height);
            _spVc->SetUpVCForMesh(_spVc->GetHorizontalNumBlocks() + 1, _spVc->GetVerticalNumBlocks() + 1, UpdatePolicy::DeferCommit());

            if (needsNewStepMesh)
            {
                TRACE << "[VC " << std::to_string(_pipeline_index) << "]: Regenerating step mesh..." << "\n" << std::flush;
                SwApi::VcMeshUtils::VcStepMesh stepMesh;
                SwApi::VcMeshUtils::VcStepSampleCoordMesh sampleMesh;
                uint8_t pip = _spVc->GetPip();

                std::tie(stepMesh, sampleMesh) = SwApi::VcMeshUtils::GenerateStepMesh(inputDetails._width, inputDetails._height, pip,
                                                                                        _spVc->GetHorizontalNumBlocks() + 1, _spVc->GetVerticalNumBlocks() + 1);
                _spVc->UploadStepLut(stepMesh.data());
            }
        }

        _spVc->SetCfaPhase(_inputCfaPhase, UpdatePolicy::Async());

        _vcCore->_currentInputVideoDetails = inputDetails;
        _vcCore->_currentOutputVideoDetails = inputDetails;
        _vcCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);
        return inputDetails;
    };

    _vcCore = PipelineCore::Create(_spVc, vcCorePipelineHandler);

    _coreIspStack.push(_vcCore);

    return true;
}

bool CoreIspPipeline::InitRouterSwitch()
{
    const uint32_t ROUTER_SW_UID = _coreIspBaseUID;
    auto spRouterSwitchHapi = _spHapi->CreateByUniqueID<Hapi::VvpSwitch>(ROUTER_SW_UID);
    if (!spRouterSwitchHapi)
    {
        return false;
    }

    auto spRouterSwitchSwApi = ISwitch::Create(spRouterSwitchHapi);
    if (!spRouterSwitchSwApi)
    {
        return false;
    }

    _spWbsSwitchRouter = std::make_shared<SwitchRouter>(spRouterSwitchSwApi);

    _spWbsSwitchRouter->SetWBSInput(SwitchRouterInput::PreWBC);

    if(_spWbsSwitchRouter == nullptr)
    {
        return false;
    }
    return true;
}

bool CoreIspPipeline::InitWbs()
{
    const uint32_t WBS_UID = _coreIspBaseUID;
    auto spVvpWbsInst = _spHapi->CreateByUniqueID<Hapi::VvpWbs>(WBS_UID);
    if (!spVvpWbsInst) return false;

    _spWbs = Wbs::Create(spVvpWbsInst);
    if (!_spWbs) return false;

    _spWbs->SetResolution(1920u, 1080u);
    WbsRoI roi = {0,0, 1920, 1080};
    _spWbs->SetRoI(roi, UpdatePolicy::DeferCommit());

    _spWbs->SetCfaPhase(_inputCfaPhase, UpdatePolicy::DeferCommit());
    _spWbs->SetResultFormat(SwApi::WbsResultFormat::RG_GB, UpdatePolicy::DeferCommit());

    _spWbs->SetStatsFreeze(false);
    _spWbs->SetBypass(false, UpdatePolicy::DeferCommit());
    _spWbs->SetCfaX0Ranges(4.0f, 0.25f, UpdatePolicy::DeferCommit());
    _spWbs->SetCfaX1Ranges(4.0f, 0.25f, UpdatePolicy::Async());
    // Pipeline core

    auto wbsCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_wbsCore->_inputCores.size() == 1 && _wbsCore->_outputCores.size() == 1);

            TRACE << "[WBS]: " << LOG_IN(inputDetails) << '\n';

            _wbsCore->_currentInputVideoDetails = inputDetails;

            _spWbs->SetResolution(inputDetails._width, inputDetails._height);
            _spWbs->SetCfaPhase(_inputCfaPhase, UpdatePolicy::Async());

            _wbsCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _wbsCore = PipelineCore::Create(_spWbs, wbsCorePipelineHandler);

    _coreIspStack.push(_wbsCore);

    return true;
}

bool CoreIspPipeline::InitWbc()
{
    const uint32_t WBC_UID = _coreIspBaseUID;
    auto spVvpWbcInst = _spHapi->CreateByUniqueID<Hapi::VvpWbc>(WBC_UID);
    if (!spVvpWbcInst)
    {
        ERR << "Failed to grab a hold of the VVP WBC IP Core. Is it present?\n";
        return false;
    }

    _spWbc = Wbc::Create(spVvpWbcInst, 1920, 1080);
    if (!_spWbc)
    {
        return false;
    }

    _spWbc->SetBypass(true, UpdatePolicy::DeferCommit());
    _spWbc->SetCfaPhase(_inputCfaPhase, UpdatePolicy::DeferCommit());

    static constexpr uint32_t scalers[4] = {2048u, 2048u, 2048u, 2048u};
    _spWbc->SetAllColorScalers(scalers, UpdatePolicy::Async());

    // Pipeline core

    auto wbcCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_wbcCore->_inputCores.size() == 1 && _wbcCore->_outputCores.size() == 1);

            TRACE << "[WBC " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, inputDetails) << '\n';

            _spWbc->SetResolution(inputDetails._width, inputDetails._height);

            if(false == _spWbc->SetCfaPhase(_inputCfaPhase, UpdatePolicy::Async()))
                TRACE << "[WBC " << std::to_string(_pipeline_index) << "]: Error setting CFA phase\n";

            _wbcCore->_currentInputVideoDetails = inputDetails;
            _wbcCore->_currentOutputVideoDetails = inputDetails;

            _wbcCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _wbcCore = PipelineCore::Create(_spWbc, wbcCorePipelineHandler);

    _coreIspStack.push(_wbcCore);

    return true;
}

bool CoreIspPipeline::InitDemosaic()
{
    const uint32_t DEMOSAIC_UID = _coreIspBaseUID;
    auto spVvpDemosaicInst = _spHapi->CreateByUniqueID<Hapi::VvpDemosaic>(DEMOSAIC_UID);
    if (!spVvpDemosaicInst)
    {
        return false;
    }

    _spDemosaic = IDemosaic::Create(spVvpDemosaicInst, 1920, 1080);
    if (!_spDemosaic)
    {
        return false;
    }

    _spDemosaic->SetCfaPhase(_inputCfaPhase);

    // Pipeline core

    auto demosaicCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_demosaicCore->_inputCores.size() == 1 &&
                   _demosaicCore->_outputCores.size() == 1);

            TRACE << "[DMS]:\t" << LOG_IO(inputDetails, inputDetails) << '\n';

            _spDemosaic->SetOutputWidth(inputDetails._width);
            _spDemosaic->SetOutputHeight(inputDetails._height);

            _spDemosaic->SetCfaPhase(_inputCfaPhase);

            _demosaicCore->_currentInputVideoDetails = inputDetails;
            _demosaicCore->_currentOutputVideoDetails = inputDetails;
            _demosaicCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);
            return inputDetails;
        };
    _demosaicCore = PipelineCore::Create(_spDemosaic, demosaicCorePipelineHandler);

    _coreIspStack.push(_demosaicCore);

    return true;
}

bool CoreIspPipeline::InitHs()
{
    const uint32_t HS_UID = _coreIspBaseUID;
    auto spVvpHsInst = _spHapi->CreateByUniqueID<Hapi::VvpHs>(HS_UID);
    if (!spVvpHsInst)
    {
        std::cerr << "[HS " << std::to_string(_pipeline_index) << "]: Failed to get Hapi" << std::endl;
        return false;
    }

    _spHs = HistogramStats::Create(spVvpHsInst);
    if (!_spHs)
    {
        std::cerr << "[HS " << std::to_string(_pipeline_index) << "]: Failed to create interface" << std::endl;
        return false;
    }

    _spHs->SetResolution(1920u, 1080u);

    SwApi::Hs::RegionOfInterest region = {0, 0, 1, 1};

    _spHs->SetRegionOfInterest(region);

    // Pipeline core

    auto histogramStatsCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_hsCore->_inputCores.size() == 1 &&
                _hsCore->_outputCores.size() == 1);

            TRACE << "[HS " << std::to_string(_pipeline_index) << "]: " << LOG_V(inputDetails) << '\n' << std::flush;

            _hsCore->_currentInputVideoDetails = inputDetails;
            _hsCore->_currentOutputVideoDetails = inputDetails;
            _spHs->SetResolution(inputDetails._width, inputDetails._height);

            _hsCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _hsCore = PipelineCore::Create(_spHs, histogramStatsCorePipelineHandler);

    _coreIspStack.push(_hsCore);

    return true;
}

bool CoreIspPipeline::InitCcm()
{
    const uint32_t CCM_UID = _coreIspBaseUID;
    auto spVvpCcmInst = _spHapi->CreateByUniqueID<Hapi::VvpCsc>(CCM_UID);
    if (!spVvpCcmInst)
    {
        return false;
    }

    _spCcm = ICsc::Create(spVvpCcmInst, 1920, 1080);
    if (!_spCcm)
    {
        return false;
    }

    _spCcm->SetOutputWidth(1920u);
    _spCcm->SetOutputHeight(1080u);

    _spCcmInterface = Ccm::Create(_spCcm);

    // Pipeline core

    auto ccmCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_ccmCore->_inputCores.size() == 1 &&
                _ccmCore->_outputCores.size() == 1);

            ImageConfig outputDetails = inputDetails;

            uint8_t bps_out = _spCcm->GetBitsPerSampleOut();
            switch(bps_out)
            {
            case 8:
                outputDetails.SetColourDepth(TColourDepth::BPC8);
                break;
            case 10:
                outputDetails.SetColourDepth(TColourDepth::BPC10);
                break;
            case 12:
                outputDetails.SetColourDepth(TColourDepth::BPC12);
                break;
            case 16:
                outputDetails.SetColourDepth(TColourDepth::BPC16);
                break;
            }

            TRACE << "[CCM " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, outputDetails) << '\n' << std::flush;

            _spCcm->SetOutputWidth(outputDetails._width);
            _spCcm->SetOutputHeight(outputDetails._height);

            

            _ccmCore->_currentInputVideoDetails = inputDetails;
            _ccmCore->_currentOutputVideoDetails = outputDetails;
            _ccmCore->_outputCores[0]->ApplyInputVideoInfo(outputDetails);

            return inputDetails;
        };
    _ccmCore = PipelineCore::Create(_spCcm, ccmCorePipelineHandler);

    _coreIspStack.push(_ccmCore);

    const uint32_t ROI_UID = _coreIspBaseUID;
    auto spVvpROIInst = _spHapi->CreateByUniqueID<Hapi::VvpROI>(ROI_UID);
    if (!spVvpROIInst)
    {
        return false;
    }

    _spIROI = ROIHelper::Create(spVvpROIInst);
    if (!_spIROI)
    {
        return false;
    }

    return true;
}

bool CoreIspPipeline::InitROI()
{
    // Pipeline core
    auto roiCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_ROICore->_inputCores.size() == 1 &&
                _ROICore->_outputCores.size() == 1);

            ImageConfig outputDetails = inputDetails;

            TRACE << "[ROI " << std::to_string(_pipeline_index) << "]:\t" << LOG_V(inputDetails) << '\n' << std::flush;

            _spIROI->SetResolution(inputDetails._width, inputDetails._height);

            _ROICore->_currentInputVideoDetails = inputDetails;
            _ROICore->_currentOutputVideoDetails = outputDetails;
            _ROICore->_outputCores[0]->ApplyInputVideoInfo(outputDetails);

            return inputDetails;
        };
    _ROICore = PipelineCore::Create(_spIROI, roiCorePipelineHandler);
    _coreIspOutputStack.push(_ROICore);

    return true;
}

bool CoreIspPipeline::InitScalers()
{
    const uint32_t H_SCALAR_UID = _coreIspBaseUID + 0U;
    const uint32_t V_SCALAR_UID = _coreIspBaseUID + 1U;

    auto spVvpHScalerInst = _spHapi->CreateByUniqueID<Hapi::VvpScaler>(H_SCALAR_UID);
    if (!spVvpHScalerInst)
    {
        TRACE
            << "Failed to create HScaler\n" << std::flush;
        return false;
    }

    auto spVvpVScalerInst = _spHapi->CreateByUniqueID<Hapi::VvpScaler>(V_SCALAR_UID);
    if (!spVvpVScalerInst)
    {
        TRACE
            << "Failed to create VScaler\n" << std::flush;
        return false;
    }

    _spHScaler = Scaler::Create(spVvpHScalerInst);
    if (!_spHScaler)
    {
        TRACE
            << "Failed to init HScaler\n" << std::flush;
        return false;
    }

    _spVScaler = Scaler::Create(spVvpVScalerInst);
    if (!_spVScaler)
    {
        TRACE
            << "Failed to init VScaler\n" << std::flush;
        return false;
    }

    // Resolution used to determine number of taps and lobes, so we do not need to generate coefficients for every resolution.
    _spHScaler->GenerateAndLoadCoefficients(std::pair<uint32_t, uint32_t>(3840, 2160), std::pair<uint32_t, uint32_t>(1920, 1080), std::pair<uint32_t, uint32_t>(0, 0));
    _spVScaler->GenerateAndLoadCoefficients(std::pair<uint32_t, uint32_t>(3840, 2160), std::pair<uint32_t, uint32_t>(1920, 1080), std::pair<uint32_t, uint32_t>(0, 0));
    _spHScaler->SetCoefficientBanks(0, 0);
    _spVScaler->SetCoefficientBanks(0, 0);

    // HScaler Pipeline Core
    auto hScalerCorePipelineInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_hScalerCore->_outputCores.size() == 1 && _hScalerCore->_inputCores.size() == 1,
                    "The VVP Scaler must take both input and output.");

            ImageConfig outputDetails = _hScalerCore->_currentOutputVideoDetails;
            outputDetails.SetHeight(inputDetails.Height());

            if(_hScalerCore->_currentInputVideoDetails != inputDetails)
            {
                _hScalerCore->_currentInputVideoDetails = inputDetails;
                _spHScaler->SetInputResolution(inputDetails.Width(), inputDetails.Height());
            }
            if(_hScalerCore->_currentOutputVideoDetails != outputDetails)
            {
                _hScalerCore->_currentOutputVideoDetails = outputDetails;
                _spHScaler->SetOutputResolution(outputDetails.Width(), outputDetails.Height());
            }
            TRACE << "[HScaler " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, outputDetails) << '\n' << std::flush;

            if(_hScalerCore->_outputCores[0]->_currentInputVideoDetails != outputDetails)
            {
                _hScalerCore->_outputCores[0]->ApplyInputVideoInfo(outputDetails);
            }
            return outputDetails;
        };
    auto hScalerCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            ASSERT(_hScalerCore->_outputCores.size() == 1 && _hScalerCore->_inputCores.size() == 1,
                    "The VVP Scaler must take both input and output.");

            ImageConfig inputDetails = _hScalerCore->_currentInputVideoDetails;
            inputDetails.SetHeight(inputDetails.Height());

            if(_hScalerCore->_currentInputVideoDetails != inputDetails)
            {
                _hScalerCore->_currentInputVideoDetails = inputDetails;
                _spHScaler->SetInputResolution(inputDetails.Width(), inputDetails.Height());
            }
            if(_hScalerCore->_currentOutputVideoDetails != outputDetails)
            {
                _hScalerCore->_currentOutputVideoDetails = outputDetails;
                _spHScaler->SetOutputResolution(outputDetails.Width(), outputDetails.Height());
            }
            TRACE << "[HScaler " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, outputDetails) << '\n' << std::flush;

            if(_hScalerCore->_inputCores[0]->_currentOutputVideoDetails != inputDetails)
            {
                _hScalerCore->_inputCores[0]->ApplyOutputVideoInfo(inputDetails);
            }
            return inputDetails;
        };
    _hScalerCore = PipelineCore::Create(_spHScaler, hScalerCorePipelineInputHandler, hScalerCorePipelineOutputHandler);

    // VScaler Pipeline Core
    auto vScalerCorePipelineInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_vScalerCore->_outputCores.size() == 1 && _vScalerCore->_inputCores.size() == 1,
                    "The VVP Scaler must take both input and output.");

            ImageConfig outputDetails = _vScalerCore->_currentOutputVideoDetails;
            outputDetails.SetWidth(_vScalerCore->_currentOutputVideoDetails.Width());

            if(_vScalerCore->_currentInputVideoDetails != inputDetails)
            {
                _vScalerCore->_currentInputVideoDetails = inputDetails;
                _spVScaler->SetInputResolution(inputDetails.Width(), inputDetails.Height());
            }
            if(_vScalerCore->_currentOutputVideoDetails != outputDetails)
            {
                _vScalerCore->_currentOutputVideoDetails = outputDetails;
                _spVScaler->SetOutputResolution(outputDetails.Width(), outputDetails.Height());
            }
            TRACE << "[VScaler " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, outputDetails) << '\n' << std::flush;

            if(_vScalerCore->_outputCores[0]->_currentOutputVideoDetails != outputDetails)
            {
                _vScalerCore->_outputCores[0]->ApplyInputVideoInfo(outputDetails);
            }
            return outputDetails;
        };
    auto vScalerCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            ASSERT(_vScalerCore->_outputCores.size() == 1 && _vScalerCore->_inputCores.size() == 1,
                    "The VVP Scaler must take both input and output.");

            ImageConfig inputDetails = _vScalerCore->_currentInputVideoDetails;
            inputDetails.SetWidth(outputDetails.Width());

            if(_vScalerCore->_currentInputVideoDetails != inputDetails)
            {
                _vScalerCore->_currentInputVideoDetails = inputDetails;
                _spVScaler->SetInputResolution(inputDetails.Width(), inputDetails.Height());
            }
            if(_vScalerCore->_currentOutputVideoDetails != outputDetails)
            {
                _vScalerCore->_currentOutputVideoDetails = outputDetails;
                _spVScaler->SetOutputResolution(outputDetails.Width(), outputDetails.Height());
            }
            TRACE << "[VScaler " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, outputDetails) << '\n' << std::flush;

            if(_vScalerCore->_inputCores[0]->_currentOutputVideoDetails != inputDetails)
            {
                _vScalerCore->_inputCores[0]->ApplyOutputVideoInfo(inputDetails);
            }
            return inputDetails;
        };
    _vScalerCore = PipelineCore::Create(_spVScaler, vScalerCorePipelineInputHandler, vScalerCorePipelineOutputHandler);

    _coreIspOutputStack.push(_hScalerCore);
    _coreIspOutputStack.push(_vScalerCore);

    return true;
}

bool CoreIspPipeline::InitWarp()
{
    // Prepare FPGA and HW access objects
    const uint32_t WARP_UID = _coreIspBaseUID;
    auto spHapiWarp = _spHapi->CreateByUniqueID<Hapi::VvpWarp>(WARP_UID);

    if (!spHapiWarp)
    {
        WARN << "Failed to get Warp hapi core\n";
        return false;
    }

    std::size_t dma_bus_offset = _pipeline_index*0x200000000;
    _spWarpAdapter = SwApi::WarpAdapter::Create(spHapiWarp, 2, dma_bus_offset);

    if (!_spWarpAdapter)
    {
        WARN << "Failed to create Warp Adapter!\n";
        return false;
    }

    // Pipeline cores
    // Warp has two. The reason for this is to enable partial pipeline updates, so WarpInput does not automatically
    // feed forward into WarpOutput

    auto warpCoreInputPipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_warpInputCore->_inputCores.size() == 1);

            TRACE << "[Warp Input " << std::to_string(_pipeline_index) << "]: " << LOG_V(inputDetails) << '\n' << std::flush;

            if(_warpInputCore->_currentInputVideoDetails != inputDetails)
            {
                const bool warpResChanged = (_warpInputCore->_currentInputVideoDetails.Width() != inputDetails.Width()) ||
                    (_warpInputCore->_currentInputVideoDetails.Height() != inputDetails.Height());

                _warpInputCore->_currentInputVideoDetails = inputDetails;
                _warpInputCore->_currentInputVideoDetails.SetColourDepth(TColourDepth::BPC10);

                if(_warpInputCore && _warpInputCore->_currentInputVideoDetails.IsValid() && 
                    _warpOutputCore && _warpOutputCore->_currentOutputVideoDetails.IsValid())
                {
                    // Reconfigure and update warp only if either resolution has changed
                    // Frame rate change is not relevant in this application
                    if(warpResChanged)
                    {
                        uint32_t outputFullHeight = 0;

                        _spWarpAdapter->SetVideoResolution(_warpInputCore->_currentInputVideoDetails.Width(), _warpInputCore->_currentInputVideoDetails.Height(),
                                                        _warpOutputCore->_currentOutputVideoDetails.Width(), _warpOutputCore->_currentOutputVideoDetails.Height(),
                                                        outputFullHeight, _warpOutputCore->_currentOutputVideoDetails.FrameRate() * 100);

                        std::cout << "[Warp " << std::to_string(_pipeline_index) << "]: Updating warp: Input: " << _warpInputCore->_currentInputVideoDetails.Width() << "x" << _warpInputCore->_currentInputVideoDetails.Height()
                        << " Output: " << _warpOutputCore->_currentOutputVideoDetails.Width() << "x" << _warpOutputCore->_currentOutputVideoDetails.Height() << "\n" << std::flush;

                        _spWarpAdapter->Update();
                    }
                }
            }

            return _warpInputCore->_currentInputVideoDetails;
        };
    _warpInputCore = PipelineCore::Create(_spWarpAdapter, warpCoreInputPipelineHandler);

    auto warpCoreOutputPipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            ASSERT(_warpOutputCore->_outputCores.size() == 1);

            TRACE << "[Warp Output " << std::to_string(_pipeline_index) << "]: " << LOG_V(outputDetails) << '\n' << std::flush;

            if(_warpOutputCore->_currentOutputVideoDetails != outputDetails)
            {
                _warpOutputCore->_currentOutputVideoDetails = outputDetails;
                _warpOutputCore->_currentOutputVideoDetails.SetColourDepth(TColourDepth::BPC10);
                if(_warpInputCore && (_warpInputCore->_currentInputVideoDetails.Width() != 0) && 
                    _warpOutputCore && (_warpOutputCore->_currentOutputVideoDetails.Width() != 0))
                {
                    uint32_t outputFullHeight = 0;

                    _spWarpAdapter->SetVideoResolution(_warpInputCore->_currentInputVideoDetails.Width(), _warpInputCore->_currentInputVideoDetails.Height(),
                                                    _warpOutputCore->_currentOutputVideoDetails.Width(), _warpOutputCore->_currentOutputVideoDetails.Height(),
                                                    outputFullHeight, _warpOutputCore->_currentOutputVideoDetails.FrameRate() * 100);

                    std::cout << "[Warp " << std::to_string(_pipeline_index) << "]: Updating warp: Input: " << _warpInputCore->_currentInputVideoDetails.Width() << "x" << _warpInputCore->_currentInputVideoDetails.Height()
                    << " Output: " << _warpOutputCore->_currentOutputVideoDetails.Width() << "x" << _warpOutputCore->_currentOutputVideoDetails.Height() << "\n" << std::flush;

                    _spWarpAdapter->Update();                   
                }
            }

            if(_vScalerCore)
            {
                if(_warpOutputCore && (_warpOutputCore->_currentOutputVideoDetails.Width() != 0))
                {   
                    _warpInputCore->ApplyInputVideoInfo(_warpOutputCore->_currentOutputVideoDetails);
                    _vScalerCore->ApplyOutputVideoInfo(_warpOutputCore->_currentOutputVideoDetails);
                }
            }

            return _warpOutputCore->_currentOutputVideoDetails;
        };
    _warpOutputCore = PipelineCore::Create(_spWarpAdapter, nullptr, warpCoreOutputPipelineOutputHandler);

    _preFbCore = _warpInputCore;
    _postFbCore = _warpOutputCore;

    return true;
}


bool CoreIspPipeline::InitVfb()
{
    const uint32_t VFB_UID = _coreIspBaseUID;
    _spVvpVfb = _spHapi->CreateByUniqueID<Hapi::VvpVfb>(VFB_UID);
    if (!_spVvpVfb) return false;

    auto spVvpVfbInst = _spVvpVfb->GetInstance();

    int err = intel_vvp_vfb_init(spVvpVfbInst, spVvpVfbInst->core_instance.base);
    if (err != kIntelVvpCoreOk )
    {
        TRACE << "Failed to intialise VFB instance\n" << std::flush;
        return false;
    }

    const uint32_t FB_WIDTH = 1920;
    const uint32_t FB_HEIGHT = 1080;

    ::intel_vvp_core_set_img_info_width(spVvpVfbInst, FB_WIDTH);
    ::intel_vvp_core_set_img_info_height(spVvpVfbInst, FB_HEIGHT);
    ::intel_vvp_core_set_img_info_interlace(spVvpVfbInst, 0);
    ::intel_vvp_core_set_img_info_subsampling(spVvpVfbInst, 0x3);
    ::intel_vvp_core_set_img_info_cositing(spVvpVfbInst, 0);
    ::intel_vvp_core_set_img_info_colorspace(spVvpVfbInst, 0);

    intel_vvp_vfb_start_output(spVvpVfbInst);

    // Pipeline core
    auto vfbCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            ASSERT(_vfbCore->_inputCores.size() == 1 && _vfbCore->_outputCores.size() == 1);

            TRACE << "[Framebuffer " << std::to_string(_pipeline_index) << "]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

            auto spVvpVfbInst = _spVvpVfb->GetInstance();

            intel_vvp_vfb_stop_output(spVvpVfbInst);

            ::intel_vvp_core_set_img_info_height(spVvpVfbInst,
                                                inputDetails._height);
            ::intel_vvp_core_set_img_info_width(spVvpVfbInst,
                                                inputDetails._width);

            intel_vvp_vfb_start_output(spVvpVfbInst);

            _vfbCore->_currentInputVideoDetails = inputDetails;
            _vfbCore->_currentOutputVideoDetails = inputDetails;
            _vfbCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    _vfbCore = PipelineCore::Create(_spVvpVfb, vfbCorePipelineHandler);

    _preFbCore = _vfbCore;
    _postFbCore = _vfbCore;

    return true;
}

bool CoreIspPipeline::InitSensorProfiling()
{
    _spProfile = std::make_shared<SensorCalibrationProfile>();

    const std::string white_balance_ctl_name = "WhiteBlncCtl" + std::to_string(_pipeline_index);
    _spWBController = std::make_shared<WhiteBalanceController>(_spProfile,
                                                            _spBlc,
                                                            _spAnr,
                                                            _spVc,
                                                            _spWbs,
                                                            _spWbsSwitchRouter,
                                                            _spWbc,
                                                            _spCcmInterface,
                                                            white_balance_ctl_name.c_str()
                                                        );

    _spAutoExposure = std::make_shared<AutoExposureController>(_spHs, _spCcmInterface);


    auto blackLevelCb = [](std::array<uint32_t, 4> blp){

        auto ui = VvpIspDemo::Get()->GetUi();

        if(ui)
            ui->UpdateBlackLevel(blp);

    };

    _spWBController->SetBlackLevelCallback(blackLevelCb);

    return true;
}

void CoreIspPipeline::LinkCoreIspPipelineCores()
{
    auto lastCore = _preFbCore;
    while (_coreIspOutputStack.size() > 0)
    {
        auto core = _coreIspOutputStack.top();
        if(lastCore)
        {
            lastCore->ConnectInput(core);
        }
        else
        {
            _coreIspOutputCore = core;
        }
        lastCore = std::move(core);
        _coreIspOutputStack.pop();
    };

    while (_coreIspStack.size() > 0)
    {
        auto core = _coreIspStack.top();
        if(lastCore)
        {
            lastCore->ConnectInput(core);
        }
        else
        {
            _coreIspOutputCore = core;
        }
        lastCore = std::move(core);
        _coreIspStack.pop();
    };
    lastCore->ConnectInput(_coreIspInputCore);
}

void CoreIspPipeline::Start()
{
    auto spHs = GetHs();

    if(spHs)
        spHs->Start();
}