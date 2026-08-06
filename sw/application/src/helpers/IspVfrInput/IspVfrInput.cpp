/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <ranges>
#include "IspVfrInput.h"
#include "TiffImageReader.h"
#include "PgmImageReader.h"
#include "intel_vvp_vfr.h"
#include "intel_vvp_core.h"
#include "VfrInputBgMsg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace SwApi 
{

IspVfrInput::IspVfrInput(Hapi::VvpVfrPtr spVfr,
        const uintptr_t buffer_offset_dma,
        const uintptr_t buffer_offset_fpga,
        const uint32_t buffer_size,
        std::unique_ptr<SwApi::VideoThrottle> spVideoThrottle):
    _spVfr{spVfr},
    _configured{false},
    _run{false},
    _numBufferSets{0},
    _buffer_offset_dma{buffer_offset_dma},
    _buffer_offset_fpga{buffer_offset_fpga},
    _buffer_size_max{buffer_size},
    _bufferset_size_max{0},
    _spVideoThrottle{std::move(spVideoThrottle)}
{
    auto probe_func = [this](std::string& errMsg) -> bool {
        if(!_spVfr || !_spVfr->GetInstance()) {
            errMsg = "Invalid VFR instance";
            return false;
        }

        const auto instance = _spVfr->GetInstance();
        if(instance->packing != kIntelVvpVfrPerfectPacking) {
            errMsg = "Unsupported packing format";
            return false;
        }

        // Use double buffering if possible
        _numBufferSets = std::max<uint32_t>(static_cast<uint32_t>(_spVfr->GetInstance()->max_buffer_sets), 2u);

        if(_numBufferSets < 1){
            errMsg = "VFR instance does not support any buffer sets";
            return false;
        }        

        _dataTransfer = MemTransferMsgdma::Create("/dev/msgdma_userio0");

        if(!_dataTransfer){
            errMsg = "Failed to create MSGDMA data transfer";
            return false;
        }

        _spMessageQueue = std::make_shared<SwUtils::MessageQueue>("VfrLoadQ");

        if(!_spMessageQueue){
            errMsg = "Failed to create message queue";
            return false;
        }        

        return true;
    };

    std::string errorMsg{};

    bool probeOk = probe_func(errorMsg);

    if(!probeOk)
        throw std::runtime_error(errorMsg);

#ifdef DEBUG    
    PrintHwConfiguration(spVfr);
#endif /* DEBUG */

    const auto instance = _spVfr->GetInstance();

    _bufferset_size_max = _buffer_size_max / _numBufferSets;

    intel_vvp_vfr_set_run_mode(instance, eIntelVvpVfrRunMode::kIntelVvpVfrStop);
    intel_vvp_vfr_set_num_buffer_sets(instance, _numBufferSets);

    for(uint32_t i = 0; i < _numBufferSets; ++i)
    {
        intel_vvp_vfr_set_bufset_base_addr(instance, i, _buffer_offset_fpga + i * _bufferset_size_max);
        intel_vvp_vfr_set_bufset_num_buffers(instance, i, 1);
        intel_vvp_vfr_set_bufset_inter_buffer_offset(instance, i, 0);
        intel_vvp_vfr_set_bufset_field_count(instance, i, 1);

        intel_vvp_vfr_set_bufset_bps(instance, i, instance->bps);
        intel_vvp_vfr_set_bufset_colorspace(instance, i, 0);
        intel_vvp_vfr_set_bufset_cositing(instance, i, 0);
        intel_vvp_vfr_set_bufset_interlace(instance, i, 0);
        intel_vvp_vfr_set_bufset_subsampling(instance, i, 0x3);
    }

    intel_vvp_vfr_set_buffer_mode(instance, eIntelVvpVfrBufferMode::kIntelVvpVfrSingleSet);
    intel_vvp_vfr_set_starting_buffer_set(instance, 0);

    intel_vvp_vfr_commit_writes(instance);

    GenerateStartupImage();
}


void IspVfrInput::Run(bool run)
{
    const auto instance = _spVfr->GetInstance();

    _run = run;
    if(_configured && _run)
    {
        intel_vvp_vfr_set_run_mode(instance, eIntelVvpVfrRunMode::kIntelVvpVfrFreeRunning);
    }
    else
    {
        intel_vvp_vfr_set_run_mode(instance, eIntelVvpVfrRunMode::kIntelVvpVfrStop);
    }
    intel_vvp_vfr_commit_writes(instance);
}


// VFR format is LE
// PGM format is BE
// This function takes care of it
void IspVfrInput::Bayer16ToBayerRggb12Packed(const VfrImage& bayerImage, VfrImage& bayerImagePacked)
{
    const uint8_t* src = bayerImage.Data();
    uint8_t* dst_row = bayerImagePacked.Data();

    const uint32_t dstStrideBytes = bayerImagePacked._strideBytes;

    for(uint32_t i = 0; i < bayerImage._height; ++i)
    {
        uint8_t* dst = dst_row;

        for(uint32_t j = 0; j < bayerImage._width; j += 2)
        {
            uint16_t dst1 = *src++;
            dst1 = (dst1 << 8) | *src++;
            dst1 = dst1 >> 4;

            uint16_t dst2 = *src++;
            dst2 = (dst2 << 8) | *src++;
            dst2 = dst2 >> 4;

            dst[0] = (dst1 & 0xff);
            dst[1] = ((dst1 >> 8) & 0x0f) | ((dst2 & 0x0f) << 4);
            dst[2] = ((dst2 >> 4) & 0xff);
            dst += 3;            
        }

        dst_row += dstStrideBytes;
    }    
}


// VFR format is LE
// PGM format is BE
// This function takes care of it
void IspVfrInput::Bayer16ToBayerRggb16Packed(const VfrImage& bayerImage, VfrImage& bayerImagePacked)
{
    const uint8_t* src = bayerImage.Data();
    uint8_t* dst_row = bayerImagePacked.Data();

    const uint32_t dstStrideBytes = bayerImagePacked._strideBytes;

    for(uint32_t i = 0; i < bayerImage._height; ++i)
    {
        uint8_t* dst = dst_row;

        for(uint32_t j = 0; j < bayerImage._width; ++j)
        {
            dst[1] = *src++;
            dst[0] = *src++;
            dst += 2;            
        }

        dst_row += dstStrideBytes;
    }    
}


void IspVfrInput::Rgb8ToBayerRggb12Packed(const VfrImage& rgbImage, VfrImage& bayerImage)
{
    const uint8_t* src = rgbImage.Data();
    uint8_t* dst_row = bayerImage.Data();

    const uint32_t dstStrideBytes = bayerImage._strideBytes;

    for(uint32_t i = 0; i < rgbImage._height; ++i)
    {
        uint8_t* dst = dst_row;

        for(uint32_t j = 0; j < rgbImage._width; j += 2)
        {
            uint16_t value1, value2;

            {
                const uint8_t r = *src++;
                const uint8_t g = *src++;
                const uint8_t b = *src++;

                value1 = (((i % 2) == 0) ? r : g) << 4;
            }

            {
                const uint8_t r = *src++;
                const uint8_t g = *src++;
                const uint8_t b = *src++;

                value2 = (((i % 2) == 0) ? g : b) << 4;
            }            

            // Shifted left by 8 bits to convert 8bps to destination 16bps
            dst[0] = (value1 & 0xff);
            dst[1] = ((value1 >> 8) & 0x0f) | ((value2 & 0x0f) << 4);
            dst[2] = ((value2 >> 4) & 0xff);
            dst +=3 ;
        }

        dst_row += dstStrideBytes;
    } 
}


void IspVfrInput::Rgb8ToBayerRggb16Packed(const VfrImage& rgbImage, VfrImage& bayerImage)
{
    const uint8_t* src = rgbImage.Data();
    uint8_t* dst_row = bayerImage.Data();

    const uint32_t dstStrideBytes = bayerImage._strideBytes;

    for(uint32_t i = 0; i < rgbImage._height; ++i)
    {
        uint8_t* dst = dst_row;

        for(uint32_t j = 0; j < rgbImage._width; ++j)
        {
            const uint8_t r = *src++;
            const uint8_t g = *src++;
            const uint8_t b = *src++;

            uint16_t value;

            if((i % 2) == 0)
            {
                value = ((j % 2) == 0) ? r : g;
            }
            else
            {
                value = ((j % 2) == 0) ? g : b;
            }

            // Shifted left by 8 bits to convert 8bps to destination 16bps
            *dst++ = 0;
            *dst++ = value;
        }

        dst_row += dstStrideBytes;
    } 
}


void IspVfrInput::GenerateStartupImage()
{
    static constexpr uint32_t WIDTH = 3840u;
    static constexpr uint32_t HEIGHT = 2160u;

    VfrImage rgbImage{WIDTH, HEIGHT, 3u, 8u};

    if(rgbImage.Data() == nullptr)
    {
        std::cerr << "Failed to allocate startup image\n";
        return;
    }

    uint8_t* p1 = rgbImage.Data();
    const uint8_t* const p2 = p1 + rgbImage.SizeBytes();

    // Fill background with Altera mid blue: #00377c
    while(p1 < p2)
    {
        *p1++ = 0x00u; // R
        *p1++ = 0x37u; // G
        *p1++ = 0x7cu; // B
    }

    // Add prompt to load images on  top
    const uint32_t h = (rgbImage._width - vfr_input_msg_width) / 2;
    const uint32_t v = (rgbImage._height - vfr_input_msg_height) / 2;

    const uint32_t rgbPixelSizeBytes = (rgbImage._bps * rgbImage._channels + 7) / 8;
    const uint32_t rgbStride = rgbImage._width * rgbPixelSizeBytes;

    p1 = rgbImage.Data() + v * rgbStride + h * rgbPixelSizeBytes;
    const uint8_t* msg_src = vfr_input_msg_data;

    for(uint32_t r = 0; r < vfr_input_msg_height; ++r)
    {
        uint8_t* dst = p1;

        for(uint32_t c = 0; c < vfr_input_msg_width; ++c)
        {                
            *dst++ = *msg_src++;
            *dst++ = *msg_src++;
            *dst++ = *msg_src++;                
        }

        p1 += rgbStride;
    }

    // Generate Bayer pattern
    VfrImage bayerImage = AllocateBayerImagePacked(rgbImage);

    const auto instance = _spVfr->GetInstance();

    if(instance->bps == 12)
        Rgb8ToBayerRggb12Packed(rgbImage, bayerImage);
    else
        Rgb8ToBayerRggb16Packed(rgbImage, bayerImage);

    // Round up to nearest 4KB for MSGDMA
    static constexpr uint32_t MSGDMA_ALIGNMENT = 0x1000;
    const uint32_t bufferSizeBytesRounded = ((bayerImage.SizeBytes() + (MSGDMA_ALIGNMENT - 1)) / MSGDMA_ALIGNMENT) * MSGDMA_ALIGNMENT;

    const uint32_t nextBufferSet = (_currentBufferSet + 1) % _numBufferSets;
    const uintptr_t bufferset_base_dma =  _buffer_offset_dma + nextBufferSet * _bufferset_size_max;    

    TransferFrameData(bayerImage.Data(), bayerImage.SizeBytes(), bufferset_base_dma);

    intel_vvp_vfr_set_starting_buffer_set(instance, nextBufferSet);

    intel_vvp_core_set_img_info_width(instance, WIDTH);
    intel_vvp_core_set_img_info_height(instance, HEIGHT);

    intel_vvp_vfr_set_bufset_width(instance, nextBufferSet, WIDTH);
    intel_vvp_vfr_set_bufset_height(instance, nextBufferSet, HEIGHT);
    intel_vvp_vfr_set_bufset_inter_line_offset(instance, nextBufferSet, bayerImage._strideBytes);

    intel_vvp_vfr_set_bufset_num_buffers(instance, nextBufferSet, 1);
    intel_vvp_vfr_set_bufset_inter_buffer_offset(instance, nextBufferSet, bufferSizeBytesRounded);

    if(_run)
    {
        intel_vvp_vfr_set_run_mode(instance, eIntelVvpVfrRunMode::kIntelVvpVfrFreeRunning);
    }
    intel_vvp_vfr_commit_writes(instance);
    _configured = true;

    _currentBufferSet = nextBufferSet;

    _width = WIDTH;
    _height = HEIGHT;

    if(_spVideoThrottle)
    {
        _spVideoThrottle->Configure(_frameRate, _height); // Default to 30fps
    }

}

std::pair<uint32_t, uint32_t> IspVfrInput::GetResolution() const
{
    std::lock_guard lock(_mutex);
    return std::make_pair(_width, _height);
}


std::pair<uint32_t, uint32_t> IspVfrInput::GetMaxResolution() const
{
    const uint32_t width = intel_vvp_vfr_get_max_width(_spVfr->GetInstance());
    const uint32_t height = intel_vvp_vfr_get_max_height(_spVfr->GetInstance());
    return std::make_pair(width, height);
}


void IspVfrInput::LoadSequence(const std::vector<std::filesystem::path>& frame_paths)
{
    if(frame_paths.empty())
        return;

    if(!_spMessageQueue)
    {
        _spMessageQueue = std::make_shared<SwUtils::MessageQueue>("VfrLoadQ");
    }

    const uint64_t generation = ++_loadGeneration;
    _spMessageQueue->FlushMessages();

    auto framePathsCopy = frame_paths;
    auto loadCb = [this, framePaths = std::move(framePathsCopy), generation]() mutable {
        LoadSequenceImpl(std::move(framePaths), generation);
    };
    _spMessageQueue->Add(new SwUtils::SingleCallbackCmd(loadCb));
}


void IspVfrInput::SetSourceMetadataCallback(source_metadata_callback_t callback)
{
    _sourceMetadataCallback = std::move(callback);
}


void IspVfrInput::SetFrameRate(const uint32_t v)
{
    std::lock_guard lock(_mutex);
    _frameRate = v;

    if(_spVideoThrottle)
        _spVideoThrottle->Configure(_frameRate, _height);
}


void IspVfrInput::LoadSequenceImpl(std::vector<std::filesystem::path> frame_paths, uint64_t generation)
{
    if(frame_paths.empty())
        return;

    if(generation != _loadGeneration.load())
        return;

    if(!_spVfr)
    {
        std::cerr << "VFR is not available\n";
        return;
    }

    if(!_dataTransfer)
    {
        std::cerr << "VFR data transfer is not available\n";
        return;
    }

    const auto instance = _spVfr->GetInstance();        
    if(!instance)
    {
        std::cerr << "Invalid VFR instance\n";
        return;
    }

    uint32_t sequenceWidth = 0;
    uint32_t sequenceHeight = 0;

    std::size_t srcStrideBytes = 0;
    uint32_t bufferSizeBytesRounded = 0;

    VfrImage bayerImage{};

    const uint32_t nextBufferSet = (_currentBufferSet + 1) % _numBufferSets;
    const uintptr_t bufferset_base_dma =  _buffer_offset_dma + nextBufferSet * _bufferset_size_max;
    uintptr_t buffer_addr_dma = bufferset_base_dma;

    // Current frame
    std::size_t i = 0;
    VfrSourceMetadata loadedSourceMetadata;

    // Sort files by name to ensure correct sequence order
    std::ranges::sort(frame_paths);

    for(const auto& filePath : frame_paths)
    {
        if(generation != _loadGeneration.load())
            return;

        std::string error_str{};

        VfrImage vfrImage = LoadImageFile(filePath, error_str);

        if(vfrImage.Data() == nullptr)
        {
            std::cerr << "Failed to load frame '" << filePath.string() << "': " << error_str << "\n";
            continue;
        }

        const auto comp = vfrImage._channels;

        if(comp != 1 && comp != 3 && comp != 4)
        {
            std::cerr << "Unsupported image channel count in frame '" << filePath.string() << "': " << comp << "\n";
            continue;
        }

        const uint32_t width = vfrImage._width;
        const uint32_t height = vfrImage._height;

        if((sequenceWidth == 0) && (sequenceHeight == 0))
        {
            const auto [minW, minH] = GetMinResolution();
            const auto [maxW, maxH] = GetMaxResolution();

            if(width < minW || height < minH){
                std::cerr << "Sequence frame resolution is below Frame Reader minimum.\n";
                continue;
            }

            if(width > maxW || height > maxH){
                std::cerr << "Sequence frame resolution exceeds Frame Reader maximum.\n";
                continue;
            }

            if((width % 2u) != 0u || (height % 2u) != 0u){
               std::cerr << "Sequence frame width and height must be even for Bayer.\n";
               continue;
            }

            sequenceWidth = width;
            sequenceHeight = height;

            bayerImage = AllocateBayerImagePacked(vfrImage);
            bufferSizeBytesRounded = ((bayerImage._sizeBytes + 0xFFF) / 0x1000) * 0x1000; // Round up to nearest 4KB for MSGDMA
        }
        else
        {
            if(width != sequenceWidth || height != sequenceHeight){
                std::cerr << "All sequence frames must have the same resolution.\n";
                continue;
            }
        }

        // Check if next frame can fit in the buffer set
        if((buffer_addr_dma + bufferSizeBytesRounded) > (bufferset_base_dma + _bufferset_size_max)){
            std::cerr << "Buffer set size exceeded, cannot load more frames in the sequence.\n";
            break;
        }

        if(comp == 1)
        {
            if(instance->bps == 12)
                Bayer16ToBayerRggb12Packed(vfrImage, bayerImage);
            else
                Bayer16ToBayerRggb16Packed(vfrImage, bayerImage);
        }
        else
        {
            if(instance->bps == 12)
                Rgb8ToBayerRggb12Packed(vfrImage, bayerImage);
            else
                Rgb8ToBayerRggb16Packed(vfrImage, bayerImage);
        }

        if(generation != _loadGeneration.load())
            return;

        TransferFrameData(bayerImage.Data(), bayerImage.SizeBytes(), buffer_addr_dma);

        ++i;
        buffer_addr_dma += bufferSizeBytesRounded;

        if(i == 1)
        {
            loadedSourceMetadata._filePath = filePath.filename();
            loadedSourceMetadata._width = width;
            loadedSourceMetadata._height = height;
            loadedSourceMetadata._bitsPerSample = vfrImage._bps;
            loadedSourceMetadata._sourceType = (comp == 1)
                ? VfrSourceMetadata::SourceType::Bayer
                : VfrSourceMetadata::SourceType::RGB;
        }
    }

    // If anything has been loaded at all
    if(i)
    {
        if(generation != _loadGeneration.load())
            return;

        intel_vvp_vfr_set_starting_buffer_set(instance, nextBufferSet);

        intel_vvp_core_set_img_info_width(instance, sequenceWidth);
        intel_vvp_core_set_img_info_height(instance, sequenceHeight);

        intel_vvp_vfr_set_bufset_width(instance, nextBufferSet, sequenceWidth);
        intel_vvp_vfr_set_bufset_height(instance, nextBufferSet, sequenceHeight);
        intel_vvp_vfr_set_bufset_inter_line_offset(instance, nextBufferSet, bayerImage._strideBytes);

        intel_vvp_vfr_set_bufset_num_buffers(instance, nextBufferSet, i);
        intel_vvp_vfr_set_bufset_inter_buffer_offset(instance, nextBufferSet, bufferSizeBytesRounded);

        if(_run)
        {
            intel_vvp_vfr_set_run_mode(instance, eIntelVvpVfrRunMode::kIntelVvpVfrFreeRunning);
        }
        intel_vvp_vfr_commit_writes(instance);
        _configured = true;

        _currentBufferSet = nextBufferSet;

        {
            std::lock_guard lock(_mutex);
            _width = sequenceWidth;
            _height = sequenceHeight;

            if(_spVideoThrottle)
                _spVideoThrottle->Configure(_frameRate, _height);
        }

        if(_sourceMetadataCallback && loadedSourceMetadata.IsValid())
        {
            _sourceMetadataCallback(loadedSourceMetadata);
        }
    }
}


VfrImage IspVfrInput::AllocateBayerImagePacked(const VfrImage& rgbImage)
{
    VfrImage bayerImage{};

    const auto instance = _spVfr->GetInstance();

    if(!instance)
    {
        std::cerr << "Invalid VFR instance\n";
        return {};
    }

    const uint32_t width = rgbImage._width;
    const uint32_t height = rgbImage._height;

    auto roundup_pwr2 = [](std::size_t v, std::size_t r){
        return (v + (r - 1)) & ~(r - 1);
    };    

    static constexpr uint32_t LINE_STRIDE_ROUNDING_BYTES = 1024;
    static constexpr std::size_t BUFFER_ALIGNMENT = 0x1000;
    const uint32_t pixelSizeBits = instance->num_color_planes * instance->bps;
    // Note: for simplicity the size is calculated assuming the worst case: colour packing
    // even though the current implementation only supports perfect packing
    const uint32_t lineSizeBytes = width * ((pixelSizeBits + 7) / 8);
    const uint32_t lineStrideBytes = roundup_pwr2(lineSizeBytes, LINE_STRIDE_ROUNDING_BYTES);
    const uint32_t bufferSizeBytes = lineStrideBytes * height;
    
    auto packedVfrBayerData = VfrImage::buffer_t{(uint8_t*)aligned_alloc(BUFFER_ALIGNMENT, bufferSizeBytes), [](uint8_t* p){ free(p);}};

    // ToDo: Currently only 1 channel x 16bps HW configuration is supported.
    // Check for this
    bayerImage.AssignExternal(width, height, instance->num_color_planes, instance->bps, std::move(packedVfrBayerData), LINE_STRIDE_ROUNDING_BYTES);

    return bayerImage;
}


VfrImage IspVfrInput::LoadImageFile(const std::filesystem::path& filePath, std::string& error_str)
{
    VfrImage vfrImage{};

    if(IsTiffFileExtension(filePath))
    {
        vfrImage = LoadBasicTiffImage8(filePath, error_str);
    }
    else if(IsPgmFileExtension(filePath))
    {
        vfrImage = LoadPgmImage(filePath, error_str);
    }
    else
    {
        int x = 0, y = 0, comp = 0;
        auto rawData = VfrImage::buffer_t{stbi_load(filePath.string().c_str(), &x, &y, &comp, 3), [](uint8_t* p){ stbi_image_free(p);}};

        vfrImage.AssignExternal(static_cast<uint32_t>(x), static_cast<uint32_t>(y),
                                static_cast<uint32_t>(comp), 8u, std::move(rawData));
    }

    return vfrImage;
}


bool IspVfrInput::TransferFrameData(const uint8_t* src, const std::size_t size, uintptr_t dst)
{
    bool ret = true;

    // Split into fixed size DMA transfers to avoid bus hogging
    static constexpr std::size_t DMA_TRANSFER_LIMIT = 0x400000;
    uintptr_t dma_dst = dst;
    
    std::size_t dma_bytes_remaining = size;

    for(std::size_t t = 0; t < size / DMA_TRANSFER_LIMIT; ++ t)
    {
        ret = ret && _dataTransfer->TransferToTarget(dma_dst, (void*)src, DMA_TRANSFER_LIMIT);
        dma_dst += DMA_TRANSFER_LIMIT;
        src += DMA_TRANSFER_LIMIT;
        dma_bytes_remaining -= DMA_TRANSFER_LIMIT;
    }

    if(dma_bytes_remaining)
        ret = ret && _dataTransfer->TransferToTarget(dma_dst, (void*)src, dma_bytes_remaining);

    return ret;
}

#ifdef DEBUG
void IspVfrInput::PrintHwConfiguration(Hapi::VvpVfrPtr spVfr)
{
    const auto instance = spVfr->GetInstance();
    if(!instance)
    {
        std::cerr << "Invalid VFR instance\n";
         return;
     }

     printf("VFR configuration:\n");
     printf("Max width: %d\n", instance->max_width);
     printf("Max height: %d\n", instance->max_height);        
     printf("Max buffer sets: %d\n", instance->max_buffer_sets);
     printf("Bits per sample: %d\n", instance->bps);
     printf("Number of color planes: %d\n", instance->num_color_planes);
     printf("Pixels in parallel: %d\n", instance->pip);
     printf("Packing: ");
     switch(instance->packing)
     {
         case kIntelVvpVfrPerfectPacking:
             printf("Perfect\n");
             break;
         case kIntelVvpVfrColorPacking:
             printf("Color\n");
             break;
         case kIntelVvpVfrPixelPacking:
             printf("Pixel\n");
             break;
         default:
             printf("Unknown\n");
             break;
     }
}
#endif /* DEBUG */

} // namespace SwApi
