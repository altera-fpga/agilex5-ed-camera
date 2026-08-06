/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <math.h>
#include <fstream>
#include <stdexcept>
#include "OpenVinoIE.h"
#define RUNTIME_DLA_PLUGIN
#include <dla_plugin_config.hpp>
#include <dlfcn.h>
#include <pthread.h>
#include "FrameQueueConfig.h"
extern std::string _bin_directory;

// Forward the OpenVINO results through the IDPU::IResultsCallback
// for consistency
OpenVinoIE::OpenVinoIE(uint32_t streaming_io_base, uint32_t streaming_io_count, bool bypass_output_layout_transform, const std::weak_ptr<IResultsCallback>& wp_results_callback)
: _core("plugins.xml")
, _next_network_handle(1)
, _current_model()
, _streaming_io_base(streaming_io_base)
, _streaming_io_count(streaming_io_count)
, _bypass_output_layout_transform(bypass_output_layout_transform)
, _wp_results_callback(wp_results_callback)
, _pending_infer_requests_sem(0)
, _results_thread_stop(true)
, _flush(false)
, _inference_count(0U)
, _inference_count_fps(0U)
, _lastVfwLowResIsrCount(0U)
, _lastVfwFullResIsrCount(0U)
, _osd_count(0U)
, _last_width(0U)
, _last_height(0U)
, _no_networks_found(false)
, _out_of_inferences(false)
, _diagnostics(false)
, _reset_diagnostics(true)
{
}

OpenVinoIE::~OpenVinoIE()
{
    Start(false);
}

std::string OpenVinoIE::GetArchName()
{
    std::string arch_name = _core.get_property("FPGA", DLIAPlugin::properties::arch_path);
    size_t last_slash = arch_name.rfind('/');
    if(last_slash != arch_name.npos) {
        arch_name = arch_name.substr(last_slash + 1);
    }
    size_t last_dot = arch_name.rfind('.');
    if(last_dot != arch_name.npos) {
        arch_name = arch_name.substr(0, last_dot);
    }
    return arch_name;
}

std::string OpenVinoIE::_xmlConfigFile;

const std::string& OpenVinoIE::GetConfigFile() 
{
    _xmlConfigFile = _bin_directory + "/plugins.xml";
    return _xmlConfigFile;
}

void OpenVinoIE::SetNoNetwoksFound()
{
    // Default input resizer
    tModelInputSize modelImputSize {640, 360, 1284, 181, 61632};
    SendUserMessage(UserMessageType::UserMessageType_ModelInputSize, &modelImputSize, sizeof(modelImputSize));
    _no_networks_found = true;
}

ModelPtr OpenVinoIE::Import(const std::string& import_filename, NetworkType network_type)
{
    ModelPtr model;
    std::cout << "Importing " << import_filename << std::endl;
    std::ifstream network_model_stream(import_filename, std::ifstream::binary);
    if (network_model_stream.good())
    {
        // If dropSourceBuffers is 0, no input buffers are dropped
        // If dropSourceBuffers is 1, then 1 buffer is processed, 1 gets dropped
        // If dropSourceBuffers is 2, then 1 buffer is processed, 2 get dropped, etc.
        uint32_t dropSourceBuffers = 0;

        // for debugging
        //dlopen("./lib/libAltera_FPGA_AI_Suite_OpenVINO_plugin.so", RTLD_NOW);
        //dlopen("./lib/libhapi_mmd.so", RTLD_NOW);

        std::string compiled_model_device;
        {
            char c;
            while (network_model_stream.get(c) && c != '\0') {
            compiled_model_device += c;
            }
        }


        _core.set_property("FPGA", 
                        {{DLIAPlugin::properties::streaming_drop_source_buffers.name(), std::to_string(dropSourceBuffers)},
                        {DLIAPlugin::properties::num_streaming_inference_requests.name(), _streaming_io_count},
                        {DLIAPlugin::properties::external_streaming.name(), "YES"},
                        {DLIAPlugin::properties::bypass_output_layout_transform.name(), _bypass_output_layout_transform ? "YES" : "NO"}}
                        );

        model = std::make_shared<Model>();
        model->_network_handle = _next_network_handle++;
        model->_network_type = network_type;
        try {
            model->_ovCompiledModel = _core.import_model(network_model_stream, "HETERO:FPGA,CPU");
        }
        catch(const std::runtime_error& e)
        {
            std::string error_msg = e.what();
            if(error_msg.find("Inference on FPGA exited with a license error") != std::string::npos)
            {
                _current_model = nullptr;
                _flush = true;
                _out_of_inferences = true;
                _results_thread_stop = true;
                model = nullptr;
            }
            else
            {
                throw e;
            }
        }
        if(!_out_of_inferences)
        {
            for(uint32_t i = 0; i < _streaming_io_count; i++) 
            {
                InferenceRequest infer_req;
                infer_req._ovInferRequest = model->_ovCompiledModel.create_infer_request();
                infer_req._network_handle = model->_network_handle;
                infer_req._network_type = model->_network_type;
                infer_req._model = model;
                ov::CompiledModel ovModel = infer_req._ovInferRequest.get_compiled_model();
                auto shape = ovModel.input().get_shape();
                int width = static_cast<int>(ovModel.input().get_shape()[3]);
                int height = static_cast<int>(ovModel.input().get_shape()[2]);
                
                infer_req._width = static_cast<uint32_t>(width);
                uint32_t padded_height = static_cast<uint32_t>(height);
                infer_req._height = (infer_req._width * 9U)/16U;
                infer_req._width_lt = (((infer_req._width + 1) + 1) / 2)*16/4;
                infer_req._height_lt = (infer_req._height + 2) / 2;
                infer_req._offset_lt = (((padded_height - infer_req._height) / 2)/2) * ((infer_req._width/2 + 1) * 16 * 2);
                model->_model_infer_requests.push_back(infer_req);
            }
        }
    }
    std::cout << "Importing Complete" << std::endl;

    if(!_sc_context)
    {
        _sc_context = _core.create_context("FPGA", {}).as<StreamControllerRemoteContext>();
    }

    return model;
}

void OpenVinoIE::SetCurrentNetwork(const ModelPtr& model, std::string network_name)
{
    _current_model = model;
    _current_network_name = std::move(network_name);
}

void OpenVinoIE::StartInference(InferenceRequest& infer_req)
{
    std::lock_guard<std::mutex> lock(_pending_infer_requests_cs);
    if((_last_width != infer_req._width) || (_last_height != infer_req._height))
    {
        tModelInputSize modelImputSize;
        modelImputSize.MODEL_INPUT_VIDEO_WIDTH = infer_req._width;
        modelImputSize.MODEL_INPUT_VIDEO_HEIGHT = infer_req._height;
        modelImputSize.MODEL_INPUT_LT_WIDTH = infer_req._width_lt;
        modelImputSize.MODEL_INPUT_LT_HEIGHT = infer_req._height_lt;
        modelImputSize.MODEL_INPUT_LT_OFFSET = infer_req._offset_lt;

        SendUserMessage(UserMessageType::UserMessageType_ModelInputSize, &modelImputSize, sizeof(modelImputSize));
        _last_width = infer_req._width;
        _last_height = infer_req._height;
    }
    infer_req._ovInferRequest.start_async();
    _pending_infer_requests.push_back(infer_req);
    _pending_infer_requests_sem.notify();
}

bool OpenVinoIE::Start(bool state)
{
    if (state) 
    {
        if(_current_model)
        {
            while(!_current_model->_model_infer_requests.empty())
            {
                InferenceRequest infer_req = _current_model->_model_infer_requests.front();
                _current_model->_model_infer_requests.pop_front();
                StartInference(infer_req);
            }
        }

        _results_thread_stop = _out_of_inferences;
        _inference_complete_thread = std::thread([this]{OpenVinoIEInferenceCompleteThread();});
        _results_thread = std::thread([this]{OpenVinoIEResultsThread();});
    }
    else
    {
        _flush = true;

        if(!_results_thread_stop) 
        {
            _results_thread_stop = true;
            _inference_complete_thread.join();
            _results_thread.join();
        }
        _flush = false;
    }

    return true;
}

bool OpenVinoIE::SendUserMessage(UserMessageType messageType, void* pPayload, uint32_t size, UserPayload* pUserPayloadResults)
{
    bool rc = false;
    if(!_sc_context)
    {
        _sc_context = _core.create_context("FPGA", {}).as<StreamControllerRemoteContext>();
    }
    if(_sc_context)
    {
        rc = _sc_context.UserMessage(messageType, pPayload, size, pUserPayloadResults);
    }
    return rc;
}

std::string OpenVinoIE::GetStatus()
{
    std::string rc;
    if(_current_model)
    {
        if(_current_model->_ovCompiledModel)
        {
            rc = _current_model->_ovCompiledModel.get_property(DLIAPlugin::properties::stream_controller_status);
        }
    }
    return rc;
}

void OpenVinoIE::SetDiagnostics(bool diagnostics)
{
    _diagnostics = diagnostics;
    if(_diagnostics)
    {
        _reset_diagnostics = true;
    }
}

void OpenVinoIE::OpenVinoIEInferenceCompleteThread()
{
#ifdef DEBUG    
    std::cout << "OpenVinoIE::OpenVinoIEInferenceCompleteThread started" << std::endl;
#endif /* DEBUG */

    auto fps_start_timestamp = std::chrono::high_resolution_clock::now();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    CPU_SET(3, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    int priority = sched_get_priority_min(SCHED_RR);
    struct	sched_param param;
    param.sched_priority = priority;
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);

    pthread_setname_np(pthread_self(), "ov_inf_comp");

    bool pending_infer_requests_empty = true;

    while (!_results_thread_stop || !pending_infer_requests_empty)
    {
        bool infer_request_valid = false;
        {
            std::lock_guard<std::mutex> lock(_pending_infer_requests_cs);
            if (!_pending_infer_requests.empty()) 
            {
                _current_infer_request = _pending_infer_requests.front();
                infer_request_valid = true;
                _pending_infer_requests.pop_front();
            }
        }
        if(!infer_request_valid && !_flush)
        {
            if (_pending_infer_requests_sem.wait(100))
            {
                std::lock_guard<std::mutex> lock(_pending_infer_requests_cs);
                if (!_pending_infer_requests.empty()) 
                {
                    _current_infer_request = _pending_infer_requests.front();
                    infer_request_valid = true;
                    _pending_infer_requests.pop_front();
                }
            }
        }
        if (infer_request_valid) 
        {
            _inference_count++;

            bool res = false;
            while (!res) 
            {
                try
                {
                    res = _current_infer_request._ovInferRequest.wait_for(std::chrono::milliseconds(100));
                }
                catch(const std::runtime_error& e)
                {
                    std::string error_msg = e.what();
                    if(error_msg.find("Inference on FPGA exited with a license error") != std::string::npos)
                    {
                        _current_model = nullptr;
                        _flush = true;
                        _out_of_inferences = true;
                        _results_thread_stop = true;
                    }
                }                
            }
            const ModelPtr& inference_model = _current_infer_request._model.lock();
            if(inference_model)
            {
                if(!_flush) 
                {
                    try
                    {
                        auto inference_end_timestamp = std::chrono::high_resolution_clock::now();

                        // if network has changed, skip processing
                        if(inference_model == _current_model)
                        {
                            uint32_t num_outputs = inference_model->_ovCompiledModel.outputs().size();
                            std::shared_ptr<tCoreDLAOutput> spCoreDLAOutput = std::make_shared<tCoreDLAOutput>();
                            spCoreDLAOutput->network_handle = _current_infer_request._network_handle;
                            spCoreDLAOutput->network_type = _current_infer_request._network_type;
                            for(uint32_t idx = 0; idx < num_outputs; idx++)
                            {
                                spCoreDLAOutput->output_tensors.emplace_back(_current_infer_request._ovInferRequest.get_output_tensor(idx));
                            }
                            spCoreDLAOutput->inference_count = _inference_count;
                            spCoreDLAOutput->inference_buffer = _inference_count % _streaming_io_count;

                            if(_inference_count_fps == 0)
                            {
                                fps_start_timestamp = std::chrono::high_resolution_clock::now();
                            }
                            _inference_count_fps++;
                            if((_inference_count_fps == 100) || _reset_diagnostics)
                            {
                                const std::chrono::duration<double> run_time = inference_end_timestamp - fps_start_timestamp;
                                float inference_rate = (float)_inference_count_fps / run_time.count();
                                if(_diagnostics && !_reset_diagnostics)
                                {
                                    std::cout << _current_network_name << std::endl;
                                    std::cout << "inference_rate " << inference_rate << " ips" << std::endl;
                                }
                                _inference_count_fps = 0;
                                fps_start_timestamp = std::chrono::high_resolution_clock::now();

                                if(_diagnostics)
                                {
                                    auto split = [](const std::string& s, char delimiter)->std::vector<std::string> {
                                        std::vector<std::string> result;
                                        std::stringstream ss (s);
                                        std::string item;

                                        while (std::getline(ss, item, delimiter)) {
                                            result.push_back (item);
                                        }

                                        return result;
                                    };

                                    std::string status = GetStatus();
                                    std::vector<std::string> fields = split(status, ',');
#ifdef FULL_STATUS
                                    uint32_t statusCode = static_cast<uint32_t>(std::stoul(fields[0]));
                                    uint32_t statusLineNumber = static_cast<uint32_t>(std::stoul(fields[1]));
                                    uint32_t numReceivedSourceBuffers = static_cast<uint32_t>(std::stoul(fields[2]));
                                    uint32_t numScheduledInferences = static_cast<uint32_t>(std::stoul(fields[3]));
                                    uint32_t numExecutedJobs = static_cast<uint32_t>(std::stoul(fields[4]));
#endif
                                    uint32_t vfwLowResIsrCount = static_cast<uint32_t>(std::stoul(fields[5]));
                                    uint32_t vfwFullResIsrCount = static_cast<uint32_t>(std::stoul(fields[6]));
#ifdef FULL_STATUS
                                    std::cout << "statusCode " << statusCode << std::endl;
                                    std::cout << "statusLineNumber " << statusLineNumber << std::endl;
                                    std::cout << "numReceivedSourceBuffers " << numReceivedSourceBuffers << std::endl;
                                    std::cout << "numScheduledInferences " << numScheduledInferences << std::endl;
                                    std::cout << "numExecutedJobs " << numExecutedJobs << std::endl;
#endif
                                    if(_reset_diagnostics)
                                    {
                                        _reset_diagnostics = false;
                                    }
                                    else
                                    {
                                        float low_res_rate = (float)(vfwLowResIsrCount - _lastVfwLowResIsrCount) / run_time.count();
                                        float full_res_rate = (float)(vfwFullResIsrCount - _lastVfwFullResIsrCount) / run_time.count();
                                        float osd_rate = (float)(_osd_count) / run_time.count();
                                        std::cout << "low_res_rate " << low_res_rate << " fps" << std::endl;
                                        std::cout << "full_res_rate " << full_res_rate << " fps" << std::endl;
                                        std::cout << "osd_rate " << osd_rate << " fps" << std::endl;
                                    }
                                    _lastVfwLowResIsrCount = vfwLowResIsrCount;
                                    _lastVfwFullResIsrCount = vfwFullResIsrCount;
                                }
                                _osd_count = 0U;
                            }
                            // Return the inference request to queue, now we've made a copy
                            {
                                StartInference(_current_infer_request);
                            }
                            #if 1
                            {
                                // Deliver to the callback
                                std::lock_guard<std::mutex> lock(_pending_results_cs);
                                //_current_infer_request.reset_state();
                                if(_pending_results.size() <= 1)
                                {
                                    _pending_results.emplace_back(spCoreDLAOutput);
                                    _pending_results_sem.notify();
                                }
                            }
                            #endif
                        }
                        else
                        {
                            std::lock_guard<std::mutex> lock(_pending_results_cs);
                            inference_model->_model_infer_requests.push_back(_current_infer_request);

                            if(_current_model)
                            {
                                if(!_current_model->_model_infer_requests.empty())
                                {
                                    InferenceRequest infer_req = _current_model->_model_infer_requests.front();
                                    _current_model->_model_infer_requests.pop_front();
                                    StartInference(infer_req);
                                }
                            }
                        }
                    }
                    catch (std::exception& e) {
                        std::cerr << e.what();
                    }
                }
                else
                {
                    std::lock_guard<std::mutex> lock(_pending_results_cs);
                    inference_model->_model_infer_requests.push_back(_current_infer_request);
                }
            }
        }
        {
            std::lock_guard<std::mutex> lock(_pending_infer_requests_cs);
            pending_infer_requests_empty = _pending_infer_requests.empty();
        }
    }

#ifdef DEBUG
    std::cout << "OpenVinoIE::OpenVinoIEInferenceCompleteThread stopped" << std::endl;
#endif /* DEBUG */    
}

void OpenVinoIE::OpenVinoIEResultsThread()
{
#ifdef DEBUG
    std::cout << "OpenVinoIE::OpenVinoIEResultsThread started" << std::endl;
#endif /* DEBUG */

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    CPU_SET(3, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    int priority = sched_get_priority_min(SCHED_RR);
    struct	sched_param param;
    param.sched_priority = priority;
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);

    pthread_setname_np(pthread_self(), "ov_results");

    while (!_results_thread_stop)
    {
        std::shared_ptr<tCoreDLAOutput> spCoreDLAOutput;

        bool output_tensor_valid = false;
        {
            std::lock_guard<std::mutex> lock(_pending_results_cs);
            if (!_pending_results.empty()) 
            {
                spCoreDLAOutput = _pending_results.front();
                output_tensor_valid = true;
            }
        }
        while(!output_tensor_valid)
        {
            _pending_results_sem.wait(100);
            if(_no_networks_found) {
                std::shared_ptr<IResultsCallback> sp_results_callback = _wp_results_callback.lock();
                if(sp_results_callback != nullptr)
                {
                    sp_results_callback->NoNetworkFound();
                }
            }
            if (_results_thread_stop) {
                break;
            }
            {
                std::lock_guard<std::mutex> lock(_pending_results_cs);
                if (!_pending_results.empty()) 
                {
                    spCoreDLAOutput = _pending_results.front();
                    output_tensor_valid = true;
                }
            }
        }

        std::shared_ptr<IResultsCallback> sp_results_callback = _wp_results_callback.lock();
        if(output_tensor_valid && (sp_results_callback != nullptr))
        {
            _osd_count++;
            sp_results_callback->ReceiveResults(spCoreDLAOutput);
        }

        if(output_tensor_valid)
        {
            _pending_results.pop_front();
        }
    }

    if(_out_of_inferences)
    {
        std::shared_ptr<IResultsCallback> sp_results_callback = _wp_results_callback.lock();
        if(sp_results_callback != nullptr)
        {
            sp_results_callback->OutOfInferences();
        }
    }

#ifdef DEBUG    
    std::cout << "OpenVinoIE::OpenVinoIEResultsThread stopped" << std::endl;
#endif /* DEBUG */
}


///////////////////////////////////////////////////////////////////////////////

