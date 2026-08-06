/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "HistogramStats.h"
#include "intel_vvp_hs.h"
#include <thread>
#include <chrono>
#include <vector>


using namespace std::chrono_literals;


namespace SwApi
{
    std::shared_ptr<HistogramStats> HistogramStats::Create(Hapi::VvpHsPtr spHs)
    {
        return std::make_shared<HistogramStats>(spHs);
    }

    HistogramStats::HistogramStats(Hapi::VvpHsPtr spVvpHs):
        VvpCoreBase{"HS"},
        _spVvpHs{spVvpHs}
    {
        _num_luma_bins = intel_vvp_hs_get_num_hist_bins(_spVvpHs->GetInstance());
        intel_vvp_hs_set_freeze_stats_request(_spVvpHs->GetInstance(), true);
    }

    HistogramStats::~HistogramStats()
    {
        if (_histogramThread.joinable())
        {
            {
                std::unique_lock lock(_histogramThreadMutex);
                _stopHistogramThread = true;
            }

            _histogramThreadCv.notify_all();
            _histogramThread.join();
        }
    }

    void HistogramStats::Start()
    {
        if(!_histogramThread.joinable())
        {
            _histogramThread = std::thread(&HistogramStats::HistogramUpdateLoop, this);
        }
    }    

    bool HistogramStats::SetResolution(uint32_t width, uint32_t height)
    {
        std::scoped_lock lock(_hwMutex);

        bool ret = true;

        ret = ret && (intel_vvp_core_set_img_info_width(_spVvpHs->GetInstance(), width) == kIntelVvpCoreOk);
        ret = ret && (intel_vvp_core_set_img_info_height(_spVvpHs->GetInstance(), height) == kIntelVvpCoreOk);

        if(ret)
        {
            _width = width;
            _height = height;
        }

        return ret;
    }

    std::pair<uint32_t, uint32_t> HistogramStats::GetResolution() const
    {
        std::scoped_lock lock(_hwMutex);
        return {_width, _height};
    }

    bool HistogramStats::SetRegionOfInterest(const Hs::RegionOfInterest &region, UpdatePolicy policy)
    {

        auto update_hw = [this, &region]()->bool {
            std::scoped_lock lock(_hwMutex);

            int rv[] = {
                (region.h_start != _roi.h_start) ? intel_vvp_hs_set_h_start(_spVvpHs->GetInstance(), region.h_start) : kIntelVvpCoreOk,
                (region.v_start != _roi.v_start) ? intel_vvp_hs_set_v_start(_spVvpHs->GetInstance(), region.v_start) : kIntelVvpCoreOk,
                (region.h_end != _roi.h_end) ? intel_vvp_hs_set_h_end(_spVvpHs->GetInstance(), region.h_end) : kIntelVvpCoreOk,
                (region.v_end != _roi.v_end) ? intel_vvp_hs_set_v_end(_spVvpHs->GetInstance(), region.v_end) : kIntelVvpCoreOk
            };

            bool ret = (
                ((rv[0] == kIntelVvpCoreOk) || (rv[0] == kIntelVvpHsCommitPendingErr)) &&
                ((rv[1] == kIntelVvpCoreOk) || (rv[1] == kIntelVvpHsCommitPendingErr)) &&
                ((rv[2] == kIntelVvpCoreOk) || (rv[2] == kIntelVvpHsCommitPendingErr)) &&
                ((rv[3] == kIntelVvpCoreOk) || (rv[3] == kIntelVvpHsCommitPendingErr))
            );
            if(ret)
            {
                _roi.h_start = region.h_start;
                _roi.v_start = region.v_start;
                _roi.h_end = region.h_end;
                _roi.v_end = region.v_end;
            }
            return ret;
        };

        bool ret = UpdateHw(update_hw, policy);

        if(!ret)
        {
            std::cerr << "[" << GetName() << "] Failed to update Region of Interest settings\n";
        }

        return ret;
    }

    Hs::RegionOfInterest HistogramStats::GetRegionOfInterest() const
    {
        std::scoped_lock lock(_hwMutex);
        return _roi;
    }

    HistogramStats::HistogramSubscriptionId HistogramStats::RegisterHistogramObserver(HistogramCallback callback)
    {
        if (!callback)
            return 0;

        std::scoped_lock lock(_observerMutex);

        const auto subscriptionId = _nextHistogramSubscriptionId;
        _histogramObservers[subscriptionId] = std::move(callback);
        ++_nextHistogramSubscriptionId;

        return subscriptionId;
    }

    bool HistogramStats::UnregisterHistogramObserver(HistogramSubscriptionId subscriptionId)
    {
        std::scoped_lock lock(_observerMutex);
        return (_histogramObservers.erase(subscriptionId) == 1);
    }

    std::shared_ptr<Hs::TableResults> HistogramStats::ReadHistogram()
    {
        std::shared_ptr<Hs::TableResults> histogramData{nullptr};

        std::scoped_lock lockHw(_hwMutex);

        const auto instance = _spVvpHs->GetInstance();

        // Make sure freeze stats request set to 0 first
        intel_vvp_hs_set_freeze_stats_request(instance, false);

        // Wait until the stats are updating
        bool statsFrozen = true;
        std::size_t attempts = 0;

        while(statsFrozen && attempts < 10)
        {
            std::this_thread::sleep_for(10ms);
            statsFrozen = intel_vvp_hs_stats_are_frozen(instance);
            ++attempts;
        }

        if(statsFrozen)
        {
            std::cerr << "Histogram stats are frozen, handshake failed.\n";
            return histogramData;
        }

        // Request stats freeze for reading
        intel_vvp_hs_set_freeze_stats_request(instance, true);

        attempts = 0;

        while(!statsFrozen && attempts < 10)
        {
            std::this_thread::sleep_for(100ms);
            statsFrozen = intel_vvp_hs_stats_are_frozen(instance);
            ++attempts;
        }

        if(!statsFrozen)
        {
            std::cerr << "Histogram stats failed to freeze, handshake failed.\n";
            return histogramData;
        }

        histogramData = std::make_shared<Hs::TableResults>();

        if(histogramData)
        {
            // Read the histogram data
            histogramData->frame_luma_bins.num_luma_bins = _num_luma_bins;
            histogramData->roi_luma_bins.num_luma_bins = _num_luma_bins;

            intel_vvp_hs_read_frame_hist_array(instance, histogramData->frame_luma_bins.luma_bins);
            intel_vvp_hs_read_roi_hist_array(instance, histogramData->roi_luma_bins.luma_bins);

            // Unfreeze stats for next round of updates
            intel_vvp_hs_set_freeze_stats_request(instance, false);

            histogramData->valid = true;
        }
        else
            std::cerr << "Unable to allocate Histogram data object\n";

        return histogramData;
    }

    void HistogramStats::HistogramUpdateLoop()
    {
        bool go = true;

        while(go)
        {
            const auto histogramData = ReadHistogram();

            if(histogramData)
                NotifyHistogramObservers(histogramData);

            std::unique_lock lock(_histogramThreadMutex);

            if (_histogramThreadCv.wait_for(lock, std::chrono::milliseconds(HISTOGRAM_UPDATE_INTERVAL_MS), [this](){ return _stopHistogramThread; }))
            {
                go = false;
            }
        }
    }

    void HistogramStats::NotifyHistogramObservers(const std::shared_ptr<Hs::TableResults>& histogramData)
    {
        std::scoped_lock lock(_observerMutex);

        for (const auto& [subscriptionId, callback] : _histogramObservers)
        {
            (void)subscriptionId;
            callback(histogramData);
        }
    }

    std::shared_ptr<Hs::TableResults> HistogramStats::GetHistogram()
    {
        return ReadHistogram();
    }    

    // bool HistogramStats::StatsFreezeHandshake()
    // {
    //     std::scoped_lock lock(_hwMutex);

    //     // REMEMBER - THIS IS INVERTED
    //     intel_vvp_hs_set_freeze_stats_request(_spVvpHs->GetInstance(), false);

    //     for (int i = 0; i < 10; i++)
    //     {
    //         // We do an extra sleep here to allow the mem copy to happen
    //         std::this_thread::sleep_for(50ms);

    //         if (!intel_vvp_hs_stats_are_frozen(_spVvpHs->GetInstance()))
    //         {
    //             return true;
    //         }
    //     }
    //     std::cout << "[HS] Handshake has failed, stats_frozen status bit not set" << std::endl;
    //     return false;
    // }

    bool HistogramStats::CommitSettings()
    {
        return (intel_vvp_hs_commit(_spVvpHs->GetInstance()) == kIntelVvpCoreOk);
    }

    bool HistogramStats::IsCommitPending()
    {
        return intel_vvp_hs_commit_is_pending(_spVvpHs->GetInstance());
    }    

}; // namespace SwApi