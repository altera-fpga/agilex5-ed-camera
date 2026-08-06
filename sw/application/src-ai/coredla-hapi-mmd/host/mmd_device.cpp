/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/* ===- mmd_device.h  ------------------------------------------------- C++ -*-=== */
/*                                                                                 */
/*                         mmd device access functions                             */
/*                                                                                 */
/* ===-------------------------------------------------------------------------=== */
/*                                                                                 */
/* This file implements the functions used access the mmd device object            */
/*                                                                                 */
/* ===-------------------------------------------------------------------------=== */

#include "mmd_device.h"

// Defined names of the UIO Nodes
#define UIO_COREDLA_PREFIX "coredla"
#define STREAM_CONTROLLER_PREFIX "stream_controller"

// Defined name of the msgdma device
#define DMA_DEVICE_PREFIX "/dev/msgdma_userio"
#define UIO_DEVICE_PREFIX "uio"

ScMsgComms::ScMsgComms(Hapi::HapiOnChipMemIIPtr spMsgMemDevice, Hapi::PioPtr spMsgSendInterruptPIODevice, std::shared_ptr<uio_device> spMsgReceiveInterruptUioDevice, Hapi::PioPtr spMsgReceiveInterruptPIODevice)
    : _spMsgMemDevice(spMsgMemDevice)
    , _spMsgSendInterruptPIODevice(spMsgSendInterruptPIODevice)
    , _spMsgReceiveInterruptUioDevice(spMsgReceiveInterruptUioDevice)
    , _spMsgReceiveInterruptPIODevice(spMsgReceiveInterruptPIODevice)
{
    if(_spMsgReceiveInterruptPIODevice != NULL)
    {
        // Enable PIO Core interrupts
        intel_pio_set_irq_mask(_spMsgReceiveInterruptPIODevice->GetInstance(), 1);
    }
}

ScMsgComms::~ScMsgComms()
{
    if(_spMsgReceiveInterruptPIODevice != NULL)
    {
        // Disable PIO Core interrupts
        intel_pio_set_irq_mask(_spMsgReceiveInterruptPIODevice->GetInstance(), 0);
    }
}

void ScMsgComms::write(uint32_t addr, const uint32_t data)
{
    intel_on_chip_mem_ii_write(_spMsgMemDevice->GetInstance(), addr>>2U, *reinterpret_cast<const uint32_t*>(&data));
}

uint32_t ScMsgComms::read(uint32_t addr)
{
    uint32_t data;
    data = intel_on_chip_mem_ii_read(_spMsgMemDevice->GetInstance(), addr>>2U);
    return data;
}

void ScMsgComms::SendInterrupt()
{
    if(_spMsgSendInterruptPIODevice != NULL)
    {
        intel_pio_write(_spMsgSendInterruptPIODevice->GetInstance(), 1);
        intel_pio_write(_spMsgSendInterruptPIODevice->GetInstance(), 0);
    }
}

void ScMsgComms::ReceiveInterrupt()
{
    if(_spMsgReceiveInterruptUioDevice != NULL)
    {
        (void)_spMsgReceiveInterruptUioDevice->wait_irq();

        // Clear the interrupt
        intel_pio_set_edge_cap(_spMsgSendInterruptPIODevice->GetInstance(), 1);
    }
}


board_names mmd_get_devices(const int max_fpga_devices)
{
    board_names names;
    std::shared_ptr<Hapi::IHapi> spHapi = Hapi::IHapiOCS::Create();
    for(uint32_t index = 0; index < static_cast<uint32_t>(max_fpga_devices); index++)
    {
        Hapi::CoreDLAPtr spCoreDLA = spHapi->CreateByIndex<Hapi::CoreDLA>(index);
        if(spCoreDLA != nullptr)
        {
            #if 0
            Hapi::HapiItemIdentifier* cid = spCoreDLA->GetIpCoreItemIdentifier(Hapi::DriverFrameworkType::OCS);
            if(cid)
            {
                names.push_back(std::to_string(cid->_ocsID._capabilityUniqueID));
            }
            #else
            names.push_back(std::to_string(index));
            #endif
        }
    }

    return names;
}


/////////////////////////////////////////////////////////
mmd_device::mmd_device(std::string name, const int mmd_handle)
: _name(name), _mmd_handle(mmd_handle) {
    int32_t index = extract_index(_name);
    std::shared_ptr<Hapi::IHapi> spHapi = Hapi::IHapiOCS::Create();
    _spCoredlaDevice = spHapi->CreateByIndex<Hapi::CoreDLA>(index);
    _spCoredlaDevice->InitializeInstance();
    std::string uio_name = "uio"+std::to_string(2*index);

    _spCoredlaInterruptDevice = std::make_shared<uio_device>(uio_name, _mmd_handle, true);

    if( (index >= 0) && _spCoredlaDevice )
    {
        std::string dma_name(DMA_DEVICE_PREFIX);
        dma_name += std::to_string(index);

 #ifdef DMA
        _spMemTransfer = SwApi::MemTransferMsgdma::Create(dma_name);
 #else
        _spMemTransfer = SwApi::MemTransferCpu::Create(0x60000000, 0x00000000, 0x20000000);
 #endif /*DMA*/

        if(nullptr == _spMemTransfer)
            throw std::runtime_error("Failed to create MemTransfer object in mmd_device");

        Hapi::HapiOnChipMemIIPtr spMsgMemDevice = spHapi->CreateByUniqueID<Hapi::HapiOnChipMemII>(40);
        Hapi::PioPtr spMsgSendInterruptPIODevice = spHapi->CreateByUniqueID<Hapi::Pio>(41);
        std::string MsgReceiveInterruptUioName = "uio"+std::to_string(2*index + 1);
        std::shared_ptr<uio_device> spMsgReceiveInterruptUioDevice = std::make_shared<uio_device>(MsgReceiveInterruptUioName, 0, true, false);
        Hapi::PioPtr spMsgReceiveInterruptPIODevice = spHapi->CreateByUniqueID<Hapi::Pio>(40);
        _spScMsgComms = std::make_shared<ScMsgComms>(spMsgMemDevice, spMsgSendInterruptPIODevice, spMsgReceiveInterruptUioDevice, spMsgReceiveInterruptPIODevice);
    }
}

mmd_device::~mmd_device()
{
}

int mmd_device::read_block(aocl_mmd_op_t op, int mmd_interface, void *host_addr, size_t offset, size_t size)
{
    if( op ) {
        LOG_ERR("op not support : %s\n", __func__ );
        return FAILURE;
    }
    if( mmd_interface == HPS_MMD_MEMORY_HANDLE ) {
        bool read_ok = _spMemTransfer->TransferFromTarget(host_addr, reinterpret_cast<uintptr_t>(offset)+0x200000000, size);
        return read_ok ? SUCCESS : FAILURE;
    } else if( mmd_interface == HPS_MMD_COREDLA_CSR_HANDLE ) {
        if( nullptr == _spCoredlaDevice ) {
            return FAILURE;
        }
        if( nullptr == host_addr ) {
            return FAILURE;
        }
        // Support for only 32bit aligned transfers
        if( (offset % sizeof(uint32_t)) || (offset % sizeof(uint32_t)) ){
              return FAILURE;
        }
        *((uint32_t*)host_addr) = intel_coredla_csr_read(_spCoredlaDevice->GetInstance(), offset>>2U);
        return SUCCESS;
    } else if( mmd_interface == HPS_MMD_STREAM_CONTROLLER_HANDLE ) {
        if( nullptr == _spScMsgComms ) {
            return FAILURE;
        }
        if( nullptr == host_addr ) {
            return FAILURE;
        }
        // Support for only 32bit aligned transfers
        if( (offset % sizeof(uint32_t)) || (offset % sizeof(uint32_t)) ){
            return FAILURE;
        }
        for(uint32_t p = 0; p < size; p+=sizeof(uint32_t))
        {
            reinterpret_cast<uint32_t*>(host_addr)[p>>2] = _spScMsgComms->read(offset + p);
        }
        return SUCCESS;
    }

    return FAILURE;
}

int mmd_device::write_block(aocl_mmd_op_t op, int mmd_interface, const void *host_addr, size_t offset, size_t size)
{
     if( op ) {
        LOG_ERR("op not support : %s\n", __func__ );
        return FAILURE;
    }
    if( mmd_interface == HPS_MMD_MEMORY_HANDLE ) {
        bool write_ok = _spMemTransfer->TransferToTarget(reinterpret_cast<uintptr_t>(offset)+0x200000000, const_cast<void*>(host_addr), size);
        return write_ok ? SUCCESS : FAILURE;
    } else if ( mmd_interface == HPS_MMD_COREDLA_CSR_HANDLE ) {
        if( nullptr == _spCoredlaDevice ) {
            return FAILURE;
        }
        if( nullptr == host_addr ) {
            return FAILURE;
        }
        // Support for only 32bit aligned transfers
        if( (offset % sizeof(uint32_t)) || (offset % sizeof(uint32_t)) ){
            return FAILURE;
        }
        intel_coredla_csr_write(_spCoredlaDevice->GetInstance(), offset>>2U, *(uint32_t*)host_addr);
        return SUCCESS;
    } else if ( mmd_interface == HPS_MMD_STREAM_CONTROLLER_HANDLE ) {
        if( nullptr == _spScMsgComms ) {
            return FAILURE;
        }
        if( nullptr == host_addr ) {
            return FAILURE;
        }
        // Support for only 32bit aligned transfers
        if( (offset % sizeof(uint32_t)) || (offset % sizeof(uint32_t)) ){
            return FAILURE;
        }
        for(uint32_t p = 0; p < size; p+=sizeof(uint32_t))
        {
            _spScMsgComms->write((offset+p), reinterpret_cast<const uint32_t*>(host_addr)[p>>2]);
        }
        return SUCCESS;
    }
    return FAILURE;
}

int mmd_device::stream_controller_msg_send_interrupt()
{
    if( nullptr == _spScMsgComms ) {
        return FAILURE;
      }
      _spScMsgComms->SendInterrupt();
      return SUCCESS;
}

int mmd_device::stream_controller_msg_receive_interrupt()
{
    if( nullptr == _spScMsgComms ) {
        return FAILURE;
      }
      _spScMsgComms->ReceiveInterrupt();
      return SUCCESS;
}

int mmd_device::set_interrupt_handler(aocl_mmd_interrupt_handler_fn fn, void *user_data) {
    if( _spCoredlaInterruptDevice ) {
        return _spCoredlaInterruptDevice->set_interrupt_handler(fn, user_data);
    }
    return FAILURE;
}

// Returns the index of a uio device
// If index cannot be found then returns -1
int mmd_device::extract_index(const std::string name) {
  int32_t index = std::stoi(name, 0, 10);
  return index;
}
