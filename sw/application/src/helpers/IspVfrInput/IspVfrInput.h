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
#include <cstdlib>
#include <cstring>
#include <memory>
#include <atomic>
#include <mutex>
#include <chrono>
#include <string>
#include <iostream>
#include <functional>
#include "HapiVvpVfr.h"
#include "IspCommon.h"
#include "SwUtils.h"
#include "SwUtilsMemTransfer.h"
#include "SwUtilsMemBuffer.h"
#include "VfrInputUtils.h"
#include "VideoThrottle.h"


namespace SwApi 
{

struct VfrSourceMetadata
{
    enum class SourceType
    {
        Unknown,
        Bayer,
        RGB
    };

    std::filesystem::path _filePath;
    uint32_t _width = 0;
    uint32_t _height = 0;
    uint32_t _bitsPerSample = 0;
    SourceType _sourceType = SourceType::Unknown;

    bool IsValid() const
    {
        return !_filePath.empty() && _width != 0 && _height != 0 && _bitsPerSample != 0 && _sourceType != SourceType::Unknown;
    }
};


class IspVfrInput
{
public:
    using source_metadata_callback_t = std::function<void(const VfrSourceMetadata&)>;

    IspVfrInput(Hapi::VvpVfrPtr spVfr,
        const uintptr_t buffer_offset_dma,
        const uintptr_t buffer_offset_fpga,
        const uint32_t buffer_size,
        std::unique_ptr<SwApi::VideoThrottle> videoThrottle = nullptr);
    virtual ~IspVfrInput(){}
    IspVfrInput(const IspVfrInput& other) = delete;
    IspVfrInput& operator=(const IspVfrInput& other) = delete;
    IspVfrInput(IspVfrInput&& other) = delete;
    IspVfrInput& operator=(IspVfrInput&& other) = delete;

    std::pair<uint32_t, uint32_t> GetResolution() const;
    std::pair<uint32_t, uint32_t> GetMaxResolution() const;
    constexpr std::pair<uint32_t, uint32_t> GetMinResolution() const { return {32, 32}; }

    void LoadSequence(const std::vector<std::filesystem::path>& frame_paths);
    void SetSourceMetadataCallback(source_metadata_callback_t callback);

    void SetFrameRate(const uint32_t v);

    void Run(bool run);

private:    
    void SetResolution(uint32_t width, uint32_t height);
    void LoadSequenceImpl(std::vector<std::filesystem::path> frame_paths, uint64_t generation);
    VfrImage LoadImageFile(const std::filesystem::path& filePath, std::string& error);

    void GenerateStartupImage();

    VfrImage AllocateBayerImagePacked(const VfrImage& rgbImage);
    
    void Rgb8ToBayerRggb12Packed(const VfrImage& rgbImage, VfrImage& bayerImage);
    void Rgb8ToBayerRggb16Packed(const VfrImage& rgbImage, VfrImage& bayerImage);
    void Bayer16ToBayerRggb12Packed(const VfrImage& bayerImage, VfrImage& bayerImagePacked);
    void Bayer16ToBayerRggb16Packed(const VfrImage& bayerImage, VfrImage& bayerImagePacked);
    bool TransferFrameData(const uint8_t* src, const std::size_t size, uintptr_t dst);

#ifdef DEBUG
    void PrintHwConfiguration(Hapi::VvpVfrPtr spVfr);
#endif /* DEBUG */    
    
    Hapi::VvpVfrPtr _spVfr = nullptr;
    bool _configured;
    bool _run;
    uint32_t _numBufferSets;
    uint32_t _currentBufferSet = 0;

    uintptr_t _buffer_offset_dma;
    uintptr_t _buffer_offset_fpga;
    uint32_t _buffer_size_max;

    uint32_t _bufferset_size_max;
    
    std::shared_ptr<SwApi::IMemTransfer> _dataTransfer;

    std::shared_ptr<SwUtils::MessageQueue> _spMessageQueue;

    std::atomic<uint64_t> _loadGeneration{0};

    static constexpr uint32_t FRAME_RATE_DEFAULT = 30;

    // Below members can be accessed concurrently
    // from the vfr queue and externally via GetResolution()
    mutable std::mutex _mutex;
    uint32_t _width = 0;
    uint32_t _height = 0;
    uint32_t _frameRate = FRAME_RATE_DEFAULT;

    source_metadata_callback_t _sourceMetadataCallback;

    std::unique_ptr<SwApi::VideoThrottle> _spVideoThrottle;
};

} // namespace SwApi
