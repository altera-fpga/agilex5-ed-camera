/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "stream_controller_comms.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

// StreamControllerComms provides an interface to the Stream Controller
// microcode running in the NIOS-V

static const uint32_t messageReadyMagicNumber = 0x55225522;
static constexpr uint32_t mailboxRamSize = 0x200;

#ifdef AOCL_JTAG_PATH
StreamControllerComms::StreamControllerComms(const std::string& jtagPath): _mmdWrapper(jtagPath) {}
#else
StreamControllerComms::StreamControllerComms() {}
#endif

bool StreamControllerComms::IsPresent() {
  // Check there is an interface to the stream controller
  if (!_mmdWrapper.bIsStreamControllerValid(_streamControllerInstance)) {
    return false;
  }

  // Check that the stream controller responds
  bool isPresent = Ping();
  return isPresent;
}

// Query for the current status
Payload<StatusMessagePayload> StreamControllerComms::GetStatus() {
  std::lock_guard<std::recursive_mutex> lock(_mutex);

  if (SendMessage(MessageType_GetStatus)) {
    if (ReceiveMessage() == MessageType_Status) {
      return _receivedStatusMessage;
    }
  }

  return {};
}

// Schedule an inference request with the stream controller
bool StreamControllerComms::ScheduleItems(std::vector<Payload<CoreDlaJobPayload>> items) {
  std::lock_guard<std::recursive_mutex> lock(_mutex);

  bool status = true;

  for (auto& job : items) {
    bool thisJobStatus = false;

    if (SendMessage(MessageType_ScheduleItem, job.GetPayload(), job.GetSize())) {
      if (ReceiveMessage() == MessageType_NoOperation) {
        thisJobStatus = true;
      }
    }

    if (!thisJobStatus) {
      status = false;
    }
  }

  return status;
}

// Sinal a scheduled inference request has complete to the stream controller
bool StreamControllerComms::ItemComplete() {
  std::lock_guard<std::recursive_mutex> lock(_mutex);

  bool status = false;

  if (SendMessage(MessageType_ItemComplete)) {
    if (ReceiveMessage() == MessageType_NoOperation) {
      status = true;
    }
  }
  return status;
}

// Send a ping command to the stream controller and wait for a pong
// response.
bool StreamControllerComms::Ping() {
  std::lock_guard<std::recursive_mutex> lock(_mutex);

  if (SendMessage(MessageType_Ping)) {
    return (ReceiveMessage() == MessageType_Pong);
  }

  return false;
}

// Initialize and reset the stream controller
//
// sourceBufferSize:
//      The size of the MSGDMA buffers that the stream
//      controller will receive from the layout transform
// dropSourceBuffers:
//      How many source buffers to drop between each
//      processed one. 0 by default unless set in the configuration
//      by the app with DLIAPlugin::properties::streaming_drop_source_buffers.name()
// numInferenceRequest:
//      A constant value set in the executable network. The
//      stream controller will start executing once it has
//      received this number of inference requests from OpenVINO
bool StreamControllerComms::Initialize(uint32_t sourceBufferSize,
                                       uint32_t dropSourceBuffers,
                                       uint32_t numInferenceRequests) {
  std::lock_guard<std::recursive_mutex> lock(_mutex);

  Payload<InitializeStreamControllerPayload> initializePayload{};
  initializePayload._sourceBufferSize = sourceBufferSize;
  initializePayload._dropSourceBuffers = dropSourceBuffers;
  initializePayload._numInferenceRequests = numInferenceRequests;

  if (SendMessage(
          MessageType_InitializeStreamController, initializePayload.GetPayload(), initializePayload.GetSize())) {
    if (ReceiveMessage() == MessageType_NoOperation) {
      return true;
    }
  }

  return false;
}

// Receive a message from the stream controller by reading from the
// mailbox memory until the magic number is set to indicate a message is ready.
// Only the Status return message has a payload
MessageType StreamControllerComms::ReceiveMessage(UserPayload* pUserPayloadResult) {
  uint32_t receiveMessageOffset = mailboxRamSize / 2;
  MessageHeader* pReceiveMessage = nullptr;
  uint32_t messageReadyMagicNumberOffset = receiveMessageOffset;
  uint32_t payloadOffset = static_cast<uint32_t>(receiveMessageOffset + (size_t)&pReceiveMessage->_payload);
  uint32_t waitCount = 0;

  if(pUserPayloadResult)
  {
    pUserPayloadResult->_userMessageHeader._userMessageType = UserMessageType_Invalid;
    pUserPayloadResult->_userMessageHeader._userMessageLength = 0;
  }

  _mmdWrapper.MsgReceiveInterruptFromStreamController(_streamControllerInstance);

  while (true /*waitCount < 100*/) {
    MessageHeader messageHeader;
    _mmdWrapper.ReadFromStreamController(
        _streamControllerInstance, receiveMessageOffset, sizeof(messageHeader), &messageHeader);
    if (messageHeader._messageReadyMagicNumber == messageReadyMagicNumber) {
      MessageType messageType = static_cast<MessageType>(messageHeader._messageType);
      uint32_t sequenceId = messageHeader._sequenceID;

      bool ok = false;

      if (messageType == MessageType_Status) {
        ok = StatusMessageHandler(payloadOffset);
      } else if (messageType == MessageType_Pong) {
        ok = true;
      } else if (messageType == MessageType_UserPayloadResult) {
        if(pUserPayloadResult)
        {
          _mmdWrapper.ReadFromStreamController(_streamControllerInstance, payloadOffset, sizeof(UserPayloadHeader), &pUserPayloadResult->_userMessageHeader);
          if(pUserPayloadResult->_userMessageHeader._userMessageLength < STREAM_CONTROLLER_MAX_USER_DATA)
          {
            _mmdWrapper.ReadFromStreamController(_streamControllerInstance, payloadOffset+sizeof(UserPayloadHeader), pUserPayloadResult->_userMessageHeader._userMessageLength, &pUserPayloadResult->_userMessageData[0]);
            ok = true;
          }
        }
        else
        {
          ok = true;
        }
      }
      if (!ok) {
        _numBadMessages++;
      }

      _mmdWrapper.WriteToStreamController(
          _streamControllerInstance, messageReadyMagicNumberOffset, sizeof(sequenceId), &sequenceId);
      _lastReceiveSequenceID = sequenceId;
      return messageType;
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    waitCount++;
  }

  return MessageType_Invalid;
}

// Send a message to the stream controller by writing to the mailbox memory,
// and wait for the message to be received/processed
bool StreamControllerComms::SendMessage(MessageType messageType, void* pPayload, size_t payloadSize) {
  uint32_t sendMessageOffset = 0;
  MessageHeader* pSendMessage = nullptr;
  uint32_t messageReadyMagicNumberOffset = 0;
  uint32_t messageTypeOffset = static_cast<uint32_t>((size_t)&pSendMessage->_messageType);
  uint32_t sequenceIDOffset = static_cast<uint32_t>((size_t)&pSendMessage->_sequenceID);
  uint32_t payloadOffset = static_cast<uint32_t>((size_t)&pSendMessage->_payload);

  uint32_t uintMessageType = static_cast<uint32_t>(messageType);

  _mmdWrapper.WriteToStreamController(
      _streamControllerInstance, messageTypeOffset, sizeof(uintMessageType), &uintMessageType);
  _mmdWrapper.WriteToStreamController(
      _streamControllerInstance, sequenceIDOffset, sizeof(_sendSequenceID), &_sendSequenceID);

  if (payloadSize > 0) {
    _mmdWrapper.WriteToStreamController(_streamControllerInstance, payloadOffset, payloadSize, pPayload);
  }

  // Signal the message as ready
  _mmdWrapper.WriteToStreamController(_streamControllerInstance,
                                      messageReadyMagicNumberOffset,
                                      sizeof(messageReadyMagicNumber),
                                      &messageReadyMagicNumber);
  _mmdWrapper.MsgSendInterruptToStreamController(_streamControllerInstance);


  // Wait until the message has been processed by looking for the sequence ID
  // in the magic number position
  uint32_t waitCount = 0;
  while (true)/*(waitCount < 100)*/ {
    MessageHeader messageHeader;
    _mmdWrapper.ReadFromStreamController(
        _streamControllerInstance, sendMessageOffset, sizeof(messageHeader), &messageHeader);

    if (messageHeader._messageReadyMagicNumber == _sendSequenceID) {
      _sendSequenceID++;
      return true;
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    waitCount++;
  }

  return false;
}

// Read the status message payload
bool StreamControllerComms::StatusMessageHandler(uint32_t payloadOffset) {
  _mmdWrapper.ReadFromStreamController(
      _streamControllerInstance, payloadOffset, sizeof(_receivedStatusMessage), &_receivedStatusMessage);
  return true;
}

// Parse the status message payload into a string
std::string StreamControllerComms::GetStatusString(Payload<StatusMessagePayload>& statusPayload) {
  std::ostringstream stringStream;
  stringStream << static_cast<uint32_t>(statusPayload._status) << "," << statusPayload._statusLineNumber << ","
               << statusPayload._numReceivedSourceBuffers << "," << statusPayload._numScheduledInferences << ","
               << statusPayload._numExecutedJobs << "," << statusPayload._vfwLowResIsrCount << "," << statusPayload._vfwFullResIsrCount;
  return stringStream.str();
}

///////////////////////////////////////////////////////////////////////////////

const std::string& StreamControllerComms::get_device_name() const
{
  return m_name;
}

const ov::AnyMap& StreamControllerComms::get_property() const
{
  return m_property;
}

ov::SoPtr<ov::IRemoteTensor> StreamControllerComms::create_tensor(const ov::element::Type& type,
                                                      const ov::Shape& shape,
                                                      const ov::AnyMap& params)
{
  ov::SoPtr<ov::IRemoteTensor> empty;
  return empty;  
}

ov::SoPtr<ov::ITensor> StreamControllerComms::create_host_tensor(const ov::element::Type type, const ov::Shape& shape)
{
  ov::SoPtr<ov::ITensor> empty;
  return empty;
}

bool StreamControllerComms::UserMessage(uint32_t userMessageType, void* pPayload, size_t size, UserPayload* pUserPayloadResult)
{
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  UserPayload userPayload;
  userPayload._userMessageHeader._userMessageType = userMessageType;
  userPayload._userMessageHeader._userMessageLength = size;
  if(size > STREAM_CONTROLLER_MAX_USER_DATA)
  {
    return false;
  }
  memcpy(userPayload._userMessageData, pPayload, size);
  if (SendMessage(MessageType_UserPayload, &userPayload, sizeof(userPayload))) {
    MessageType recieved_message_type = ReceiveMessage(pUserPayloadResult);
    if(pUserPayloadResult)
    {
      if(recieved_message_type == MessageType_UserPayloadResult)
      {
        return true;
      }
    }
    else
    if (recieved_message_type == MessageType_NoOperation)
    {
      return true;
    }
  }
 
  return false;
}