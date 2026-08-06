/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VideoFrameReader.h"
#include "intel_vvp_vfr.h"
#include "intel_vvp_core.h"
#include "VfrUtils.h"
#include <future>


namespace SwApi 
{
    VideoFrameReader::VideoFrameReader(Hapi::VvpVfrPtr spVfr, uintptr_t buffer_addr_cpu, uintptr_t buffer_addr_fpga)
    : _spVfr(spVfr),
    _buffer_addr_fpga{buffer_addr_fpga},
    _lineSizeBytes{0},
    _lineStride{0}
    {
        intel_vvp_vfr_set_num_buffer_sets(_spVfr->GetInstance(), 1);
        intel_vvp_vfr_set_bufset_base_addr(_spVfr->GetInstance(), 0, _buffer_addr_fpga);
        intel_vvp_vfr_set_bufset_num_buffers(_spVfr->GetInstance(), 0, 1);

        static constexpr uint32_t WIDTH_DEFAULT = 3840;
        static constexpr uint32_t HEIGHT_DEFAULT = 2160;

        SetResolutionOfImage(WIDTH_DEFAULT, HEIGHT_DEFAULT);

        intel_vvp_vfr_set_bufset_inter_buffer_offset(_spVfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_run_mode(_spVfr->GetInstance(), eIntelVvpVfrRunMode::kIntelVvpVfrStop);

        intel_vvp_vfr_commit_writes(_spVfr->GetInstance());

        auto dataTransfer = MemTransferMsgdma::Create("/dev/msgdma_userio0");

        if(dataTransfer)
        {
            std::cout << "VFR data transfer: MSGDMA\n";
            _dataTransfer = dataTransfer;
        }
        else
        {
            auto  dataTransfer = MemTransferCpu::Create(buffer_addr_cpu, _buffer_addr_fpga);

            if(dataTransfer)
            {
                std::cout << "VFR data transfer: CPU\n";
                _dataTransfer = dataTransfer;
            }
        }
    }


    void VideoFrameReader::SetResolutionOfImage(uint32_t width, uint32_t height)
    {
        intel_vvp_core_set_img_info_width(_spVfr->GetInstance(), width);
        intel_vvp_core_set_img_info_height(_spVfr->GetInstance(), height);

        auto roundup_pwr2 = [](std::size_t v, std::size_t r){
            return (v + (r - 1)) & ~(r - 1);
        };

        const uint32_t pixelSizeBits = intel_vvp_vfr_get_number_of_color_planes(_spVfr->GetInstance()) * intel_vvp_vfr_get_bps(_spVfr->GetInstance());
        _lineSizeBytes = roundup_pwr2(width * pixelSizeBits, 8) / 8;

        // Some HW configurations require burst size alignment for individual lines
        // 1024 bytes works for all currently supported HW variants
        static constexpr uint32_t LINE_STRIDE_ROUNDING_BYTES = 1024;
        _lineStride = roundup_pwr2(_lineSizeBytes, LINE_STRIDE_ROUNDING_BYTES);

        intel_vvp_vfr_set_bufset_base_addr(_spVfr->GetInstance(), 0, _buffer_addr_fpga);
        intel_vvp_vfr_set_bufset_inter_buffer_offset(_spVfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_inter_line_offset(_spVfr->GetInstance(), 0, _lineStride);
        intel_vvp_vfr_set_bufset_field_count(_spVfr->GetInstance(), 0, 1);

        intel_vvp_vfr_set_bufset_bps(_spVfr->GetInstance(), 0, 10);
        intel_vvp_vfr_set_bufset_colorspace(_spVfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_cositing(_spVfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_interlace(_spVfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_subsampling(_spVfr->GetInstance(), 0, 0x3);

        intel_vvp_vfr_set_bufset_width(_spVfr->GetInstance(), 0, width);
        intel_vvp_vfr_set_bufset_height(_spVfr->GetInstance(), 0, height);

        intel_vvp_vfr_commit_writes(_spVfr->GetInstance());
    }


    std::pair<uint32_t, uint32_t> VideoFrameReader::GetResolutionOfImage() const
    {
        const uint32_t width = intel_vvp_core_get_img_info_width(_spVfr->GetInstance());
        const uint32_t height = intel_vvp_core_get_img_info_height(_spVfr->GetInstance());
        return std::make_pair(width, height);
    }


    // This will make the Vfr fill the memory with the stuff we want
    void VideoFrameReader::Run(bool state)
    {
        if(state)
        {
            intel_vvp_vfr_set_run_mode(_spVfr->GetInstance(), eIntelVvpVfrRunMode::kIntelVvpVfrFreeRunning);
        }
        else
        {
            intel_vvp_vfr_set_run_mode(_spVfr->GetInstance(), eIntelVvpVfrRunMode::kIntelVvpVfrStop);
        }
        intel_vvp_vfr_commit_writes(_spVfr->GetInstance());
    }


    void VideoFrameReader::WriteFrame(vfr_frame_t& frame, bool RGB)
    {   
        const uint32_t bps = intel_vvp_vfr_get_bps(_spVfr->GetInstance());
        const eIntelVvpVfrPacking packingMode = intel_vvp_vfr_get_mem_word_packing(_spVfr->GetInstance());

        if(packingMode != kIntelVvpVfrPerfectPacking)
        {
            std::cerr << "Unsppported packing mode!\n";
            return;
        }

        pack_function_t pack_func{};
        
        switch(bps)
        {
            case 10:
                if(RGB)
                {
                    if(frame._rgb)
                    {
                        pack_func = pack_line_16_10<true, true>;
                    }
                    else
                    {
                        pack_func = pack_line_16_10<true, false>;
                    }
                }
                else
                {
                    if(frame._rgb)
                    {
                        pack_func = pack_line_16_10<false, true>;
                    }
                    else
                    {
                        pack_func = pack_line_16_10<false, false>;
                    }
                }
            break;
            case 12:
                if(RGB)
                {
                    if(frame._rgb)
                    {
                        pack_func = pack_line_16_12<true, true>;
                    }
                    else
                    {
                        pack_func = pack_line_16_12<true, false>;
                    }
                }
                else
                {
                    if(frame._rgb)
                    {
                        pack_func = pack_line_16_12<false, true>;
                    }
                    else
                    {
                        pack_func = pack_line_16_12<false, false>;
                    }
                }
            break;
            case 16:
                if(RGB)
                {
                    if(frame._rgb)
                    {
                        pack_func = pack_line_16_16<true, true>;
                    }
                    else
                    {
                        pack_func = pack_line_16_16<true, false>;
                    }
                }
                else
                {
                    if(frame._rgb)
                    {
                        pack_func = pack_line_16_16<false, true>;
                    }
                    else
                    {
                        pack_func = pack_line_16_16<false, false>;
                    }
                }
            break;
            default:
            break;

        }

        if(!pack_func)
        {
            std::cerr << "Unsupported BPS! Input: " << frame._bps << " Output: " << bps << "\n";
            return;
        }

        std::cout << "Unpacking image, please wait\n";

        const uint32_t width = intel_vvp_core_get_img_info_width(_spVfr->GetInstance());
        const uint32_t height = intel_vvp_core_get_img_info_height(_spVfr->GetInstance());
        const uint32_t numChannels = intel_vvp_vfr_get_number_of_color_planes(_spVfr->GetInstance());
        const uint32_t lineStoreSize = _lineStride * height;

        // Aligned buffer to satisfy the MSGDMA constranints if necessary
        static constexpr std::size_t BUFFER_ALIGNMENT = 0x1000;
        vfr_data_t lineStore{lineStoreSize, BUFFER_ALIGNMENT};
        const std::size_t store_size = width * height * numChannels * sizeof(uint16_t);

        const std::size_t src_stride_bytes = (width * numChannels * frame._bps + 7) / 8;
        const uint8_t* src = frame._data.data();
        uint8_t* dst = lineStore.data();

        for(uint32_t l = 0; l < height; ++l)
        {
            pack_func(src, _lineSizeBytes, dst);
            src += src_stride_bytes;
            dst += _lineStride;
        }
        
        _dataTransfer->TransferToTarget(_buffer_addr_fpga, lineStore.data(), lineStoreSize);
        static constexpr uintptr_t extra_offset = 0x0200000000;
        _dataTransfer->TransferToTarget(extra_offset+_buffer_addr_fpga, lineStore.data(), lineStoreSize);
    }

} // namespace SwApi
