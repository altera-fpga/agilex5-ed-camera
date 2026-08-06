/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpAdapter.h"
#include "WarpDataHelper.h"
#include "intel_vvp_warp.h"
#include "WarpAdapterRawRgbImage.h"
#include "SwUtils.h"
#include "OWarpFile.h"

#include "intel_vvp_warp_regs.h"

#include <cinttypes>
#include <filesystem>

#ifdef __linux__
#include <unistd.h>
#include <sys/mman.h>
#endif

#define VERBOSE_PRINTF(...)

namespace SwApi
{
    IMemTransferPtr CreateDataTransfer();

    // Total coefficient memory size required for 8K warp (4x engines)
    static const uint32_t WARP_DATA_PAGE_SIZE = 0x1800000;

    // Factory create with a Hapi::VvpWarpPtr
    std::shared_ptr<WarpAdapter> WarpAdapter::Create(Hapi::VvpWarpPtr spVvpWarp,
                const uint32_t num_engines,
                const std::size_t dma_bus_offset)
    {
        if (!spVvpWarp)
            return nullptr;

        auto dataTransfer = CreateDataTransfer();

        if(!dataTransfer)
            return nullptr;
            
        intel_vvp_warp_instance_t* pInstance = spVvpWarp->GetInstance();
        intel_vvp_warp_channel_t* pChannel{nullptr};

        static constexpr uint32_t input = 0;
        static constexpr uint32_t engine0 = 0;
        static constexpr uint32_t engine1 = 1;
        static constexpr uint32_t output = 0;

        const uint32_t engines = std::min(num_engines, pInstance->num_engines);
        
        switch(engines)
        {
            case 4:
                pChannel = intel_vvp_warp_create_quad_channel(pInstance, input, output);
                break;
            case 2:
                pChannel = intel_vvp_warp_create_double_channel(pInstance, input, engine0, engine1, output);
                break;
            case 1:
            default:
                pChannel = intel_vvp_warp_create_channel(pInstance, input, engine0, output);
                break;
        };

        // If all above fails try creating an easy warp channel
        if(!pChannel)
            pChannel = intel_vvp_warp_create_easy_warp_channel(pInstance, input, output);

        // Create Warp adapter object
        if (pChannel)
        {
            if(pChannel->num_engines)
                std::cout << "Created warp channel with " << pChannel->num_engines << " engine(s)\n";
            else
                std::cout << "Created easy warp channel\n";

            WarpChannelPtr spWarpChannel(pChannel, &intel_vvp_warp_free_channel);
            auto spWarpAdapter = std::make_shared<WarpAdapter>(std::move(spWarpChannel), dataTransfer, WARP_BASE_FPGA, dma_bus_offset);

            if (spWarpAdapter)
                spWarpAdapter->SetHapiItem(spVvpWarp);

            return spWarpAdapter;
        }
        else
            std::cerr << "Unable to create a warp channel" << std::endl;

        return nullptr;
    }


    WarpAdapter::WarpAdapter(WarpChannelPtr spChannel, 
                                IMemTransferPtr spDataTransfer,
                                uintptr_t warp_ram_base,
                                std::size_t dma_bus_offset)
    :	_ch{spChannel.get()}
    ,   _spChannel{std::move(spChannel)}
    ,   _configurator{intel_vvp_warp::WarpDataHelper::GetHwContext(_ch)}
    ,   _framebuffer_ram_base{warp_ram_base}
    ,	_coef_ram_base{warp_ram_base + intel_vvp_warp_get_channel_framebuffer_size(_ch)}
    ,	_bypass{false}
    ,   _lowFrameRateFallback{true}
    ,	_page{0}
    ,   _skip_ram_page{0}
    ,   _skip_ram_page_bypass_off{0}
    ,   _spDataTransfer{std::move(spDataTransfer)}
    ,   _updatePending{false}
    ,   _isEasyWarp{false}
    ,  _dma_bus_offset{dma_bus_offset}
    {
        _configurator.Reset();
        _updateSucceeded.Set();
        _isEasyWarp = (::intel_vvp_warp_check_easy_warp_capable(_ch) == 0);
        _ch->width_output = 0;
        _ch->height_output = 0;
        _spMessageQueue = std::make_shared<SwUtils::MessageQueue>("WarpMessageQ");
    }

    WarpAdapter::~WarpAdapter()
    {
    }

    bool WarpAdapter::TransferEngineData(uintptr_t* dst, void** src, std::size_t* sz)
    {
        bool ret = true;

        std::scoped_lock lock(_txMutex);
        ret = ret && _spDataTransfer->TransferToTarget(_dma_bus_offset + dst[0], src[0], sz[0]);
        ret = ret && _spDataTransfer->TransferToTarget(_dma_bus_offset + dst[1], src[1], sz[1]);
        ret = ret && _spDataTransfer->TransferToTarget(_dma_bus_offset + dst[2], src[2], sz[2]);
        return ret;
    }

    void WarpAdapter::SetHapiItem(Hapi::VvpWarpPtr spVvpWarp)
    {
        _spVvpWarp = spVvpWarp;
    }

    bool WarpAdapter::IsEasyWarp()
    {
        return _isEasyWarp;
    }

    bool WarpAdapter::IsSingleBounce()
    {
        // Output block scan indicates there is an output bounce (either block or raster)
        return (_ch && _ch->output_block_scan == false) ? true : false;
    }

    bool WarpAdapter::IsMipmapEnabled() const
    {
        return (GetWarpInstance()->mipmap_enable == 1);
    }

    bool WarpAdapter::SetEasyWarpMirror(bool mirror)
    {
        if (!_isEasyWarp)
            return false;

        // The driver doesn't keep the state of the easy warp
        _easyWarpMirror = mirror;
        return ProgramEasyWarp();
    }

    bool WarpAdapter::SetEasyWarpRotation(EasyWarpRotation rotation)
    {
        if (!_isEasyWarp)
            return false;

        // The driver doesn't keep the state of the easy warp
        _easyWarpRotation = rotation;
        return ProgramEasyWarp();
    }

    bool WarpAdapter::ProgramEasyWarp()
    {
        uint32_t easyWarpReg = 0;
        auto [outputWidth, outputHeight] = GetOutputResolution();

        if (_easyWarpRotation == EasyWarpRotation::None)
            easyWarpReg = 0;
        else if (_easyWarpRotation == EasyWarpRotation::R90)
            easyWarpReg = 3;
        else if (_easyWarpRotation == EasyWarpRotation::R180)
            easyWarpReg = 2;
        else if (_easyWarpRotation == EasyWarpRotation::R270)
            easyWarpReg = 1;

        if (_easyWarpMirror)
            easyWarpReg |= 0x4;

        SetBypass(true, outputWidth, outputHeight);

        return (::intel_vvp_warp_set_easy_warp(_ch, easyWarpReg) == 0);
    }

    void WarpAdapter::SetInputResolution(uint32_t width, uint32_t height)
    {
        auto configurator = GetConfiguratorSafe();
        configurator->SetInputResolution(width, height);
        ConfigureChannel(configurator->GetInputResolution(), configurator->GetOutputResolution());
    }

    void WarpAdapter::SetOutputResolution(uint32_t width, uint32_t height)
    {
        auto configurator = GetConfiguratorSafe();
        configurator->SetOutputResolution(width, height);
        ConfigureChannel(configurator->GetInputResolution(), configurator->GetOutputResolution());
    }

    std::pair<uint32_t, uint32_t> WarpAdapter::GetOutputResolution()
    {
        auto configurator = GetConfiguratorSafe();
        return configurator->GetOutputResolution();
    }

    void WarpAdapter::SetVideoResolution(uint32_t input_width, uint32_t input_height, 
                                        uint32_t output_width, uint32_t output_height,
                                        uint32_t outputFullHeight, uint32_t frameRateX100)
    {
        auto configurator = GetConfiguratorSafe();
        configurator->SetInputResolution(input_width, input_height);
        configurator->SetOutputResolution(output_width, output_height);
        _outputFullHeight = outputFullHeight;
        _frameRateX100 = frameRateX100;
        ConfigureChannel(configurator->GetInputResolution(), configurator->GetOutputResolution());
    }

    uint32_t WarpAdapter::GetPage()
    {
        return _page;
    }

    void WarpAdapter::SetPage(const uint32_t page)
    {
        _page = page % _COEF_PAGES_MAX;            
    }

    uint32_t WarpAdapter::GetSkipRamPage()
    {
        return _skip_ram_page;
    }

    void WarpAdapter::SetSkipRamPage(const uint32_t page)
    {
        _skip_ram_page = page % _SKIP_RAM_PAGES_HW_TOTAL;
        VERBOSE_PRINTF("\nSkip RAM page: %d!\n", _skip_ram_page);
    }        

    void WarpAdapter::SetBypass(bool bypass, uint32_t overrideWidth, uint32_t overrideHeight)
    {
        uint32_t skip_ram_page = 0;

        if(bypass)
        {
            skip_ram_page = (GetSkipRamPage() + 1) % _SKIP_RAM_PAGES_HW_TOTAL;
            intel_vvp_warp_reset_skip_ram(_ch, skip_ram_page);
            intel_vvp_warp_reset_input_skip_ram(_ch, skip_ram_page);
        }
        else
        {
            skip_ram_page = _skip_ram_page_bypass_off;
        }

        int ret = intel_vvp_warp_bypass(_ch, (bypass ? 1 : 0), skip_ram_page, overrideWidth, overrideHeight);
        
        if(ret == 0)
        {
            if(bypass) _skip_ram_page_bypass_off = GetSkipRamPage();

            SetSkipRamPage(skip_ram_page);
            
            _bypass = bypass;
            VERBOSE_PRINTF("\nWarp bypass: %s\n", bypass ? "Enabled":"Disabled");
        }
        else
            VERBOSE_PRINTF("\nError setting warp bypass!\n");

        if (!_bypass && _updatePending)
            ScheduleUpdate(true); // Wait to avoid skip RAM page override
    }

    bool WarpAdapter::ConfigureChannel(const warp_res_t& ires, const warp_res_t& ores)
    {
        bool ret = false;

        intel_vvp_warp_channel_config_t cfg;
        
        cfg.ram_addr = _framebuffer_ram_base;
        cfg.cs = ERGB_FULL;
        cfg.width_input = ires.first;
        cfg.height_input = ires.second;
        cfg.width_output = ores.first;
        cfg.height_output = ores.second;
        cfg.lfr = (IsSingleBounce() ? 0 : GetLowFramerateFallback() ? 1 : 0);

        int rc = intel_vvp_warp_configure_channel(_ch, &cfg);
        ret = (rc == 0);
        
        if(!ret)
            VERBOSE_PRINTF("Error configuring warp channel %d, Code: %d\n", _ch->idx, rc);
        
        return ret;
    }

    bool WarpAdapter::IsChannelConfigured() const
    {
        return (_ch->width_output != 0 && _ch->height_output != 0);
    }
        
    void WarpAdapter::Update(bool waitForCompletion)
    {
        ScheduleUpdate(waitForCompletion);
    }

    void WarpAdapter::UpdateFixed(bool waitForCompletion)
    {
        _fixedWarp = true;
        using namespace intel_vvp_warp;
        SetMeshFunction([](WarpConfigurator& cfg)->WarpMeshPtr { return cfg.GenerateMeshFromFixed();});
        ScheduleUpdate(waitForCompletion);
        _mode = WarpAdapterMode::Fixed;
    }

    void WarpAdapter::UpdateCorners(bool waitForCompletion)
    {
        _fixedWarp = false;
        using namespace intel_vvp_warp;
        SetMeshFunction([](WarpConfigurator& cfg)->WarpMeshPtr { return cfg.GenerateMeshFromCorners();});
        ScheduleUpdate(waitForCompletion);            
        _mode = WarpAdapterMode::Corners;
    }

    void WarpAdapter::UpdateArbitrary(bool waitForCompletion)
    {
        _fixedWarp = false;
        using namespace intel_vvp_warp;
        SetMeshFunction([](WarpConfigurator& cfg)->WarpMeshPtr { return cfg.GenerateMeshFromArbitrary();});
        ScheduleUpdate(waitForCompletion);           
        _mode = WarpAdapterMode::Arbitrary;
    }

    void WarpAdapter::UpdateFisheye(bool waitForCompletion)
    {
        _fixedWarp = false;
        using namespace intel_vvp_warp;
        SetMeshFunction([](WarpConfigurator& cfg)->WarpMeshPtr { return cfg.GenerateMeshFromFisheye();});
        ScheduleUpdate(waitForCompletion);
        _mode = WarpAdapterMode::Fisheye;
    }    

    WarpAdapter::WarpAdapterMode WarpAdapter::GetMode()
    {
        return _mode;
    }

    void WarpAdapter::PrintDebugRegisters()
    {
        intel_vvp_warp_instance_t* instance = _ch->instance;

        uint32_t reg_val = 0x0;

        auto print_reg_header = [instance](const char* reg_name, const uint32_t reg_idx)->uint32_t {
            uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(reg_idx);
            printf("0x%08x ", (reg_idx) << 2);
            printf("%-32s: 0x%08x %" PRIu32 "", reg_name, reg_val, reg_val);
            return reg_val;
        };

        printf("\n--- Warp status and debug registers ---\n\n");

        reg_val = print_reg_header("Input Config", INTEL_VVP_WARP_INPUT_BASE(0) + INTEL_VVP_WARP_INPUT_CONFIG);
        printf("\tPPC: %d\n", (reg_val >> 16) & 0x7);

        reg_val = print_reg_header("Input Status", INTEL_VVP_WARP_INPUT_BASE(0) + INTEL_VVP_WARP_INPUT_STATUS);
        printf("\t%dx%d\n", reg_val & 0x1fff, (reg_val >> 16) & 0x1fff);

        reg_val = print_reg_header("Input Resolution", INTEL_VVP_WARP_INPUT_BASE(0) + INTEL_VVP_WARP_INPUT_RESOLUTION);
        printf("\t%dx%d\n", reg_val & 0x1fff, (reg_val >> 16) & 0x1fff);

        // Debug counters
        printf("Warp debug\n");
        reg_val = print_reg_header("Warp Debug Mem Wr Queues0", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_WR_QUEUES0);
        printf("\n");

        reg_val = print_reg_header("Warp Debug Mem Wr Queues1", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_WR_QUEUES1);
        printf("\n");

        reg_val = print_reg_header("Warp Debug Mem Rd Queues0", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_RD_QUEUES0);
        printf("\n");

        reg_val = print_reg_header("Warp Debug Mem Rd Queues1", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_RD_QUEUES1);
        printf("\n");        

        reg_val = print_reg_header("Warp Debug Mem Rd Cycles", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_RD_CYCLES);
        printf("\n");

        reg_val = print_reg_header("Warp Debug Mem Wr Cycles", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_WR_CYCLES);
        printf("\n");

        reg_val = print_reg_header("Warp Debug Mem Wr Cycles", INTEL_VVP_WARP_DEBUG_BASE + INTEL_VVP_WARP_DEBUG_MEM_STALL_CYCLES);
        printf("\n");

        // Input
        printf("Warp input debug\n");
        reg_val = print_reg_header("Input Debug Frame Count", INTEL_VVP_WARP_DEBUG_INPUT_BASE(0) + INTEL_VVP_WARP_DEBUG_INPUT_FRAME_COUNT);
        printf("\n");

        reg_val = print_reg_header("Input Debug Frame Period", INTEL_VVP_WARP_DEBUG_INPUT_BASE(0) + INTEL_VVP_WARP_DEBUG_INPUT_FRAME_PERIOD);
        printf("\n");

        reg_val = print_reg_header("Input Debug Mem Writes", INTEL_VVP_WARP_DEBUG_INPUT_BASE(0) + INTEL_VVP_WARP_DEBUG_INPUT_MEM_WRITES);
        printf("\n");

        reg_val = print_reg_header("Input Debug Mipmap Mem Writes", INTEL_VVP_WARP_DEBUG_INPUT_BASE(0) + INTEL_VVP_WARP_DEBUG_INPUT_MIPMAP_MEM_WRITES);
        printf("\n");

        // Output
        printf("Warp output debug\n");
        reg_val = print_reg_header("Output Debug Frame Count", INTEL_VVP_WARP_DEBUG_OUTPUT_BASE(0) + INTEL_VVP_WARP_DEBUG_OUTPUT_FRAME_COUNT);
        printf("\n");

        reg_val = print_reg_header("Output Debug Frame Period", INTEL_VVP_WARP_DEBUG_OUTPUT_BASE(0) + INTEL_VVP_WARP_DEBUG_OUTPUT_FRAME_PERIOD);
        printf("\n");

        uint64_t cache_loads_total = 0;

        for(uint32_t e = 0; e < _ch->num_engines; ++e)
        {
            const uint32_t idx = _ch->engines[e]->idx;
            printf("Engine %d\n", idx);

            reg_val = print_reg_header("Engine Debug Frame Count", INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_FRAME_COUNT);
            printf("\n");

            reg_val = print_reg_header("Engine Debug Frame Period", INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_FRAME_PERIOD);
            printf("\n");

            reg_val = print_reg_header("Engine Debug Busy Cycles", INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_BUSY_CYCLES);
            printf("\n");

            reg_val = print_reg_header("Engine Debug Frame Delay", INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_FRAME_DELAY);
            printf("\n");

            reg_val = print_reg_header("Engine Debug Run Status", INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_RUN_STATUS);
            printf("\n");

            reg_val = print_reg_header("Engine Debug Cache Loads", INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_CACHE_LOADS);
            printf("\n");

            cache_loads_total += reg_val;
        }

        printf("\nCache Loads Total: %" PRIu64 "\n", cache_loads_total);

        printf("\n---------------------------------------\n");
        fflush(stdout);
    }


    void WarpAdapter::SetMeshFunction(TMeshFunction meshFunction)
    {
        std::lock_guard lock(_paramsCs);
        _meshFunction = std::move(meshFunction);
    }

    void WarpAdapter::ScheduleUpdate(bool waitForCompletion)
    {
        if (!_bypass)
        {
            if(IsChannelConfigured())
            {
                auto doUpdateCB = [this]() { DoUpdate(); };

                _spMessageQueue->FlushMessages();

                if (waitForCompletion)
                    _spMessageQueue->AddAndWait(new SwUtils::SingleCallbackCmd(doUpdateCB));
                else
                    _spMessageQueue->Add(new SwUtils::SingleCallbackCmd(doUpdateCB));

                _updatePending = false;
            }
        }
        else
            _updatePending = true;
    }        

    // Called on message queue thread
    void WarpAdapter::DoUpdate()
    {
        std::unique_lock lock(_paramsCs);
        auto upConfigurator = std::make_unique<intel_vvp_warp::WarpConfigurator>(_configurator);
        auto meshFunction = _meshFunction;
        lock.unlock();

        bool success = false;

        if(meshFunction)
        {
            intel_vvp_warp::WarpHwContextPtr hw = intel_vvp_warp::WarpDataHelper::GetHwContext(_ch);
            const auto [inputWidth, inputHeight] = upConfigurator->GetInputResolution();
            const auto [outputWidth, outputHeight] = upConfigurator->GetOutputResolution();

            // Input and output dimensions the same
            intel_vvp_warp::WarpDataContext context
            {
                hw,
                inputWidth, inputHeight,
                outputWidth, outputHeight
            };

            auto mesh = meshFunction(*upConfigurator);
            auto spUserData = _data_generator.GenerateData(context, mesh);              
            
            if(spUserData)
            {
                ApplyTransform(context, spUserData);

                uint32_t systemClock = 300000000;
                uint32_t videoClock = 300000000;

                {
                    std::lock_guard lock(_paramsCs);
                    if ((_outputFullHeight != 0) && (_frameRateX100 != 0))
                    {
                        _latencyParams = _data_generator.GenerateLatencyParams(context, spUserData, systemClock, 
                                                                                videoClock, _outputFullHeight, _frameRateX100);
                    }
                }

                if(_data_generator.GetLastErrorCode() == intel_vvp_warp::WarpDataGenerator::Success)
                    success = true;
            }
            else
            {
                std::lock_guard lock(_paramsCs);
                _latencyParams = {};
                success = false;
            }
        }

        if (success)
            _updateSucceeded.Set();
        else
        {
            _updateSucceeded.Reset();
            std::cerr << "Error generating warp data: " << _data_generator.GetErrorString(_data_generator.GetLastErrorCode()) << std::endl;
        }
    }

    bool WarpAdapter::ApplyTransform(const intel_vvp_warp::WarpDataContext& ctx, 
                                                    intel_vvp_warp::WarpDataPtr user_data)
    {
        bool ret = false;

        const uint32_t engines = user_data->GetEngines();
        
        if(engines <= _ch->num_engines)
        {
            const uint32_t next_page = (GetPage() + 1) % _COEF_PAGES_MAX;	
            const uint32_t coef_base_address = _coef_ram_base + (next_page * WARP_DATA_PAGE_SIZE);

            const uint32_t skip_ram_page = (GetSkipRamPage() + 1) % _SKIP_RAM_PAGES_HW_TOTAL;

            auto driver_data = intel_vvp_warp::WarpDataHelper::GenerateDriverData(ctx, user_data, coef_base_address, skip_ram_page);

            if(driver_data)
            {
                intel_vvp_warp_engine_data_t* engine_data = driver_data->engine_data;

                for(uint32_t i = 0; i < driver_data->num_engines; ++i)
                {
                    // Transfer generated warp data to the calculated destination 
                    auto ued = user_data->GetEngineData(i);
                    uintptr_t dst[] = {engine_data[i].mesh_addr, engine_data[i].filter_addr, engine_data[i].fetch_addr};
                    void* src[] = {(void*)ued->GetMeshData(), (void*)ued->GetFilterData(), (void*)ued->GetFetchData()};
                    std::size_t sz[] = {engine_data[i].mesh_size, engine_data[i].filter_size, engine_data[i].fetch_size};

                    TransferEngineData(dst, src, sz);
                }

                VERBOSE_PRINTF("Warp data uploaded\n");

                SetPage(next_page);
                SetSkipRamPage(skip_ram_page);

                int rc = intel_vvp_warp_apply_transform(_ch, driver_data.get());
                ret = (rc == 0);                    
            }               
        }
        else
            VERBOSE_PRINTF("Not enough warp engines! Available: %" PRIu32 " required: %" PRIu32 " \n", _ch->num_engines, engines);
        
        return ret;
    }

    // Capture 8 bit picture
    std::shared_ptr<RawRgbImage> WarpAdapter::CapturePicture()
    {
        auto spPlanes = CapturePicturePlanes();
        if (spPlanes)
            return spPlanes->GetInterleavedImage();
        else
            return nullptr;
    }

    // Capture 10 bit picture, 10 bits in 16
    std::shared_ptr<RawRgbImage16> WarpAdapter::CapturePicture10()
    {
        auto spPlanes = CapturePicturePlanes();
        if (spPlanes)
            return spPlanes->GetInterleavedImage16();
        else
            return nullptr;
    }

    std::shared_ptr<PicturePlanes> WarpAdapter::CapturePicturePlanes()
    {
        const auto [inputWidth, inputHeight] = GetConfiguratorSafe()->GetInputResolution();

        const auto pInstance = GetWarpInstance();

        if (!pInstance || (inputWidth == 0))
            return nullptr;
            
        intel_vvp_warp_channel_t* pChannel = _ch;

        // Enable capture in the driver
        intel_vvp_warp_enable_capture(pInstance, true);

        
        int capture_width = inputWidth;// * 2;
        int capture_height = inputHeight;// * 2;

        std::vector<uint32_t> destination_data(capture_width * capture_height);
        int vblocks = capture_height / 8;
        int hblocks = capture_width / 16;

        const uint32_t tile_step = 16 * 8 * static_cast<uint32_t>(sizeof(uint32_t));
        const uint32_t tileStride = 256;
        const uint32_t data_size = vblocks*tileStride*tile_step;

        // MSGDMA needs 4096 alligned buffer
        uint32_t* raw_data_buffer = (uint32_t*)std::aligned_alloc(0x1000, data_size);

        uint32_t buffer_index = intel_vvp_warp_get_locked_buffer_index(pChannel);
        uint64_t capture_address = intel_vvp_warp_get_capture_address(pChannel, buffer_index) + _framebuffer_ram_base;

        uint64_t max_transfer = 0x80000;
        uint64_t remaining_data = data_size;
        uint8_t* receive_buffer = (uint8_t*)raw_data_buffer;
        while (remaining_data > 0)
        {
            uint64_t transfer_size = std::min(max_transfer, remaining_data);
            if(!_spDataTransfer->TransferFromTarget(receive_buffer, capture_address, transfer_size))
                return nullptr;

            remaining_data -= transfer_size;
            receive_buffer += transfer_size;
            capture_address += transfer_size;
        }

        // Capture has finished, turn off in the driver
        intel_vvp_warp_enable_capture(pInstance, false);

        uintptr_t p_data = (uintptr_t)raw_data_buffer;
        uintptr_t p_data_end = (uintptr_t)(raw_data_buffer + data_size / sizeof(uint32_t) - 1);

        // We are only interested in the top left quarter
        //vblocks /= 2;
        //hblocks /= 2;

        for (int v = 0; v < vblocks; ++v)
        {
            int tile_line = v * 8;

            for (int h = 0; h < hblocks; ++h)
            {
                int tile_pixel = h * 16;

                uint32_t tile_idx = v * tileStride + h;
                uintptr_t byte_address = tile_idx * (16 * 8 * 4);

                /*uintptr_t remap_address = (byte_address & ~((uintptr_t)0x1fffff));
                remap_address += (byte_address & 0x0001ff) << 0;
                remap_address += (byte_address & 0x000600) << 4;
                remap_address += (byte_address & 0x007800) >> 2;
                remap_address += (byte_address & 0x018000) << 1;
                remap_address += (byte_address & 0x020000) >> 2;
                remap_address += (byte_address & 0x1c0000) << 0;*/
                uintptr_t remap_address = byte_address;

                uint32_t* p_pixel = (uint32_t*)(p_data + remap_address);
                if(p_pixel > (uint32_t*)(p_data_end))
                {
                    std::cerr << "Error" << std::endl;
                }

                for (int burst = 0; burst < 8; ++burst)
                {
                    int tile_v = tile_line + 4 * (burst / 4);
                    int tile_h = tile_pixel + 4 * (burst % 4);

                    int data_reminder = 32;		// how many bits left in current dword

                    for (int sub_pixel = 0; sub_pixel < 16; ++sub_pixel)
                    {
                        uint32_t pixel = 0x0;
                        int pixel_reminder = 30;	// how many bits to extract

                        while (pixel_reminder)
                        {
                            if (data_reminder >= pixel_reminder)
                            {
                                uint32_t p = (*p_pixel >> (32 - data_reminder));
                                pixel |= (p << (30 - pixel_reminder));
                                data_reminder -= pixel_reminder;
                                pixel_reminder = 0;
                            }
                            else
                            {
                                uint32_t p = (*p_pixel >> (32 - data_reminder));
                                pixel |= (p << (30 - pixel_reminder));
                                pixel_reminder -= data_reminder;
                                data_reminder = 0;
                            }

                            if (0 == data_reminder)
                            {
                                ++p_pixel;
                                if(p_pixel > (uint32_t*)(p_data_end))
                                {
                                    std::cerr << "Error" << std::endl;
                                }
                                data_reminder = 32;
                            }
                        }

                        // The algorithm above sometimes lets
                        // two top bits have unwanted data
                        // clear it here
                        pixel &= 0x3fffffff;

                        int sub_v = sub_pixel / 4;
                        int sub_h = sub_pixel % 4;

                        int data_idx = ((tile_v + sub_v) * capture_width) + (tile_h + sub_h);
                        destination_data[data_idx] = pixel;
                    }

                    // There are 32 not used bits at the end of each burst
                    ++p_pixel;
                }
            }
        }

        std::free(raw_data_buffer);

        uint32_t* big_picture = &destination_data[0];

        auto red_data = std::make_shared<std::vector<uint16_t>>(inputWidth * inputHeight);
        auto green_data = std::make_shared<std::vector<uint16_t>>(inputWidth * inputHeight);
        auto blue_data = std::make_shared<std::vector<uint16_t>>(inputWidth * inputHeight);

        // Write out top left
        uint16_t* r_ptr = blue_data->data();
        uint16_t* g_ptr = green_data->data();
        uint16_t* b_ptr = red_data->data();

        for (uint32_t y = 0; y < inputHeight; y++)
        {
            uint32_t* source_line = big_picture + y * capture_width;

            for (uint32_t x = 0; x < inputWidth; x++)
            {
                uint32_t pixel_data = source_line[x];

                uint16_t value = (uint16_t)((pixel_data >> 20) & 0x3ff);
                *b_ptr++ = value;

                value = (uint16_t)((pixel_data >> 10) & 0x3ff);
                *g_ptr++ = value;

                value = (uint16_t)((pixel_data >> 0) & 0x3ff);
                *r_ptr++ = value;
            }
        }

        auto spPicturePlanes = std::make_shared<PicturePlanes>(inputWidth, inputHeight, red_data, green_data, blue_data);
        return spPicturePlanes;
    }

    bool WarpAdapter::ExportMesh(std::filesystem::path filePath, bool asJSON)
    {
        const auto configurator = GetConfiguratorSafe();

        uint32_t nKnots = configurator->GetArbitraryKnotsNum();

        uint32_t totalKnots = nKnots * nKnots;

        if (asJSON)
        {
            auto jsonParser = SwUtils::IJson::Create();
            auto spObject = jsonParser->RootObject();
            spObject->AddValue("rows", (int32_t)nKnots);
            spObject->AddValue("columns", (int32_t)nKnots);
            auto pointsArray = spObject->AddArray("points");

            for(uint32_t i = 0; i < totalKnots; ++i)
            {
                auto pointElement = pointsArray->AddElement();
                auto pointObject = pointElement->AddObject();
                const auto [x, y] = configurator->GetArbitraryKnotNorm(i);
                pointObject->AddValue("i", (int32_t)i);
                pointObject->AddValue("x", (double)x);
                pointObject->AddValue("y", (double)y);
            }

            return jsonParser->Save(filePath);
        }
        else
        {
            // Save as binary OWF
            auto spPointsArray = std::make_shared<PointsArray>();
            OWarpFile owf(OwfMeshType::FORWARD,
                            OwfResolution::INDEPENDENT,
                            16, 9,
                            nKnots, nKnots,
                            32,
                            24,
                            spPointsArray);

            for(uint32_t i = 0; i < totalKnots; ++i)
            {
                const auto [x, y ] = configurator->GetArbitraryKnotNorm(i);
                uint32_t uintX = owf.ToUint32(x);
                uint32_t uintY = owf.ToUint32(y);
                spPointsArray->push_back(uintX);
                spPointsArray->push_back(uintY);
            }

            return owf.Save(filePath);
        }
    }

    bool WarpAdapter::ImportMesh(std::filesystem::path filePath,
                                                std::vector<std::pair<float, float>>& warpPoints,
                                                uint32_t& nRows,
                                                uint32_t& nColumns)
    {
        std::string fileExtension = filePath.extension();
        SwUtils::MakeLower(fileExtension);
        static const char* mesh_dim_error_msg = "Arbitrary mesh: row and column numbers must be the same!";

        if (fileExtension == ".json")
        {
            nRows = 0;
            nColumns = 0;

            auto jsonParser = SwUtils::IJson::Create(filePath);
            if (!jsonParser)
                return false;

            auto spObject = jsonParser->Parse();
            if (!spObject)
                return false;

            try
            {
                nRows = (uint32_t)spObject->GetValue<int32_t>("rows");
                nColumns = (uint32_t)spObject->GetValue<int32_t>("columns");

                if (nRows != nColumns)
                    throw std::runtime_error(mesh_dim_error_msg);
                
                auto pointsArray = spObject->GetArray("points");
                
                if (!pointsArray)
                    return false;
                    
                size_t nPoints = pointsArray->Size();

                if (nPoints != (size_t)(nRows * nColumns))
                    return false;

                warpPoints.resize(nPoints);

                auto configurator = GetConfiguratorSafe();

                configurator->SetArbitraryKnotsNum(nRows);    // row and column dimensions are the same

                for (size_t i = 0; i < nPoints; i++)
                {
                    auto element = pointsArray->At(i);
                    auto elementObject = element->GetObject();
                    if (!elementObject)
                        return false;

                    float x = static_cast<float>(elementObject->GetValue<double>("x"));
                    float y = static_cast<float>(elementObject->GetValue<double>("y"));


                    warpPoints[i].first = x;
                    warpPoints[i].second = y;
                    configurator->SetArbitraryKnotNorm(i, x, y);          
                }

                UpdateArbitrary(false);
            }
            catch (std::string errorString)
            {
                std::cout << "ImportMesh failed, reason: " << errorString << std::endl;
                return false;
            }
        }
        else if (fileExtension == ".owf")
        {
            OWarpFile owf;
            if (!owf.Load(filePath))
                return false;

            if (owf.GetSampleSize() != 32)
                return false;

            auto spPointsArray = owf.GetPointsArray();

            nRows = owf.GetNumRows();
            nColumns = owf.GetNumColumns();

            if (nRows != nColumns)
            {
                std::cout << mesh_dim_error_msg << std::endl;
                return false;
            }
            size_t nPoints = nRows * nColumns;

            if (spPointsArray->size() != (nPoints * 2))
                return false;

            warpPoints.resize(nPoints);
            _configurator.SetArbitraryKnotsNum(nRows);    // row and column dimensions are the same

            size_t valueIndex = 0;
            for (size_t i = 0; i < nPoints; i++)
            {
                uint32_t uint_x = spPointsArray->at(valueIndex++);
                uint32_t uint_y = spPointsArray->at(valueIndex++);
                
                float x = static_cast<float>(owf.FromUint32(uint_x));
                float y = static_cast<float>(owf.FromUint32(uint_y));


                warpPoints[i].first = x;
                warpPoints[i].second = y;

                _configurator.SetArbitraryKnotNorm(i, x, y);
            }

            UpdateArbitrary(false);        
        }
        else
        {
            return false;
        }

        return true;
    }

    bool WarpAdapter::IsValidTransformation()
    {
        // The update happens asynchronously so this needs to be polled
        return _updateSucceeded.IsSignalled() && IsEngineRunStatusOk();
    }

    void WarpAdapter::SetLowFramerateFallback(bool state)
    {
        std::lock_guard lock(_paramsCs);
        _lowFrameRateFallback = state;
        ::intel_vvp_warp_lfr(_ch, state ? 1 : 0); 
    }

    bool WarpAdapter::GetLowFramerateFallback()
    {
        std::lock_guard lock(_paramsCs);
        return _lowFrameRateFallback;
    }

    // Returns true if the hardware had to repeat the output frame
    // because a new one was not ready in time (Low Frame Rate fallback)
    bool WarpAdapter::GetLowFrameRateDetected() const
    {
        const uint32_t outputStatus = intel_vvp_warp_get_output_status(GetWarpInstance(), _ch->output->idx);
        return outputStatus != 0;
    }

    intel_vvp_warp_instance* WarpAdapter::GetWarpInstance() const
    {
        return _spChannel->instance;
    }

    intel_vvp_warp::WarpLatencyParams WarpAdapter::GetLatencyParams()
    {
        std::unique_lock lock(_paramsCs);
        return _latencyParams;
    }

    bool WarpAdapter::SetLatency(uint32_t clockCycles)
    {
        if (!_ch)
            return false;
        ::intel_vvp_warp_set_output_latency(_ch, clockCycles); 
        return true;
    }  

    bool WarpAdapter::IsEngineRunStatusOk()
    {
        bool ret = false;

        if(_ch)
        {
            intel_vvp_warp_instance_t* instance = _ch->instance;

            ret = true;

            // For the arbitrary warp and 4x corner warp check
            // potential bandwidth exhaustion (valid for single bounce only)
            if((!_fixedWarp) && IsSingleBounce())
            {
                for(uint32_t e = 0; e < _ch->num_engines; ++e)
                {
                    const uint32_t idx = _ch->engines[e]->idx;
                    uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_DEBUG_ENGINE_BASE(idx) + INTEL_VVP_WARP_DEBUG_ENGINE_RUN_STATUS);

                    static constexpr uint32_t SAFE_DISTANCE = 30;
                    const uint32_t distance = (reg_val & 0xffff);

                    ret = ret & (distance > SAFE_DISTANCE);
                }
            }
        }

        return ret;
    }

    IMemTransferPtr CreateDataTransfer()
    {
        // Try create MSGDMA data transfer if the hardware is available
        IMemTransferPtr dataTransfer = MemTransferMsgdma::Create("/dev/msgdma_userio0");

        if(dataTransfer)
        {
            std::cout << "Warp data transfer: MSGDMA\n";
        }
        else
        {
            static constexpr uintptr_t MEM_BASE_CPU = 0x60000000;
            static constexpr uintptr_t MEM_BASE_FPGA = 0x00000000;
            static constexpr size_t MEM_SIZE = 0x20000000;
            dataTransfer = MemTransferCpu::Create(MEM_BASE_CPU, MEM_BASE_FPGA, MEM_SIZE);

            if(dataTransfer)
                std::cout << "Warp data transfer: CPU\n";
        }

        return dataTransfer;
    }

    PicturePlanes::PicturePlanes()
    {
    }

    PicturePlanes::PicturePlanes(uint32_t width, uint32_t height,
                                std::shared_ptr<std::vector<uint16_t>> spRedBuffer,
                                std::shared_ptr<std::vector<uint16_t>> spGreenBuffer,
                                std::shared_ptr<std::vector<uint16_t>> spBlueBuffer)
    :   _width{width}
    ,   _height{height}
    ,   _spRedBuffer{spRedBuffer}
    ,   _spGreenBuffer{spGreenBuffer}
    ,   _spBlueBuffer{spBlueBuffer}
    {
    }

    std::shared_ptr<RawRgbImage> PicturePlanes::GetInterleavedImage()
    {
        uint32_t numPixels = _width * _height;
        auto spImage = std::make_shared<RawRgbImage>(_width, _height);

        auto pDestination = spImage->GetPixel(0, 0);
        uint16_t* pRedSource = _spRedBuffer->data();
        uint16_t* pGreenSource = _spGreenBuffer->data();
        uint16_t* pBlueSource = _spBlueBuffer->data();

        for (uint32_t i = 0; i < numPixels; i++)
        {
            pDestination->_red   = (uint8_t)((*pRedSource++) >> 2);
            pDestination->_green = (uint8_t)((*pGreenSource++) >> 2);
            pDestination->_blue  = (uint8_t)((*pBlueSource++) >> 2);
            pDestination++;
        }

        return spImage;
    }

    std::shared_ptr<RawRgbImage16> PicturePlanes::GetInterleavedImage16()
    {
        uint32_t numPixels = _width * _height;
        auto spImage = std::make_shared<RawRgbImage16>(_width, _height);

        auto pDestination = spImage->GetPixel(0, 0);
        uint16_t* pRedSource = _spRedBuffer->data();
        uint16_t* pGreenSource = _spGreenBuffer->data();
        uint16_t* pBlueSource = _spBlueBuffer->data();

        for (uint32_t i = 0; i < numPixels; i++)
        {
            pDestination->_red   = *pRedSource++;
            pDestination->_green = *pGreenSource++;
            pDestination->_blue  = *pBlueSource++;
            pDestination++;
        }

        return spImage;
    }
} // namespace SwApi