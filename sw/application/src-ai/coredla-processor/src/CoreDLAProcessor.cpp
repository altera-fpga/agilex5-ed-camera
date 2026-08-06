/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CoreDLAProcessor.h"
#include "FrameQueueConfig.h"
#include "HapiCoreDLA.h"
#include "YoloClassificationItem.h"
#include "YoloDetectionProcessor.h"
#include "YoloPoseProcessor.h"
#include "OcsUioHapiCore.h"

// Logging.h defines conflict with compiled_result header, so we need to undef them before including compiled_result headers and redefine them after.
#undef TRACE
#undef INFO
#undef WARN
#undef ERR
#undef FATAL

#include "compiled_result.h"

#include "Logging.h"

extern std::string _bin_directory;

#define BUILD_VERSION_CSR_OFFSET (ARCH_HASH_SIZE)
#define ARCH_NAME_CSR_OFFSET (ARCH_HASH_SIZE + BUILD_VERSION_SIZE)

static std::string arch_hash_to_string(const std::vector<uint32_t> &arch_hash) {
  std::stringstream s;
  for (size_t i = 0; i < ARCH_HASH_WORD_SIZE; ++i) {
    s << std::setfill('0') << std::setw(8) << std::hex << std::right << arch_hash[i] << " ";
  }

  return s.str();
}

static std::string read_string_from_bitstream_rom(Hapi::CoreDLAPtr spCoreDLA,
                                           const uint32_t str_word_size_in_bytes,
                                           const uint32_t str_offset_in_rom) {
  std::string str_from_rom;
  bool done = false;
  for (uint32_t i = 0; i < str_word_size_in_bytes && (!done); ++i) {
    uint32_t chunk = intel_coredla_csr_read(spCoreDLA->GetInstance(), str_offset_in_rom/4 + i);
    // Parse the int word into chars. Stops at any NUL char.
    for (int j = 0; j < 4; ++j) {
      char rom_char = (chunk >> (j * 8)) & 0xFF;
      if (rom_char == 0) {
        done = true;
        break;
      } else {
        str_from_rom.push_back(rom_char);
      }
    }
  }
  return str_from_rom;
}

static void parse_network_path(const std::filesystem::path& network_path, NetworkType& network_type, uint32_t& width, uint32_t& height)
{
    network_type = NetworkType::UNKNOWN;
    width = 0;
    height = 0;
    std::string network_filename = network_path.stem();

    if (network_filename.find("yolov8n_") != std::string::npos)
    {
        network_type = NetworkType::YOLOV8N;
    }
    if (network_filename.find("yolov8n-pose_") != std::string::npos)
    {
        network_type = NetworkType::YOLOV8N_POSE;
    }
    size_t res_2 = network_filename.rfind("_");
    if(res_2 != std::string::npos)
    {
        size_t res_1 = network_filename.rfind("_", res_2-1);
        if(res_1 != std::string::npos)
        {
            width = std::stoul(network_filename.substr(res_1 + 1, res_2-(res_1+1)));
            height = std::stoul(network_filename.substr(res_2 + 1));
        }
    }
}

static bool sort_network_path(const std::filesystem::path& a, const std::filesystem::path& b)
{
    bool rc = false;
    NetworkType a_network_type;
    uint32_t a_width;
    uint32_t a_height;
    NetworkType b_network_type;
    uint32_t b_width;
    uint32_t b_height;
    parse_network_path(a, a_network_type, a_width, a_height);
    parse_network_path(b, b_network_type, b_width, b_height);

    if(a_width < b_width)
    {
        rc = true;
    }
    else if(a_width > b_width)
    {
        rc = false;
    }
    else if(a_network_type == NetworkType::YOLOV8N)
    {
        rc = true;
    }
    else
    {
        rc = false;
    }
    return rc;
}

namespace SwApi 
{

    std::shared_ptr<ICoreDLAIntf> ICoreDLAIntf::Create(std::shared_ptr<Hapi::IHapi> spHapi, NetworkType network_type)
    {
        return std::make_shared<CoreDLAProcessor>(spHapi, network_type);
    }

    CoreDLAProcessor::CoreDLAProcessor(std::shared_ptr<Hapi::IHapi> spHapi, NetworkType network_type)
    : _spHapi(spHapi)
    , _network_handle(0)
    , _resultsCallback(nullptr)
    , _processing_time(std::chrono::milliseconds::zero())
    , _rendering_time(std::chrono::milliseconds::zero())
    , _count(0)
    , _diagnostics(false)
    {
        Hapi::CoreDLAPtr spCoreDLA = _spHapi->CreateByUniqueID<Hapi::CoreDLA>(40);
        spCoreDLA->InitializeInstance();

        // ARCH_HASH_SIZE bytes for the arch hash.
        std::vector<uint32_t> bitstream_arch_hash;
        for (size_t i = 0; i < ARCH_HASH_WORD_SIZE; ++i)
        {
            bitstream_arch_hash.push_back(intel_coredla_csr_read(spCoreDLA->GetInstance(), i));
        }
        _bitstream_arch_hash = arch_hash_to_string(bitstream_arch_hash);

        // Next BUILD_VERSION_SIZE bytes are for the build version string
        _bitstream_build_version =
            read_string_from_bitstream_rom(spCoreDLA, BUILD_VERSION_WORD_SIZE, BUILD_VERSION_CSR_OFFSET);

        // Next ARCH_NAME_SIZE bytes are for the arch name string
        _bitstream_arch_name =
            read_string_from_bitstream_rom(spCoreDLA, ARCH_NAME_WORD_SIZE, ARCH_NAME_CSR_OFFSET);
       
        std::string categoriesFile = _bin_directory + "/yolov8n_categories.txt";
        if(std::filesystem::exists(categoriesFile))
        {
            YoloClassificationItem::InitialiseCategoryNames(categoriesFile.c_str());
        }
        else
        {
            // Fallback to pose if detection cotegories not found
            categoriesFile = _bin_directory + "/yolov8n-pose_categories.txt";
            if(std::filesystem::exists(categoriesFile))
            {
                YoloClassificationItem::InitialiseCategoryNames(categoriesFile.c_str());
            }
        }
    }

    CoreDLAProcessor::~CoreDLAProcessor()
    {
        Stop();
    }

    void CoreDLAProcessor::LoadNetwork()
    {
        if (_open_vino_ie)
        {
            std::cout << "Networks already loaded" << std::endl;
        }
        else
        {
            char dla_disable_runtime_version_check[] = "DLA_DISABLE_RUNTIME_VERSION_CHECK=1";
            putenv(dla_disable_runtime_version_check);
            char dla_disable_version_check[] = "DLA_DISABLE_VERSION_CHECK=1";
            putenv(dla_disable_version_check);
            std::stringstream ss_coredla_root;
            ss_coredla_root << "COREDLA_ROOT=" << _bin_directory;
            std::string coredla_root = ss_coredla_root.str();
            putenv((char*)coredla_root.c_str());
            std::stringstream ss_ld_library_path;
            ss_ld_library_path << "LD_LIBRARY_PATH=" << _bin_directory;
            std::string ld_library_path = ss_ld_library_path.str();
            putenv((char*)ld_library_path.c_str());

            std::cout << "Loading networks ..." << std::endl;

            std::weak_ptr<IResultsCallback> wpIRresultsCallback = weak_from_this();
            _open_vino_ie = std::make_shared<OpenVinoIE>(0, FQ::CORE_DLA_BUFFERS, FQ::CORE_DLA_BYPASS_OUTPUT_LAYOUT_TRANSFORM, wpIRresultsCallback);

            std::vector<std::filesystem::path> network_filenames = GetNetworkFilenames();
            for(const std::filesystem::path& network_path: network_filenames)
            {
                if(!network_path.empty())
                {
                    NetworkType network_type;
                    uint32_t width;
                    uint32_t height;
                    parse_network_path(network_path, network_type, width, height);
                    if((network_type != NetworkType::UNKNOWN) && (width != 0U) && (height != 0U))
                    {
                        std::stringstream ss_network_name;
                        if(network_type == NetworkType::YOLOV8N)
                        {
                            ss_network_name << "YOLO 8n Detect";
                        }
                        else if(network_type == NetworkType::YOLOV8N_POSE)
                        {
                            ss_network_name << "YOLO 8n Pose";
                        }
                        ss_network_name << " " << width << "x" << height;

                        auto ocsUioHapiCore = std::dynamic_pointer_cast<Hapi::OcsUioHapiCore>(_spHapi);
                        if(ocsUioHapiCore)
                        {
                            // Unmap all UIO devices before loading the network to avoid gdb issue
                            // This is a workaround for an issue where loading shared objects, after mmapping uio devices, results in a segfault in gdbserver.
                            // On loading the shared objects, gdbserver tries to access an inaccessable memory address.
                            ocsUioHapiCore->UnmapAll();
                        }
                        ModelPtr network_model = _open_vino_ie->Import(network_path, network_type);
                        if(network_model)
                        {
                            network_model->_network_name = ss_network_name.str();
                            if(network_type == NetworkType::YOLOV8N)
                            {
                                std::shared_ptr<YoloDetectionProcessor> yolo_detection_results_processor = std::make_shared<YoloDetectionProcessor>(network_model, FQ::CORE_DLA_BYPASS_OUTPUT_LAYOUT_TRANSFORM);
                                _yolo_results_processors.emplace(network_model->_network_handle, yolo_detection_results_processor);
                            }
                            else
                            {
                                std::shared_ptr<YoloPoseProcessor> yolo_pose_results_processor = std::make_shared<YoloPoseProcessor>(network_model, FQ::CORE_DLA_BYPASS_OUTPUT_LAYOUT_TRANSFORM);
                                _yolo_results_processors.emplace(network_model->_network_handle, yolo_pose_results_processor);
                            }
                            _network_model_list.emplace_back(network_model);
                        }
                    }
                }
            }

            if(!_network_model_list.empty())
            {
                ModelPtr network_model = _network_model_list[0];
                if(network_model)
                {
                    _network_handle = network_model->_network_handle;
                    std::string network_name = network_model->_network_name;
                    _open_vino_ie->SetCurrentNetwork(network_model, std::move(network_name));
                }
            }
        }
    }

    void CoreDLAProcessor::Start()
    {
        if(_open_vino_ie)
            _open_vino_ie->Start(true);
    }

    void CoreDLAProcessor::Stop()
    {
        if(_open_vino_ie)
            _open_vino_ie->Start(false);
    }    

    void CoreDLAProcessor::SetNetworkHandle(uint32_t network_handle)
    {
        if(network_handle != _network_handle)
        {
            for(auto network_model: _network_model_list)
            {
                if(network_model)
                {
                    if(network_model->_network_handle == network_handle)
                    {
                        _network_handle = network_handle;
                        std::string network_name = network_model->_network_name;
                        std::cout << "Switching AI model to : " << network_name << std::endl << std::flush;
                        _open_vino_ie->SetCurrentNetwork(network_model, std::move(network_name));
                    }
                }

            }
        }
    }

    const std::vector<ModelPtr>& CoreDLAProcessor::GetNetworkList()
    {
        return _network_model_list;
    }

    void CoreDLAProcessor::SetDetectionThreshold(float detectionThreshold)
    {
        for(const auto& yolo_results_processor_it : _yolo_results_processors)
        {
            yolo_results_processor_it.second->SetDetectionThreshold(detectionThreshold);
        }
    }

    void CoreDLAProcessor::SetIOUThreshold(float iouThreshold)
    {
        for(const auto& yolo_results_processor_it : _yolo_results_processors)
        {
            yolo_results_processor_it.second->SetIOUThreshold(iouThreshold);
        }
    }

    void CoreDLAProcessor::SetDiagnostics(bool diagnostics)
    {
        _diagnostics = diagnostics;
        _open_vino_ie->SetDiagnostics(_diagnostics);
    }

    void CoreDLAProcessor::RegisterResultsCallback(ICoreDLAResultsCallback resultsCallback)
    {
        _resultsCallback = std::move(resultsCallback);
    }

    void CoreDLAProcessor::ReceiveResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput)
    {
        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        std::shared_ptr<YoloClassificationResult> result;
        auto it = _yolo_results_processors.find(spCoreDLAOutput->network_handle);
        if(it != _yolo_results_processors.end())
        {
            result = it->second->ProcessResults(spCoreDLAOutput);
        }
        std::chrono::high_resolution_clock::time_point mid = std::chrono::high_resolution_clock::now();
        if(_resultsCallback)
        {
            _resultsCallback(ResultsType::RESULTS, std::move(result));
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        _processing_time += std::chrono::duration_cast<std::chrono::milliseconds>(mid - start);
        _rendering_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - mid);
        _count++;
        if(_count == 100)
        {
            if(_diagnostics)
            {
                std::cout << "_processing_time = " << _processing_time.count() << " ms" << std::endl;
                std::cout << "_rendering_time = " << _rendering_time.count() << " ms" << std::endl;
            }
            _processing_time = std::chrono::milliseconds::zero();
            _rendering_time = std::chrono::milliseconds::zero();
            _count = 0;
        }
    }

    void CoreDLAProcessor::NoNetworkFound()
    {
        if(_resultsCallback)
        {
            _resultsCallback(ResultsType::NO_NETWORK, nullptr);
        }
    }

    void CoreDLAProcessor::OutOfInferences()
    {
        if(_resultsCallback)
        {
            _resultsCallback(ResultsType::OUT_OF_INFERENCES, nullptr);
        }
    }

    bool CoreDLAProcessor::SendUserMessage(UserMessageType messageType, void* pPayload, uint32_t size, UserPayload* pUserPayloadResults)
    {
        bool rc = false;
        if(_open_vino_ie != nullptr)
        {
            rc = _open_vino_ie->SendUserMessage(messageType, pPayload, size, pUserPayloadResults);
        }
        return rc;
    }

    std::string CoreDLAProcessor::GetStatus()
    {
        std::string rc;
        if(_open_vino_ie != nullptr)
        {
            rc = _open_vino_ie->GetStatus();
        }
        return rc;
    }

    void CoreDLAProcessor::StatusMessage(const char* message)
    {
        std::cout << "Application::StatusMessage " << message << std::endl;
    }

    std::vector<std::filesystem::path> CoreDLAProcessor::GetNetworkFilenames()
    {
        std::vector<std::filesystem::path> networkFilenames;
        std::filesystem::path network_dir = _bin_directory + std::string("/") + _bitstream_arch_name + std::string(".arch");
        if(std::filesystem::exists(network_dir))
        {
            for (auto const& dir_entry : std::filesystem::directory_iterator{network_dir}) 
            {
                if(dir_entry.is_regular_file())
                {
                    auto network_path = dir_entry.path();
                    if(network_path.extension() == ".bin")
                    {
                        networkFilenames.emplace_back(network_path);
                    }
                }
            }

            std::sort (networkFilenames.begin(), networkFilenames.end(), sort_network_path);
        }
        
        if(networkFilenames.empty())
        {
            if(_open_vino_ie)
            {
                _open_vino_ie->SetNoNetwoksFound();
            }
        }

        return networkFilenames;
    }
}