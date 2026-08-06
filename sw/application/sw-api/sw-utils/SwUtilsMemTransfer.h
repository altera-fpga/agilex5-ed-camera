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
#include <memory>
#include <map>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


namespace SwApi
{

class IMemTransfer
{
    public:
        virtual ~IMemTransfer() = default;
        virtual bool TransferToTarget(uintptr_t dst, void* src, std::size_t sz) = 0;
        virtual bool TransferFromTarget(void* dst, uintptr_t src, std::size_t sz) = 0;
};

using IMemTransferPtr = std::shared_ptr<IMemTransfer>;


class MemTransferMsgdma : public IMemTransfer
{
public:
    static std::shared_ptr<IMemTransfer> Create(const std::string &device_name);

    virtual ~MemTransferMsgdma();

    MemTransferMsgdma(const MemTransferMsgdma& other) = delete;
    MemTransferMsgdma &operator=(const MemTransferMsgdma& other) = delete;

    virtual bool TransferToTarget(uintptr_t dst, void* src, std::size_t sz) override;
    virtual bool TransferFromTarget(void* dst, uintptr_t src, std::size_t sz) override;         

private:
    MemTransferMsgdma(const std::string& device_name);

    FILE* _file = {nullptr};
    std::mutex _mutex;
};

class MemTransferCpu : public IMemTransfer
{
public:
    static std::shared_ptr<IMemTransfer> Create(const uintptr_t base_addr_cpu, const uintptr_t base_addr_fpga, const std::size_t sz = 0x20000000);


    MemTransferCpu(const uintptr_t base_addr_cpu, const uintptr_t base_addr_fpga, const std::size_t sz = 0x20000000);
    virtual ~MemTransferCpu();

    MemTransferCpu(const MemTransferCpu& other) = delete;
    MemTransferCpu &operator=(const MemTransferCpu& other) = delete;

    virtual bool TransferToTarget(uintptr_t dst, void* src, std::size_t sz) override;
    virtual bool TransferFromTarget(void* dst, uintptr_t src, std::size_t sz) override;

private:
    uintptr_t _base_addr_cpu;
    uintptr_t _base_addr_fpga;
    std::size_t _sz_mapped;
    int _fd;
    void* _ptr;
};

}