/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <array>
#include <vector>

#include "CommonApplicationBase.h"
#include "CompileTimeConfiguration.h"
#include "AccessLogging.h"
#include "WebServer.h"

#include "TaskHandler.h"
#include "UdxVvpIspPipeline.h"
#include "StitchPipeline.h"

#include "Hapi.h"
#include "I2CControls.h"
#include "LED.h"
#include "Logging.h"

#include "CrsControls.h"
#include "CscControls.h"
#include "TpgControls.h"
#include "VvpIspAutoWhiteBalanceControls.h"
#include "VvpIspAutoExposureControls.h"

#include "VvpIspSourceSelectControls.h"
#include "SwitchControls.h"

#include "BlsControls.h"
#include "ClipperControls.h"

#include "DpcControls.h"
#include "AnrControls.h"
#include "BlcControls.h"
#include "VcControls.h"
#include "WbsControls.h"
#include "WbcControls.h"
#include "DemosaicControls.h"
#include "HsControls.h"
#include "UsmControls.h"
#include "CcmControls.h"
#include "VvpIspWarpOutputControls.h"
#include "WarpFullControls.h"
#include "HapiVvpWarp.h"
#include "TmoControls.h"
#include "TmoOverride.h"
#include "Lut3dControls.h"
#include "Screensaver.h"
#include "LogoControls.h"
#include "Lut1dControls.h"
#include "VvpIspVfwControls.h"
#include "CameraUiControls.h"
#include "OutputControls.h"
#include "HsPreview.h"

#include "MultiChannelAutoExposureControls.h"
#include "MultiChannelWhiteBalanceControls.h"
#include "StitchControls.h"

// Calibration controls
#include "VvpIspProfileBlackLevelControls.h"
#include "VvpIspProfileLoaderControls.h"
#include "VvpIspFrameReaderControls.h"
#include "VvpIspProfileOverviewControls.h"
#include "VvpIspProfileMasterRegionControls.h"
#include "VvpIspProfileVignetteCorrectionControls.h"
#include "VvpIspProfileWhiteBalanceControls.h"
#include "VvpIspProfileColourCorrectionControls.h"


class StitchTabbedUiControls
{
    public:
        StitchTabbedUiControls(std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> appCreateUiTabCB,
                        std::shared_ptr<UdxVvpIspPipeline> spPipeline, std::shared_ptr<StitchPipeline> spStitchPipeline, bool debugMode, bool powerUser, std::function<void(uint32_t)> warpCaptureCB = nullptr);
        ~StitchTabbedUiControls();
        StitchTabbedUiControls(const StitchTabbedUiControls& other) = delete;
        StitchTabbedUiControls& operator=(const StitchTabbedUiControls& other) = delete;
        StitchTabbedUiControls(StitchTabbedUiControls&& other) = delete;
        StitchTabbedUiControls& operator=(StitchTabbedUiControls&& other) = delete;

        void CreateInputConfigTab();
        void UpdateInputParams(const ImageConfig& params);

        void CreateISPTab();

        void CreateOutputConfigTab();
        void UpdateOutputParams(const ImageConfig& outputConfig);
        void UpdateMixerParams(const ImageConfig& mixerConfig);

        void CreateStitchTab();
        void CreateStatsTab();
        void CreateCalibrationTab();
        void CreateDebugTab();
        void PostTabCreationUpdates();
        void UpdateUiPostOutputChange(ImageConfig &config);
        void EnableUIUpdates();
        void UpdateBlackLevel(const std::array<uint32_t, 4>& blc);
        void UpdateOutputStatus(const uint32_t status, const std::string& format_str, const std::vector<uint32_t>& format_opts, const bool bpc10_support);
        void UpdateInputFrameReaderControls(const SwApi::VfrSourceMetadata& metadata);

        bool _uiInitialised;

        typedef struct sISPPipelineInputControls{
            std::shared_ptr<TopLevelUiTab> _spISPPipelineInputTab;
            std::shared_ptr<ClipperControls> _spClipperControls;
            std::shared_ptr<HistogramPreview> _spHistogramPreview;
            std::shared_ptr<VvpIspProfileLoaderControls> _spCameraProfileControls;
            std::shared_ptr<VvpIspAutoWhiteBalanceControls> _spAutoWhiteBalanceControls;
            std::shared_ptr<VvpIspAutoExposureControls> _spAutoExposureControls;
        } ISPPipelineInputControls;

        typedef struct sISPPipelineControls{
            std::shared_ptr<TopLevelUiTab> _spISPPipelineTab;
                std::shared_ptr<DpcControls> _spDpcControls;
                std::shared_ptr<AnrControls> _spAnrControls;
                std::shared_ptr<BlcControls> _spBlcControls;
                std::shared_ptr<VcControls> _spVcControls;
                std::shared_ptr<WbcControls> _spWbcControls;
                std::shared_ptr<DemosaicControls> _spDemosaicControls;
                std::shared_ptr<CcmControls> _spCcmControls;
        } ISPPipelineControls;

        typedef struct sISPStatisticsControles {
            std::shared_ptr<TopLevelUiTab> _spStatisticsTab;
                std::shared_ptr<BlsControls> _spBlsControls;
                std::shared_ptr<WbsControls> _spWbsControls;
                std::shared_ptr<HsControls> _spHsControls;
        } ISPStatisticsControls;

        std::shared_ptr<TopLevelUiTab> _spInputConfigTab;
            std::shared_ptr<VvpIspSourceSelectControls> _spSourceSelector;

            std::vector<CameraUiControlsPtr> _cameraControls;

            std::shared_ptr<TpgControls> _spTpgControls;
            std::shared_ptr<VvpIspFrameReaderControls> _spFrameReaderControls;

            std::shared_ptr<MultiChannelAutoExposureControls> _spMultiChannelAutoExposureControls;
            std::shared_ptr<MultiChannelWhiteBalanceControls> _spMultiChannelWhiteBalanceControls;

        std::vector<ISPPipelineInputControls> _ispPipelineInputControls;

        std::vector<ISPPipelineControls> _ispPipelineControls;

        std::shared_ptr<TopLevelUiTab> _spOutputConfigTab;
            std::vector<std::shared_ptr<WarpFullControls>> _vspWarpControls;
            std::shared_ptr<VvpIspWarpOutputControls> _spHdmiWarpOutputControls;
            std::shared_ptr<TmoControls> _spTmoControls;

            std::shared_ptr<UsmControls> _spUsmControls;

            // HDR related
            std::shared_ptr<Lut3dControls> _sp3dLutSrgbHlgControls;
            std::shared_ptr<Lut1dControls> _sp1dLutLinearSrgbControls;
            //////////////

            std::shared_ptr<Lut3dControls> _sp3dLutControls;
            std::shared_ptr<LogoControls> _spLogoControls;
            std::shared_ptr<Lut1dControls> _sp1dLutControls;
            std::shared_ptr<TpgControls> _spDisplayportTpgControls;
            std::shared_ptr<VvpIspVfwControls> _spVfwControls;

        std::shared_ptr<TopLevelUiTab> _spStitchTab;
            std::shared_ptr<StitchControls> _spStitchControls;

        std::vector<ISPStatisticsControls> _ispStatisticsControls;

        std::shared_ptr<TopLevelUiTab> _spCalibrationTab;
            std::shared_ptr<VvpIspProfileOverviewControls> _spProfileOverviewControls;
            std::shared_ptr<VvpIspProfileMasterRegionControls> _spProfileRegionSelectControls;
            std::shared_ptr<VvpIspProfileBlackLevelControls> _spProfileBlackLevelControls;
            std::shared_ptr<VvpIspProfileWhiteBalanceControls> _spProfileWhiteBalanceControls;
            std::shared_ptr<VvpIspProfileVignetteCorrectionControls> _spProfileVignetteCorrectionControls;
            std::shared_ptr<VvpIspProfileColourCorrectionControls> _spProfileColourCorrectionControls;

        std::shared_ptr<TopLevelUiTab> _spDebugTab;
            std::shared_ptr<SwitchControls> _spRgbSwitchControls;
            std::shared_ptr<SwitchControls> _spBayerSwitchControls;
            std::shared_ptr<I2CControls> _spI2CControls;

        std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> _appCreateUiTabCB;

        std::shared_ptr<TaskHandler> _spTaskHandler;
        std::shared_ptr<UdxVvpIspPipeline> _spPipeline;
        std::shared_ptr<StitchPipeline> _spStitchPipeline;
        std::shared_ptr<SwApi::ITmoOverride> _spTmoOverride;

        bool _debugMode = false;
        bool _powerUser = false;
        std::function<void(uint32_t)> _warpCaptureCB = nullptr;

        bool _warpHFlip = false;
        bool _warpVFlip = false;

    private:
        std::shared_ptr<VvpIspSourceSelectControls> CreateInputSelector();
        CameraUiControlsPtr _spActiveCameraUiControls[2];

        float _backedUpCameraGain = 1.0f;
        bool _autoExpEnableState = false;
        std::size_t _aeTaskHandle[2];
        std::size_t _awbTaskHandle[2];
        std::array<SwApi::HistogramStats::HistogramSubscriptionId, 2> _histogramPreviewSubscriptionIds = {0, 0};
        std::array<std::weak_ptr<SwApi::HistogramStats>, 2> _wpHistogramPreviewSources;

        std::shared_ptr<Screensaver> _spScreensaver;

        std::shared_ptr<OutputControls> _spOutputControls;

        std::size_t _warpUpdateCounter = 0;
};
