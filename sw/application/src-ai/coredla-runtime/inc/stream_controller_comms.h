/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include <mutex>
#include <string>
#include <vector>
#include "mmd_wrapper.h"
#include "stream_controller_messages.h"

#include "stream_controller_IRemoteContext.h"

template <class T>
struct Payload : public T {
  void* GetPayload() { return this; }
  size_t GetSize() { return sizeof(*this); }
};

class StreamControllerComms : public StreamControllerIRemoteContext {
 public:
#ifdef AOCL_JTAG_PATH
  StreamControllerComms(const std::string& jtagPath);
#else
  StreamControllerComms();
#endif
  virtual ~StreamControllerComms() {}
  bool IsPresent();
  Payload<StatusMessagePayload> GetStatus();
  std::string GetStatusString(Payload<StatusMessagePayload>& statusPayload);
  bool ScheduleItems(std::vector<Payload<CoreDlaJobPayload>> items);
  bool ItemComplete();
  bool Ping();
  bool Initialize(uint32_t sourceBufferSize, uint32_t dropSourceBuffers, uint32_t numInferenceRequests);

  // ov::IRemoteContext
  virtual const std::string& get_device_name() const override;
  virtual const ov::AnyMap& get_property() const override;
  virtual ov::SoPtr<ov::IRemoteTensor> create_tensor(const ov::element::Type& type,
                                                      const ov::Shape& shape,
                                                      const ov::AnyMap& params = {}) override;
  virtual ov::SoPtr<ov::ITensor> create_host_tensor(const ov::element::Type type, const ov::Shape& shape) override;

  // StreamControllerIRemoteContext
  virtual bool UserMessage(uint32_t userMessageType, void* pPayload = nullptr, size_t size = 0, UserPayload* pUserPayloadResult = nullptr) override;

 private:
  StreamControllerComms(const StreamControllerComms&) = delete;
  StreamControllerComms(StreamControllerComms&) = delete;
  StreamControllerComms& operator=(const StreamControllerComms&) = delete;
 private:
  bool StatusMessageHandler(uint32_t payloadOffset);
  MessageType ReceiveMessage(UserPayload* pUserPayloadResultPayload = nullptr);
  bool SendMessage(MessageType, void* pPayload = nullptr, size_t size = 0);
  MmdWrapper _mmdWrapper;
  uint32_t _lastReceiveSequenceID = 0;
  uint32_t _sendSequenceID = 0;
  uint32_t _numBadMessages = 0;
  const int _streamControllerInstance = 0;
  Payload<StatusMessagePayload> _receivedStatusMessage;

 private:
  // ov::IRemoteContext
  std::string m_name;
  ov::AnyMap m_property;
  std::recursive_mutex _mutex;
};
