/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VideoFrameWriter.h"
#include "intel_vvp_vfw.h"
#include "intel_vvp_core.h"
#include "VfwUtils.h"
#include <future>


namespace SwApi 
{
    VideoFrameWriter::VideoFrameWriter(Hapi::VvpVfwPtr spVfw, uintptr_t buffer_addr_dma, uintptr_t buffer_addr_fpga)
    : _spVfw(spVfw),
    _buffer_addr_dma{buffer_addr_dma},
    _buffer_addr_fpga{buffer_addr_fpga},
    _dataTransfer{nullptr},
    _lineSizeBytes{0},
    _lineStride{0}
    {
        bool initOk = false;
        std::string errorMsg{};

        if(_spVfw && _spVfw->GetInstance())
        {
            const auto instance = _spVfw->GetInstance();

            intel_vvp_vfw_set_base_addr(instance, static_cast<uint32_t>(buffer_addr_fpga));
            intel_vvp_vfw_set_num_buffers(instance, 1);

            static constexpr uint32_t WIDTH_DEFAULT = 3840;
            static constexpr uint32_t HEIGHT_DEFAULT = 2160;

            SetResolutionOfImage(WIDTH_DEFAULT, HEIGHT_DEFAULT);

            intel_vvp_vfw_set_inter_buffer_offset(instance, 0);
            intel_vvp_vfw_overwrite_broken_fields(instance, true);
            intel_vvp_vfw_set_run_mode(instance, eIntelVvpVfwRunMode::kIntelVvpVfwStop);

            intel_vvp_vfw_commit_writes(instance);
            intel_vvp_vfw_acknowledge_buffer(instance);

            auto dataTransfer = MemTransferMsgdma::Create("/dev/msgdma_userio0");

            if(dataTransfer)
            {
                _dataTransfer = dataTransfer;
                initOk = true;
            }
            else
                errorMsg = "Unable to create MSGDMA data transfer";
        }
        else
        {
            errorMsg = "Invalid VFW instance";
        }

        if(!initOk)
        {
            throw std::runtime_error("Failed to initialize VideoFrameWriter: " + errorMsg);
        }
    }


    void VideoFrameWriter::SetResolutionOfImage(uint32_t width, uint32_t height)
    {
        const auto instance = _spVfw->GetInstance();

        if(instance)
        {
            intel_vvp_core_set_img_info_width(instance, width);
            intel_vvp_core_set_img_info_height(instance, height);

            auto roundup_pwr2 = [](std::size_t v, std::size_t r){
                return (v + (r - 1)) & ~(r - 1);
            };

            const uint32_t pixelSizeBits = intel_vvp_vfw_get_number_of_color_planes(instance) * intel_vvp_vfw_get_bps(instance);
            _lineSizeBytes = roundup_pwr2(width * pixelSizeBits, 8) / 8;

            // Some HW configurations require burst size alignment for individual lines
            // 1024 bytes works for all currently supported HW variants
            static constexpr uint32_t LINE_STRIDE_ROUNDING_BYTES = 1024;
            _lineStride = roundup_pwr2(_lineSizeBytes, LINE_STRIDE_ROUNDING_BYTES);

            intel_vvp_vfw_set_inter_line_offset(instance, _lineStride);
            intel_vvp_vfw_commit_writes(instance);
        }
    }


    std::pair<uint32_t, uint32_t> VideoFrameWriter::GetResolutionOfImage() const
    {
        const auto instance = _spVfw->GetInstance();
        const uint32_t width = intel_vvp_core_get_img_info_width(instance);
        const uint32_t height = intel_vvp_core_get_img_info_height(instance);
        return std::make_pair(width, height);
    }


    // This will make the Vfw fill the memory with the stuff we want
    void VideoFrameWriter::CaptureFrame()
    {
        const auto instance = _spVfw->GetInstance();

        if(instance)
        {
            // Prepare
            intel_vvp_vfw_set_run_mode(instance, eIntelVvpVfwRunMode::kIntelVvpVfwSingleShot);
            intel_vvp_vfw_commit_writes(instance);
            intel_vvp_vfw_acknowledge_buffer(instance);

            for (int i = 0; i < 1000; i++)
            {
                if (intel_vvp_vfw_is_buffer_available(instance))
                {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (!intel_vvp_vfw_is_buffer_available(instance))
            {
                std::cout << "Error, VFW stalled!\n";
                return;
            }

            intel_vvp_vfw_set_run_mode(instance, eIntelVvpVfwRunMode::kIntelVvpVfwStop);
            intel_vvp_vfw_commit_writes(instance);
            intel_vvp_vfw_acknowledge_buffer(instance);
        }
    }


    vfw_frame_t VideoFrameWriter::ReadFrame(const uint32_t targetBps, bool RGB)
    {   
        vfw_frame_t frame{};

        const auto instance = _spVfw->GetInstance();

        if(instance)
        {
            const uint32_t bps = intel_vvp_vfw_get_bps(instance);
            
            const eIntelVvpVfwPacking packingMode = intel_vvp_vfw_get_mem_word_packing(instance);

            if(packingMode != kIntelVvpVfwPerfectPacking)
            {
                std::cerr << "Unsupported packing mode!\n";
                return frame;
            }

            unpack_function_t unpack_func{};
            
            switch(bps)
            {
                case 10:
                    unpack_func = (RGB ? unpack_line_10_16<true> : unpack_line_10_16<false>);
                break;
                case 12:
                    unpack_func = (RGB ? unpack_line_12_16<true> : unpack_line_12_16<false>);
                break;
                case 16:
                    unpack_func = (RGB ? unpack_line_16_16<true> : unpack_line_16_16<false>);
                break;
                default:
                break;

            }

            if(!unpack_func)
            {
                std::cerr << "Unsupported BPS! Input: " << bps << " Output: " << targetBps << "\n";
                return frame;
            }

            std::cout << "Unpacking image, please wait\n";

            const uint32_t width = intel_vvp_core_get_img_info_width(instance);
            const uint32_t height = intel_vvp_core_get_img_info_height(instance);
            const uint32_t numChannels = intel_vvp_vfw_get_number_of_color_planes(instance);
            const uint32_t lineStoreSize = _lineStride * height;

            // Aligned buffer to satisfy the MSGDMA constranints if necessary
            static constexpr std::size_t BUFFER_ALIGNMENT = 0x1000;
            vfw_data_t lineStore{lineStoreSize, BUFFER_ALIGNMENT};

            _dataTransfer->TransferFromTarget(lineStore.data(), _buffer_addr_dma, lineStoreSize);

            const std::size_t store_size = width * height * numChannels * sizeof(uint16_t);

            frame = vfw_frame_t{width, height, vfw_data_t{store_size}};

            const std::size_t dst_stride_bytes = (width * numChannels * targetBps + 7) / 8;
            const uint8_t* src = lineStore.data();
            uint8_t* dst = frame._data.data();

            for(uint32_t l = 0; l < height; ++l)
            {
                unpack_func(src, _lineSizeBytes, dst);
                src += _lineStride;
                dst += dst_stride_bytes;
            }
        }

        return frame;
    }

} // namespace SwApi
