/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "Bls.h"
#include "intel_vvp_bls.h"

#include <unistd.h>

namespace SwApi
{
    std::shared_ptr<Bls> Bls::Create(Hapi::VvpBlsPtr spBls) 
    {
        return std::make_shared<Bls>(spBls);
    }


    Bls::Bls(Hapi::VvpBlsPtr spVvpBls):
        VvpCoreBase("BLS"),
        _spVvpBls(spVvpBls) 
    { 
        optical_black_region.h_start = 0xFFFF;
        optical_black_region.v_start = 0xFFFF;
        optical_black_region.h_end = 0xFFFF;
        optical_black_region.v_end = 0xFFFF;


        _maxBitValue = (1 << intel_vvp_bls_get_bits_per_sample_in(_spVvpBls->GetInstance())) - 1;
    }

    void Bls::WaitForPendingRead()
    {
        // Wait for read to finish before applying settings
        while (_read_pending.load(std::memory_order_relaxed) != (int)ReadState::NONE)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    TCfaPhase Bls::GetCfaPhase()
    {
        return static_cast<TCfaPhase>(intel_vvp_bls_get_cfa_phase(_spVvpBls->GetInstance()));
    }

    bool Bls::SetCfaPhase(TCfaPhase cfa_phase)
    {
        // Wait for read to finish before applying settings
        WaitForPendingRead();

        return (intel_vvp_bls_set_cfa_phase(_spVvpBls->GetInstance(), static_cast<uint8_t>(cfa_phase)) == kIntelVvpCoreOk);
    }

    uint8_t Bls::GetBps()
    {
        return intel_vvp_bls_get_bits_per_sample_out(_spVvpBls->GetInstance());
    }

    bool Bls::SetResolution(uint32_t width, uint32_t height)
    {
        bool ret = true;
        ret = ret && (intel_vvp_core_set_img_info_width(_spVvpBls->GetInstance(), width) == kIntelVvpCoreOk);
        ret = ret && (intel_vvp_core_set_img_info_height(_spVvpBls->GetInstance(), height) == kIntelVvpCoreOk);
        return ret;
    }

    std::pair<uint32_t, uint32_t> Bls::GetResolution() const
    {
        uint32_t width = intel_vvp_core_get_img_info_width(_spVvpBls->GetInstance());
        uint32_t height = intel_vvp_core_get_img_info_height(_spVvpBls->GetInstance());

        return {width, height};
    }

    bool Bls::GetOpticalBlackRegion(uint16_t* h_start, uint16_t* v_start, uint16_t* h_end, uint16_t* v_end)
    {
        *h_start = optical_black_region.h_start;
        *v_start = optical_black_region.v_start;
        *h_end = optical_black_region.h_end;
        *v_end = optical_black_region.v_end;

        return true;
    }

    BlackRegion Bls::GetCombinedOpticalBlackRegion(void)
    {
        BlackRegion region;

        GetOpticalBlackRegion(&region.h_start, &region.v_start, &region.h_end, &region.v_end);

        return region;
    }

    bool Bls::SetOpticalBlackRegion(uint16_t h_start, uint16_t v_start, uint16_t h_end, uint16_t v_end, UpdatePolicy policy)
    {
        auto update_hw = [this, h_start, v_start, h_end, v_end]()->bool {
            // Wait for read to finish before applying settings
            WaitForPendingRead();

            int rv[] = {
                (optical_black_region.h_start != h_start) ? intel_vvp_bls_set_h_start(_spVvpBls->GetInstance(), h_start) : kIntelVvpCoreOk,
                (optical_black_region.v_start != v_start) ? intel_vvp_bls_set_v_start(_spVvpBls->GetInstance(), v_start - 1) : kIntelVvpCoreOk,
                (optical_black_region.h_end != h_end) ? intel_vvp_bls_set_h_end(_spVvpBls->GetInstance(), h_end + 1) : kIntelVvpCoreOk,
                (optical_black_region.v_end != v_end) ? intel_vvp_bls_set_v_end(_spVvpBls->GetInstance(), v_end) : kIntelVvpCoreOk
            };

            return (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpBlsCommitPendingErr) &&
                (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpBlsCommitPendingErr) &&
                (rv[2] == kIntelVvpCoreOk || rv[2] == kIntelVvpBlsCommitPendingErr) &&
                (rv[3] == kIntelVvpCoreOk || rv[3] == kIntelVvpBlsCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
        {
            optical_black_region.h_start = h_start;
            optical_black_region.v_start = v_start;
            optical_black_region.h_end = h_end;
            optical_black_region.v_end = v_end;
        }
        else
            std::cerr << "[" << GetName() << "] Failed to set optical black region\n";

        return ret;
    }

    bool Bls::SetCombinedOpticalBlackRegion(const BlackRegion &region)
    {
        return SetOpticalBlackRegion(region.h_start, region.v_start, region.h_end, region.v_end);
    }

    bool Bls::GetFrameStats(uint32_t *stats_out)
    {
        ReadState read = (ReadState)_read_pending.load(std::memory_order_relaxed);
        if (read == ReadState::STATS)
        {
            // Wait for read to finish
            WaitForPendingRead();
            std::atomic_thread_fence(std::memory_order_relaxed);
            *stats_out = _results.stats_out;

            return _results.valid;
        }
        else 
        {
            _read_pending.store((int)ReadState::STATS, std::memory_order_relaxed);
            _results.valid = StatsFreezeHandshake();

            intel_vvp_bls_get_frame_stats(_spVvpBls->GetInstance(), &_results.stats_out);
            *stats_out = _results.stats_out;

            intel_vvp_bls_set_freeze_stats_request(_spVvpBls->GetInstance(), false);

            std::atomic_thread_fence(std::memory_order_relaxed);
            _read_pending.store((int)ReadState::NONE, std::memory_order_relaxed);

            return _results.valid;
        }
    }

    bool Bls::GetCfaAccumulatorValues(uint32_t* cfa_00, uint32_t* cfa_01, uint32_t* cfa_10, uint32_t* cfa_11)
    {
        ReadState read = (ReadState)_read_pending.load(std::memory_order_relaxed);
        if (read == ReadState::CFA)
        {
            // Wait for read to finish before applying settings
            WaitForPendingRead();
            std::atomic_thread_fence(std::memory_order_relaxed);
            *cfa_00 = _results.cfa_00;
            *cfa_01 = _results.cfa_01;
            *cfa_10 = _results.cfa_10;
            *cfa_11 = _results.cfa_11;

            return _results.valid;
        }
        else
        {
            _read_pending.store((int)ReadState::CFA, std::memory_order_relaxed);
            _results.valid = StatsFreezeHandshake();

            intel_vvp_bls_get_cfa_00_sum(_spVvpBls->GetInstance(), &_results.cfa_00);
            intel_vvp_bls_get_cfa_01_sum(_spVvpBls->GetInstance(), &_results.cfa_01);
            intel_vvp_bls_get_cfa_10_sum(_spVvpBls->GetInstance(), &_results.cfa_10);
            intel_vvp_bls_get_cfa_11_sum(_spVvpBls->GetInstance(), &_results.cfa_11);
            *cfa_00 = _results.cfa_00;
            *cfa_01 = _results.cfa_01;
            *cfa_10 = _results.cfa_10;
            *cfa_11 = _results.cfa_11;

            intel_vvp_bls_set_freeze_stats_request(_spVvpBls->GetInstance(), false);

            std::atomic_thread_fence(std::memory_order_relaxed);
            _read_pending.store((int)ReadState::NONE, std::memory_order_relaxed);

            return _results.valid;
        }
    }

    bool Bls::GetAverageRegionCfaAccumulatorValues(uint32_t* cfa_00, uint32_t* cfa_01, uint32_t* cfa_10, uint32_t* cfa_11)
    {
        uint32_t horizPhaseTotals[2];
        uint32_t verticalPhaseTotals[2];
        uint32_t preOffsetPhaseTotals[4];

        uint32_t cfaValues[4];
        uint8_t cfaPhase = intel_vvp_bls_get_cfa_phase(_spVvpBls->GetInstance());

        uint32_t averagedAccumulators[4];

        GetCfaAccumulatorValues(&cfaValues[0], &cfaValues[1], &cfaValues[2], &cfaValues[3]);

        uint32_t regionWidth = (optical_black_region.h_end - optical_black_region.h_start) + 1;
        uint32_t regionHeight = (optical_black_region.v_end - optical_black_region.v_start) + 1;
        
        horizPhaseTotals[0] = (regionWidth >> 1) + (regionWidth & 0x1);
        horizPhaseTotals[1] = regionWidth >> 1;
        
        verticalPhaseTotals[0] = (regionHeight >> 1) + (regionHeight & 0x1);
        verticalPhaseTotals[1] = regionHeight >> 1;

        preOffsetPhaseTotals[0] = verticalPhaseTotals[0] * horizPhaseTotals[0]; // 00
        preOffsetPhaseTotals[1] = verticalPhaseTotals[0] * horizPhaseTotals[1]; // 01
        preOffsetPhaseTotals[2] = verticalPhaseTotals[1] * horizPhaseTotals[0]; // 10
        preOffsetPhaseTotals[3] = verticalPhaseTotals[1] * horizPhaseTotals[1]; // 11

        for (uint32_t i = 0; i < 4; i++)
        {
            if (preOffsetPhaseTotals[cfaPhase ^ i])
            {
                averagedAccumulators[i] = cfaValues[i] / preOffsetPhaseTotals[cfaPhase ^ i];
            }
            else
            {
                averagedAccumulators[i] = 0;
            }
        }

        *cfa_00 = averagedAccumulators[0];
        *cfa_01 = averagedAccumulators[1];
        *cfa_10 = averagedAccumulators[2];
        *cfa_11 = averagedAccumulators[3];

        return true;
    }

    bool Bls::GetPedestalsAndScalarsForCurrentConfig(uint32_t* cfa_00, uint32_t* cfa_01, uint32_t* cfa_10, uint32_t* cfa_11,
                                                                    uint32_t* scalar_00, uint32_t* scalar_01, uint32_t* scalar_10, uint32_t* scalar_11)
    {
        uint32_t cfaQuantityVals[4];
        uint32_t adjustedScalars[4];

        // First, calculate the pedestals and scalars
        GetAverageRegionCfaAccumulatorValues(&cfaQuantityVals[0], &cfaQuantityVals[1], &cfaQuantityVals[2], &cfaQuantityVals[3]);

        for (uint32_t i = 0; i < 4; i++)
        {
            if (cfaQuantityVals[i] > (BLS_SCALAR_UNORM_ONE >> 1))
            {
                // The BLC can't increase a pixel strength larger than 2x.
                // This means that if the pedestal is greater than 0.5, we can just set
                // the scalar to the max value possible, rather than dealing with floats.
                cfaQuantityVals[i] = (BLS_SCALAR_UNORM_ONE >> 1);
                adjustedScalars[i] = BLS_SCALAR_UNORM_ONE;
            } else {
                // Formula for color pedestal:
                // Full scalar = 1 / (1 - pedestal) = 1.X, where X is the fractional component we're
                // after. Therefore frac component = (1 / (1 - pedestal)) - 1
                // This then needs multiplying with unorm value of 1 to get the correct value to pass to the BLC
                adjustedScalars[i] = (uint32_t)(((_maxBitValue / (float)(_maxBitValue - cfaQuantityVals[i])) - 1.0f) * BLS_SCALAR_UNORM_ONE);
            }
        }

        *cfa_00 = cfaQuantityVals[0];
        *cfa_01 = cfaQuantityVals[1];
        *cfa_10 = cfaQuantityVals[2];
        *cfa_11 = cfaQuantityVals[3];
        *scalar_00 = adjustedScalars[0];
        *scalar_01 = adjustedScalars[1];
        *scalar_10 = adjustedScalars[2];
        *scalar_11 = adjustedScalars[3];

        return true;
    }

    bool Bls::IsCommitPending()
    {
        return intel_vvp_bls_commit_is_pending(_spVvpBls->GetInstance());
    }

    bool Bls::CommitSettings()
    {
        return (intel_vvp_bls_commit(_spVvpBls->GetInstance()) == kIntelVvpCoreOk);
    }

    bool Bls::StatsFreezeHandshake()
    {
        // TODO - replace this with a sub thread rather than stalling the main thread
        intel_vvp_bls_set_freeze_stats_request(_spVvpBls->GetInstance(), true);
        usleep(20000); // Wait a 50fps frame, should be a reasonable window.
        for (int i = 0; i < 5; i++)
        {
            if (intel_vvp_bls_stats_are_frozen(_spVvpBls->GetInstance()))
            {
                return true;
            }
            usleep(20000);
        }
        std::cout << "Handshake has failed, stats_frozen status bit not set" << std::endl;
        return false;
    }

    bool Bls::SetStatsFreeze(bool state)
    {
        return intel_vvp_bls_set_freeze_stats_request(_spVvpBls->GetInstance(), state) == kIntelVvpCoreOk;
    }

}; // namespace SwApi
