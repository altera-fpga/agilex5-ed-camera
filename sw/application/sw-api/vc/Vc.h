/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "VvpCoreBase.h"
#include "HapiVvpVc.h"
#include "IspCommon.h"
#include <cstdint>

namespace SwApi
{

static constexpr uint32_t VC_COLOURPLANES_MAX = 4;

class Vc : public VvpCoreBase
{
public:

    static std::shared_ptr<Vc> Create(Hapi::VvpVcPtr spVc,
                                       uint32_t initialOutputWidth,
                                       uint32_t initialOutputHeight);

    Vc(Hapi::VvpVcPtr spVc,
       uint32_t initialOutputWidth,
       uint32_t initialOutputHeight);

    bool SetBypass(bool bypass, UpdatePolicy policy = UpdatePolicy::Async());
    bool GetBypass();

    uint8_t GetPip();

    bool IsRunning();

    bool HasCfaEnable();
    bool SetCfaPhase(TCfaPhase to, UpdatePolicy policy = UpdatePolicy::Async());
    TCfaPhase GetCfaPhase();

    
    uint16_t GetVerticalNumBlocks();
    uint16_t GetHorizontalNumBlocks();

    uint16_t GetMaxGainMeshPointsAllowed();
    bool GetPerColorGainEnable();

    bool SetUpVCForMesh(uint16_t meshXDim, uint16_t meshYDim, UpdatePolicy policy = UpdatePolicy::Async());

    bool UploadMeshCpLut(const uint32_t cp_idx, const uint32_t* meshCpLutValues);
    bool UploadStepLut(const uint32_t* stepLutValues);

    bool SetResolution(uint32_t width, uint32_t height);
    std::pair<uint32_t, uint32_t> GetResolution();

    bool CheckIfResolutionFitsExistingStepMeshConfig(uint16_t newWidth, uint16_t newHeight);

private:
    bool SetBlockPixCount(uint16_t blockPixCount, bool autoUpdateRampFrac = true);
    bool SetBlockLineCount(uint16_t blockLineCount, bool autoUpdateRampFrac = true);

    bool CommitSettings() override;
    bool IsCommitPending() override;

    Hapi::VvpVcPtr _spVc = nullptr;

    uint16_t _width;
    uint16_t _height;

    bool _bypass;
    TCfaPhase _cfaPhase;
    uint16_t _meshXDim;
    uint16_t _meshYDim;
};

} // namespace SwApi
