#ifndef MMD_DEVICE_H_
#define MMD_DEVICE_H_

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
#include <memory>
#include <string>

#include "hapi_types.h"
#include "uio_device.h"
#include "HapiOCS.h"
#include "HapiCoreDLA.h"
#include "HapiOnChipMemII.h"
#include "HapiPio.h"
#include "SwUtilsMemTransfer.h"

#include "aocl_mmd.h"

// LOG ERRORS
#define MMD_ERR_LOGGING 1
#ifdef MMD_ERR_LOGGING
#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG_ERR(...)
#endif

class ScMsgComms
{
public:
  ScMsgComms(Hapi::HapiOnChipMemIIPtr spMsgMemDevice, Hapi::PioPtr spMsgSendInterruptPIODevice, std::shared_ptr<uio_device> spMsgReceiveInterruptUioDevice, Hapi::PioPtr spMsgReceiveInterruptPIODevice);
  ~ScMsgComms();

  void write(uint32_t addr, const uint32_t data);
  uint32_t read(uint32_t addr);

  void SendInterrupt();
  void ReceiveInterrupt();

private:
  ScMsgComms() = delete;
  ScMsgComms(const ScMsgComms& other) = delete;
  ScMsgComms& operator=(const ScMsgComms& other) = delete;

private:
  Hapi::HapiOnChipMemIIPtr _spMsgMemDevice;
  Hapi::PioPtr _spMsgSendInterruptPIODevice;
  std::shared_ptr<uio_device> _spMsgReceiveInterruptUioDevice;
  Hapi::PioPtr _spMsgReceiveInterruptPIODevice;
};
typedef std::shared_ptr<ScMsgComms> ScMsgCommsPtr;

class mmd_device {
public:
  mmd_device(std::string name, const int mmd_handle);
  ~mmd_device();

  bool bValid() { 
    return _spCoredlaDevice && _spCoredlaInterruptDevice && _spCoredlaInterruptDevice->bValid() && _spMemTransfer;
  };
  bool bStreamControllerValid() { return _spCoredlaDevice && _spScMsgComms; };
  int write_block(aocl_mmd_op_t op, int mmd_interface, const void *host_addr, size_t offset, size_t size);
  int read_block(aocl_mmd_op_t op, int mmd_interface, void *host_addr, size_t offset, size_t size);

  int stream_controller_msg_send_interrupt();
  int stream_controller_msg_receive_interrupt();

  int set_interrupt_handler(aocl_mmd_interrupt_handler_fn fn, void *user_data);

  std::string getName() {return _name;};
private:
  int32_t extract_index(const std::string name);

  mmd_device() = delete;
  mmd_device(mmd_device const&) = delete;
  void operator=(mmd_device const &) = delete;
  std::string _name;

  uint32_t* _scMailboxBaseAddress;
  ScMsgCommsPtr _spScMsgComms;
  Hapi::CoreDLAPtr _spCoredlaDevice;
  uio_device_ptr _spCoredlaInterruptDevice;
  SwApi::IMemTransferPtr _spMemTransfer;
  int            _mmd_handle;
};

typedef std::shared_ptr<mmd_device> mmd_device_ptr;

extern board_names mmd_get_devices(const int max_fpga_devices);

#endif // MMD_DEVICE_H_
