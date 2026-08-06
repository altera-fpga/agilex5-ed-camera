/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <stack>

#include "PipelineCore.h"

#include "Bls.h"
#include "IClipper.h"
#include "ISwitch.h"
#include "IDpc.h"
#include "Anr.h"
#include "Blc.h"
#include "SwitchRouter.h"
#include "Vc.h"
#include "Wbs.h"
#include "Wbc.h"
#include "IDemosaic.h"
#include "Scaler.h"
#include "WarpAdapter.h"
#include "HistogramStats.h"
#include "ICsc.h"
#include "Ccm.h"
#include "HapiVvpVfb.h"
#include "IROI.h"

#include "VvpIspWarpOutputControls.h"
#include "WhiteBalance.h"
#include "AutoExposure.h"

class CoreIspPipeline : public std::enable_shared_from_this<CoreIspPipeline>
{
public:
    CoreIspPipeline(uint32_t pipeline_index, const std::shared_ptr<Hapi::IHapi>& spHapi, const uint32_t coreIspBaseUID);

    bool InitCoreIspPipeline();

    void SetInputCfaPhase(TCfaPhase inputCfaPhase);
    TCfaPhase GetInputCfaPhase() { return _inputCfaPhase; }
    void ResetClipper(uint32_t imageWidth, uint32_t imageHeight);
    void UpdateClipper(uint32_t& imageWidth, uint32_t& imageHeight);
    bool InitSensorProfiling();
    const ICameraPtr& GetActiveCamera() { return _activeCamera; }
    void SetActiveCamera(ICameraPtr pCamera);

    void InsertAdditionalCore(std::shared_ptr<VideoPipeline::PipelineCore> additionalCore);

    const std::shared_ptr<SwApi::Bls>& GetBls() { return _spBls; }
    const std::shared_ptr<SwApi::IClipper>& GetClipper() { return _spClipper; }

    const std::shared_ptr<SwApi::ISwitch>& GetVfwPostClipperSwitch() { return _spVfwClipperSwitch; }
    
    // ISP
    const std::shared_ptr<SwApi::IDpc>& GetDpc() { return _spDpc; }
    const std::shared_ptr<SwApi::Anr>& GetAnr() { return _spAnr; }
    const std::shared_ptr<SwApi::Blc>& GetBlc() { return _spBlc; }

    const std::shared_ptr<SwitchRouter>& GetSwitchRouter() { return _spWbsSwitchRouter; };

    const std::shared_ptr<SwApi::Vc>& GetVc() { return _spVc; }
    const std::shared_ptr<SwApi::Wbs>& GetWbs() { return _spWbs; }
    const std::shared_ptr<SwApi::Wbc>& GetWbc() { return _spWbc; }
    const std::shared_ptr<SwApi::IDemosaic>& GetDemosaic() { return _spDemosaic; }
    const std::shared_ptr<SwApi::HistogramStats>& GetHs() { return _spHs; }
    const std::shared_ptr<SwApi::Ccm>& GetCcm() { return _spCcmInterface; } // Note that the CCM uses the same interface as the CSC
    const IROIPtr& GetROI() { return _spIROI; }

    // Output
    const std::shared_ptr<SwApi::Scaler>& GetHScaler() { return _spHScaler; }
    const std::shared_ptr<SwApi::Scaler>& GetVScaler() { return _spVScaler; }
    const std::shared_ptr<SwApi::WarpAdapter>& GetWarpAdapter() { return _spWarpAdapter; }


    // Pipeline profile
    const std::shared_ptr<SensorCalibrationProfile>& GetProfile() { return _spProfile; }
    const std::shared_ptr<WhiteBalanceController>& GetWhiteBalanceController() { return _spWBController; }
    const std::shared_ptr<AutoExposureController>& GetAutoExposureController() { return _spAutoExposure; }

    const std::shared_ptr<VideoPipeline::PipelineCore>& GetInputCore() { return _coreIspInputCore; }
    const std::shared_ptr<VideoPipeline::PipelineCore>& GetOutputCore() { return _coreIspOutputCore; }

    void LinkCoreIspPipelineCores();

    void Start();

protected:
    bool InitBls();
    bool InitClipper();
    bool InitDpc();
    bool InitAnr();
    bool InitBlc();
    bool InitVc();
    bool InitRouterSwitch();
    bool InitWbs();
    bool InitWbc();
    bool InitDemosaic();
    bool InitHs();
    bool InitCcm();
    bool InitROI();
    bool InitScalers();
    bool InitWarp();
    bool InitVfb();

private:

    const uint32_t _pipeline_index;
    const std::shared_ptr<Hapi::IHapi> _spHapi;
    const uint32_t _coreIspBaseUID;

    ICameraPtr _activeCamera;

    std::shared_ptr<SwApi::Bls> _spBls;
    std::shared_ptr<SwApi::IClipper> _spClipper;

    std::shared_ptr<SwApi::ISwitch> _spVfwClipperSwitch;    

    std::shared_ptr<SwApi::IDpc> _spDpc;
    std::shared_ptr<SwApi::Anr> _spAnr;
    std::shared_ptr<SwApi::Blc> _spBlc;
    std::shared_ptr<SwApi::Vc> _spVc;
    std::shared_ptr<SwApi::Wbc> _spWbc;
    std::shared_ptr<SwitchRouter> _spWbsSwitchRouter;
    std::shared_ptr<SwApi::Wbs> _spWbs;
    std::shared_ptr<SwApi::IDemosaic> _spDemosaic;
    std::shared_ptr<SwApi::HistogramStats> _spHs;
    std::shared_ptr<SwApi::ICsc> _spCcm;
    std::shared_ptr<SwApi::Ccm> _spCcmInterface;
    std::shared_ptr<SwApi::Scaler> _spHScaler;
    std::shared_ptr<SwApi::Scaler> _spVScaler;
    std::shared_ptr<SwApi::WarpAdapter> _spWarpAdapter;
    Hapi::VvpVfbPtr _spVvpVfb;
    IROIPtr _spIROI;

    std::shared_ptr<VideoPipeline::PipelineCore> _blsCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _clipperCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _dpcCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _anrCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _blcCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _vcCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _wbcCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _wbsCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _demosaicCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _hsCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _ccmCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _hScalerCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _vScalerCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _warpInputCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _warpOutputCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _vfbCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _ROICore;

    std::shared_ptr<VideoPipeline::PipelineCore> _preFbCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _postFbCore;

    std::shared_ptr<VideoPipeline::PipelineCore> _coreIspInputCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _coreIspOutputCore;

    std::stack<std::shared_ptr<VideoPipeline::PipelineCore>> _coreIspStack;
    std::stack<std::shared_ptr<VideoPipeline::PipelineCore>> _coreIspOutputStack;
    std::shared_ptr<WhiteBalanceController> _spWBController;
    std::shared_ptr<AutoExposureController> _spAutoExposure;

    ImageConfig _currentInputVideoDetails;
    TCfaPhase _inputCfaPhase = TCfaPhase::RGGB;

    std::shared_ptr<SensorCalibrationProfile> _spProfile;
};
