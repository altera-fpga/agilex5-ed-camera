/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include <cstdint>
#include <mutex>
#include <thread>
#include <chrono>
#include "Semaphore.h"
#include "IInputCallback.h"
#include "IResultsCallback.h"
#include "InferenceTypes.h"
#include "ICoreDLAIntf.h"
#include <stream_controller_RemoteContext.h>

class OpenVinoIE
{
public:
    OpenVinoIE(uint32_t streaming_io_base, uint32_t streaming_io_count, bool bypass_output_layout_transform, const std::weak_ptr<IResultsCallback>& wp_results_callback);
    ~OpenVinoIE();
    std::string GetArchName();
    const std::string& GetConfigFile();

    void SetNoNetwoksFound();
    ModelPtr Import(const std::string& import_filename, NetworkType network_type);

    void SetCurrentNetwork(const ModelPtr& model, std::string network_name);
    bool Start(bool state);

    bool SendUserMessage(UserMessageType messageType, void* pPayload=nullptr, uint32_t size=0, UserPayload* pUserPayloadResults=nullptr);
    std::string GetStatus();
    void SetDiagnostics(bool diagnostics);

private:
    void StartInference(InferenceRequest& infer_req);
    void OpenVinoIEInferenceCompleteThread();
    void OpenVinoIEResultsThread();

private:
    OpenVinoIE() = delete;
    OpenVinoIE(const OpenVinoIE& other) = delete;
    OpenVinoIE& operator=(const OpenVinoIE& other) = delete;

    ov::Core _core;
    uint32_t _next_network_handle;
    ModelPtr _current_model;
    std::string _current_network_name;
    uint32_t _streaming_io_base;
    uint32_t _streaming_io_count;
    bool _bypass_output_layout_transform;
    std::weak_ptr<IResultsCallback> _wp_results_callback;
    Semaphore _pending_infer_requests_sem;
    std::mutex _pending_infer_requests_cs;
    std::list<InferenceRequest> _pending_infer_requests;
    InferenceRequest _current_infer_request;
    Semaphore _pending_results_sem;
    std::mutex _pending_results_cs;
    std::list<std::shared_ptr<tCoreDLAOutput>> _pending_results;
    std::thread _inference_complete_thread;
    std::thread _results_thread;
    bool _results_thread_stop;
    static std::string _xmlConfigFile;
    bool _flush;
    StreamControllerRemoteContext _sc_context;

    uint32_t _inference_count;
    uint32_t _inference_count_fps;
    uint32_t _lastVfwLowResIsrCount;
    uint32_t _lastVfwFullResIsrCount;
    uint32_t _osd_count;

    uint32_t _last_width;
    uint32_t _last_height;

    bool _no_networks_found;
    bool _out_of_inferences;
    bool _diagnostics;
    bool _reset_diagnostics;
};
