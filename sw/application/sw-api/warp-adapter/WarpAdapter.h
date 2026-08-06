/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpWarp.h"
#include <memory>
#include <cstdint>
#include "intel_vvp_warp.h"
#include "WarpConfigurator.h"
#include "WarpDataGenerator.h"
#include "SwUtils.h"
#include "SwUtilsMemTransfer.h"
#include "WarpAdapterRawRgbImage.h"
#include "SwUtilsLockedHandle.h"


namespace SwApi
{
    class PicturePlanes;
    
    class WarpAdapter
    {
    public:
        static std::shared_ptr<WarpAdapter> Create(Hapi::VvpWarpPtr spVvpWarp,
                const uint32_t num_engines,
                const std::size_t dma_bus_offset = 0x0);

        // Base address for the Warp RAM (frame buffers + coefficient data)
        static constexpr uintptr_t WARP_BASE_FPGA   = 0x00000000;

        enum class WarpAdapterMode {
            Uninitialized = -1,
            Fixed = 0,
            Corners,
            Arbitrary,
            Fisheye,
            Max
        };

        using WarpChannelPtr = std::unique_ptr<intel_vvp_warp_channel_t, decltype(&intel_vvp_warp_free_channel)>;

        WarpAdapter(WarpChannelPtr spWarpChannel, 
                                    IMemTransferPtr spDataTransfer,
                                    uintptr_t warp_ram_base = WARP_BASE_FPGA,
                                    std::size_t dma_bus_offset = 0x0);
        virtual ~WarpAdapter();

        enum class EasyWarpRotation : uint32_t { None, R90, R180, R270 };

        void SetHapiItem(Hapi::VvpWarpPtr spVvpWarp);
        
        bool IsEasyWarp();
        bool IsSingleBounce();
        bool IsMipmapEnabled() const;
        bool SetEasyWarpMirror(bool mirror);
        bool SetEasyWarpRotation(EasyWarpRotation rotation);
        void SetInputResolution(uint32_t width, uint32_t height);
        void SetOutputResolution(uint32_t width, uint32_t height);
        std::pair<uint32_t, uint32_t> GetOutputResolution();
        void SetVideoResolution(uint32_t input_width, uint32_t input_height, 
                                uint32_t output_width, uint32_t output_height,
                                uint32_t outputFullHeight, uint32_t frameRatex100);
        
        bool GetBypass() { return _bypass; }
        void SetBypass(bool bypass, uint32_t overrideWidth = 0, uint32_t overrideHeight = 0);

        void Update(bool waitForCompletion = false);
        void UpdateFixed(bool waitForCompletion = false);
        void UpdateCorners(bool waitForCompletion = false);
        void UpdateArbitrary(bool waitForCompletion = false);
        void UpdateFisheye(bool waitForCompletion = false);
        WarpAdapterMode GetMode();
        std::shared_ptr<RawRgbImage> CapturePicture();
        std::shared_ptr<RawRgbImage16> CapturePicture10();
        bool ExportMesh(std::filesystem::path filePath, bool asJSON);
        bool ImportMesh(std::filesystem::path filePath,
                        std::vector<std::pair<float, float>>& warpPoints,
                        uint32_t& nRows,
                        uint32_t& nColumns);
        bool IsValidTransformation();
        void SetLowFramerateFallback(bool state);
        bool GetLowFramerateFallback();
        bool GetLowFrameRateDetected() const;
        intel_vvp_warp::WarpLatencyParams GetLatencyParams();
        bool SetLatency(uint32_t clockCycles);

        void PrintDebugRegisters();

        bool IsEngineRunStatusOk();

        auto GetConfiguratorSafe()
        {
            using TWarpConfiguratorSafe = SwUtils::LockedHandle<intel_vvp_warp::WarpConfigurator, std::lock_guard<std::recursive_mutex>>;
            return TWarpConfiguratorSafe{_configurator, _paramsCs};
        }

    private:
        intel_vvp_warp_instance* GetWarpInstance() const;
        std::shared_ptr<PicturePlanes> CapturePicturePlanes();

        static constexpr uint32_t _COEF_PAGES_MAX = 2;
        static constexpr uint32_t _SKIP_RAM_PAGES_HW_TOTAL = 2;

        using warp_res_t = std::pair<uint32_t, uint32_t>;
        bool ConfigureChannel(const warp_res_t& ires, const warp_res_t& ores);
        bool IsChannelConfigured() const;
        bool ApplyTransform(const intel_vvp_warp::WarpDataContext& ctx, intel_vvp_warp::WarpDataPtr user_data);
        
        uint32_t GetPage();
        void SetPage(const uint32_t page);
        uint32_t GetSkipRamPage();
        void SetSkipRamPage(const uint32_t page);
        void DoUpdate();
        void ScheduleUpdate(bool waitForCompletion);

        using TMeshFunction = std::function<intel_vvp_warp::WarpMeshPtr(intel_vvp_warp::WarpConfigurator& cfg)>;
        void SetMeshFunction(TMeshFunction meshFunction);
        bool ProgramEasyWarp();

        intel_vvp_warp_channel_t* _ch;	
        WarpChannelPtr _spChannel;	
        std::recursive_mutex _paramsCs;
        SwUtils::Event _updateSucceeded;
        intel_vvp_warp::WarpConfigurator _configurator;
        intel_vvp_warp::WarpDataGenerator _data_generator;
        uintptr_t _framebuffer_ram_base;
        uintptr_t _coef_ram_base;
        bool _bypass;
        bool _lowFrameRateFallback;
        uint32_t _page;
        uint32_t _skip_ram_page;
        uint32_t _skip_ram_page_bypass_off;
        IMemTransferPtr _spDataTransfer;
        Hapi::VvpWarpPtr _spVvpWarp;
        std::shared_ptr<SwUtils::MessageQueue> _spMessageQueue;

        TMeshFunction _meshFunction;
        bool _updatePending;
        bool _isEasyWarp = false;
        bool _easyWarpMirror = false;
        EasyWarpRotation _easyWarpRotation = EasyWarpRotation::None;

        intel_vvp_warp::WarpLatencyParams _latencyParams = {};
        uint32_t _outputFullHeight = 0;
        uint32_t _frameRateX100 = 0;

        WarpAdapterMode _mode = WarpAdapterMode::Uninitialized;

        // Mutex for syncronizing access to _spDataTransfer
        // when e.g. multiple transforms are loaded in the background
        std::mutex _txMutex;
        // Optional offset for MSGDMA transfers
        // May be reququired in some designs to match
        // RTL address mapping
        std::size_t _dma_bus_offset;
        bool TransferEngineData(uintptr_t* dst, void** src, std::size_t* sz);

        bool _fixedWarp = true;
    };

    class PicturePlanes
    {
    public:
        PicturePlanes();
        PicturePlanes(uint32_t width, uint32_t height,
                    std::shared_ptr<std::vector<uint16_t>> spRedBuffer,
                    std::shared_ptr<std::vector<uint16_t>> spGreenBuffer,
                    std::shared_ptr<std::vector<uint16_t>> spBlueBuffer);
        bool IsValid() { return ((_width * _height) != 0); }
        std::shared_ptr<RawRgbImage> GetInterleavedImage();
        std::shared_ptr<RawRgbImage16> GetInterleavedImage16();

        uint32_t _width = 0;
        uint32_t _height = 0;
        std::shared_ptr<std::vector<uint16_t>> _spRedBuffer;
        std::shared_ptr<std::vector<uint16_t>> _spGreenBuffer;
        std::shared_ptr<std::vector<uint16_t>> _spBlueBuffer;
    };
} // namespace SwApi