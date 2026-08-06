/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once


#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <chrono>
#include <thread>
#include <unordered_map>
#include "HistogramStatsTypes.h"
#include "HapiVvpHs.h"
#include "VvpCoreBase.h"


namespace SwApi
{
    class HistogramStats : public VvpCoreBase
    {
    public:
        using HistogramCallback = std::function<void(const std::shared_ptr<Hs::TableResults>&)>;
        using HistogramSubscriptionId = uint64_t;

        static std::shared_ptr<HistogramStats> Create(Hapi::VvpHsPtr spHs);

        HistogramStats(Hapi::VvpHsPtr spHs);
        ~HistogramStats();
        HistogramStats(const HistogramStats& other) = delete;
        HistogramStats& operator=(const HistogramStats& other) = delete;
        HistogramStats(HistogramStats&& other) = delete;
        HistogramStats& operator=(HistogramStats&& other) = delete;


        bool SetResolution(uint32_t width, uint32_t height);
        std::pair<uint32_t, uint32_t> GetResolution() const;

        bool SetRegionOfInterest(const Hs::RegionOfInterest &region, UpdatePolicy policy = UpdatePolicy::Async());
        Hs::RegionOfInterest GetRegionOfInterest() const;

        HistogramSubscriptionId RegisterHistogramObserver(HistogramCallback callback);
        bool UnregisterHistogramObserver(HistogramSubscriptionId subscriptionId);

        std::shared_ptr<Hs::TableResults> GetHistogram();

        void Start();

    private:
        void HistogramUpdateLoop();
        void NotifyHistogramObservers(const std::shared_ptr<Hs::TableResults>& histogramData);

        bool CommitSettings() override;
        bool IsCommitPending() override;
        
        // ToDo: The two methods below use inverted
        // freeze stats bit logic. Investigate the reason why
        // Remove the code if no longer needed
        // The methods have been replaced with the code
        // that follows the documentation
        //bool StatsFreezeHandshake();
        //void ReadHistogramOrig();

        std::shared_ptr<Hs::TableResults> ReadHistogram();

        Hapi::VvpHsPtr _spVvpHs;

        uint16_t _num_luma_bins;
        uint32_t _width;
        uint32_t _height;

        Hs::RegionOfInterest _roi = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

        mutable std::mutex _hwMutex;

        mutable std::mutex _observerMutex;
        std::unordered_map<HistogramSubscriptionId, HistogramCallback> _histogramObservers;
        HistogramSubscriptionId _nextHistogramSubscriptionId{1};

        static constexpr uint32_t HISTOGRAM_UPDATE_INTERVAL_MS = 100;
        std::thread _histogramThread;
        mutable std::mutex _histogramThreadMutex;
        std::condition_variable _histogramThreadCv;
        bool _stopHistogramThread{false};
    };

} // namespace SwApi