/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiOCS.h"
#include "ICoreDLAIntf.h"
#include "OpenVinoIE.h"
#include "IResultsCallback.h"
#include "IResultProcessor.h"
#include <filesystem>
#include <map>
#include <queue>
#include <vector>

namespace SwApi 
{
    class CoreDLAProcessor : public ICoreDLAIntf, public IResultsCallback
    {
    public:
        CoreDLAProcessor(std::shared_ptr<Hapi::IHapi> spHapi, NetworkType network_type);
        ~CoreDLAProcessor();

        void LoadNetwork() override;
        void Start() override;
        void Stop() override;

        void RegisterResultsCallback(ICoreDLAResultsCallback resultsCallback) override;
        bool SendUserMessage(UserMessageType messageType, void* pPayload=nullptr, uint32_t size=0, UserPayload* pUserPayloadResults=nullptr) override;

        std::string GetStatus() override;

        // IResultsCallback interface
        void ReceiveResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput) override;
        void NoNetworkFound() override;
        void OutOfInferences() override;
        void StatusMessage(const char* message) override;

        const std::vector<ModelPtr>& GetNetworkList() override;

        void SetNetworkHandle(uint32_t network_handle) override;
        void SetDetectionThreshold(float detectionThreshold) override;
        void SetIOUThreshold(float iouThreshold) override;
        void SetDiagnostics(bool diagnostics) override;
    private:
        std::vector<std::filesystem::path> GetNetworkFilenames();
    private:
        std::shared_ptr<Hapi::IHapi> _spHapi;
        std::string _bitstream_arch_hash;
        std::string _bitstream_build_version;
        std::string _bitstream_arch_name;
        static std::string _arch_name;

        uint32_t _network_handle;
        std::shared_ptr<OpenVinoIE> _open_vino_ie;
        std::vector<ModelPtr> _network_model_list;
        std::map<uint32_t, std::shared_ptr<IResultProcessor>> _yolo_results_processors;

        ICoreDLAResultsCallback _resultsCallback;

        std::chrono::milliseconds _processing_time;
        std::chrono::milliseconds _rendering_time;
        uint32_t _count;

        bool _diagnostics;

        std::mutex _results_queue_cs;
        std::queue<std::shared_ptr<std::vector<YoloClassificationItem>>> _results_queue;
    };

} // namespace SwApi
