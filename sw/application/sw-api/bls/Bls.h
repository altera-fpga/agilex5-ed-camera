/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "BlsUtils.h"
#include "HapiVvpBls.h"
#include "VvpCoreBase.h"
#include "IspCommon.h"

namespace SwApi
{

class Bls : public VvpCoreBase
{
public:
    static std::shared_ptr<Bls> Create(Hapi::VvpBlsPtr spBls);

    Bls(Hapi::VvpBlsPtr spBls);

    bool SetResolution(uint32_t width, uint32_t height);
    std::pair<uint32_t, uint32_t> GetResolution() const;

    uint8_t GetBps();

    // Core specific
    TCfaPhase GetCfaPhase();
    bool SetCfaPhase(TCfaPhase cfa_phase);

    bool GetOpticalBlackRegion(uint16_t* h_start,
                               uint16_t* v_start,
                               uint16_t* h_end,
                               uint16_t* v_end);
    BlackRegion GetCombinedOpticalBlackRegion(void);

    bool SetOpticalBlackRegion(uint16_t h_start,
                               uint16_t v_start,
                               uint16_t h_end,
                               uint16_t v_end,
                               UpdatePolicy policy = UpdatePolicy::Async());
    bool SetCombinedOpticalBlackRegion(const BlackRegion& region);

    bool GetFrameStats(uint32_t *stats_out);

    bool GetCfaAccumulatorValues(uint32_t* cfa_00,
                                 uint32_t* cfa_01,
                                 uint32_t* cfa_10,
                                 uint32_t* cfa_11);

    bool GetAverageRegionCfaAccumulatorValues(
                                 uint32_t* cfa_00,
                                 uint32_t* cfa_01,
                                 uint32_t* cfa_10,
                                 uint32_t* cfa_11);

    bool GetPedestalsAndScalarsForCurrentConfig(uint32_t* cfa_00, 
                                                uint32_t* cfa_01, 
                                                uint32_t* cfa_10, 
                                                uint32_t* cfa_11,
                                                uint32_t* scalar_00, 
                                                uint32_t* scalar_01, 
                                                uint32_t* scalar_10, 
                                                uint32_t* scalar_11);

    bool StatsFreezeHandshake();
    bool SetStatsFreeze(bool state);

private:
    void WaitForPendingRead();
    bool CommitSettings() override;
    bool IsCommitPending() override;
    Hapi::VvpBlsPtr _spVvpBls;

    BlackRegion optical_black_region;

    uint32_t _maxBitValue;
    
    static constexpr uint32_t BLS_DECIMAL_BITS = 18;
    static constexpr uint32_t BLS_SCALAR_UNORM_ONE = (1 << BLS_DECIMAL_BITS) - 1;

    enum ReadState 
    {
        NONE = 0,
        STATS = 1,
        CFA = 2
    };
    std::atomic<int> _read_pending = 0;
    struct BlsResults
    {
        bool valid;
        uint32_t stats_out;
        uint32_t cfa_00;
        uint32_t cfa_01;
        uint32_t cfa_10;
        uint32_t cfa_11;
    } _results;

};

} // namespace SwApi
