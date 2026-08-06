/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <cstring>
#include <format>
#include <iostream>
#include "SwUtilsMemTransfer.h"

namespace SwApi
{

std::shared_ptr<IMemTransfer> MemTransferMsgdma::Create(const std::string &device_name)
{
    static std::map<std::string, std::shared_ptr<MemTransferMsgdma>> _device_map;

    std::shared_ptr<MemTransferMsgdma> device{nullptr};

    auto it = _device_map.find(device_name);

    if(it != _device_map.end())
        device = it->second;
    else
    {
        try
        {
            device = std::shared_ptr<MemTransferMsgdma>(new MemTransferMsgdma(device_name));
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error creaging MSGDMA transfer handler: " << e.what() << "\n";
        }

        if(device)
            _device_map[device_name] = device;
    }

    return device;
}

MemTransferMsgdma::MemTransferMsgdma(const std::string& device_name)
{
    _file = fopen(device_name.c_str(), "rw+");

    if(_file != nullptr)
    {
        // Turn off buffering
        setvbuf(_file, NULL, _IONBF, 0);
    }
    else
    {
        std::string error_msg = "Failed to open " + device_name;
        throw std::runtime_error(error_msg);
    }
}

MemTransferMsgdma::~MemTransferMsgdma()
{
    if( _file )
    {
        fclose(_file);
        _file = nullptr;
    }
}

bool MemTransferMsgdma::TransferToTarget(uintptr_t dst, void* src, std::size_t sz)
{
    bool ret = false;

    std::unique_lock<std::mutex> lock(_mutex);

    if(_file)
    {
        if(fseek(_file, dst, SEEK_SET) == 0)
        {
            size_t actual_write_size = fwrite(src, 1, sz, _file);

            if (actual_write_size == sz)
                ret = true;
        }
    }

    return ret;        
}

bool MemTransferMsgdma::TransferFromTarget(void* dst, uintptr_t src, std::size_t sz)
{   
    bool ret = false;

    std::unique_lock<std::mutex> lock(_mutex);

    if(_file)
    {
        if(fseek(_file, src, SEEK_SET) == 0)
        {
            // Transfer in 4Mb chunks to avoid HPS saturation
            static constexpr std::size_t CHUNK_SIZE = 0x400000;

            const std::size_t num_transfers = sz / CHUNK_SIZE;

            size_t actual_read_size = 0;

            uint8_t* dst_byte = static_cast<uint8_t*>(dst);

            for(std::size_t i = 0; i < num_transfers; ++i)
            {
                actual_read_size += fread(dst_byte, 1, CHUNK_SIZE, _file);
                dst_byte += CHUNK_SIZE;
            }

            const std::size_t ramainder = sz % CHUNK_SIZE;

            if(ramainder)
                actual_read_size += fread(dst_byte, 1, ramainder, _file);

            if (actual_read_size == sz)
                ret = true;                
        }
    }

    return ret;
}            


std::shared_ptr<IMemTransfer> MemTransferCpu::Create(const uintptr_t base_addr_cpu, const uintptr_t base_addr_fpga, const std::size_t sz)
{
    return std::make_shared<MemTransferCpu>(base_addr_cpu, base_addr_fpga, sz);
}

MemTransferCpu::MemTransferCpu(const uintptr_t base_addr_cpu, const uintptr_t base_addr_fpga, const std::size_t sz):
    _base_addr_cpu{base_addr_cpu},
    _base_addr_fpga{base_addr_fpga},
    _sz_mapped{sz},
    _fd{-1},
    _ptr{nullptr} 
{
    _fd = open("/dev/mem", O_RDWR);

    if (_fd != -1)
    {
        _ptr = mmap(0, _sz_mapped, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, (uint64_t)_base_addr_cpu);

        if (_ptr == MAP_FAILED)
        {
            std::string error_msg{std::format("Failed to map buffer at 0x{:x}, size {}", _base_addr_cpu, _sz_mapped)};
            throw std::runtime_error(error_msg);
        }
    }
    else
    {
        std::string error_msg = "Failed to open /dev/mem";
        throw std::runtime_error(error_msg);
    }
}

MemTransferCpu::~MemTransferCpu()
{
    if(_ptr && _ptr != MAP_FAILED)
    {
        munmap(_ptr, _sz_mapped);
        _ptr = nullptr;
    }

    if(_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

bool MemTransferCpu::TransferToTarget(uintptr_t dst, void* src, std::size_t sz)
{
    bool ret = false;

    if(_ptr)
    {
        memcpy((uint8_t*)(_ptr) + (dst - _base_addr_fpga), src, sz);

        ret = true;
    }

    return ret;
}

bool MemTransferCpu::TransferFromTarget(void* dst, uintptr_t src, std::size_t sz)
{
    bool ret = false;

    if(_ptr)
    {
        memcpy(dst, (uint8_t*)(_ptr) + (src - _base_addr_fpga), sz);
        ret = true;
    }

    return ret;
}
}