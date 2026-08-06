/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "AtUtils.h"
#include "OSocket.h"
#include "IHttpServerHost.h"

class SocketReceiveThread;
class WebSocket;
class WebSocketCommand;

class SocketReceiveThread : public AtUtils::Thread
{
public:
    SocketReceiveThread(WebSocket* web_socket);
    virtual ~SocketReceiveThread();

    // Thread implementation
    virtual void RunThread();

    WebSocket* _web_socket;
};

class SendNegotiationCmd : public AtUtils::Message
{
public:
    SendNegotiationCmd(WebSocket* web_socket,
                       std::string version_str,
                       bool service_available);
    virtual void Execute();

private:
    WebSocket*	_web_socket;
    std::string	_message;
    bool		_service_available;
};

class DataSendRequestCmd : public AtUtils::Message
{
public:
    DataSendRequestCmd(WebSocket* pWebSocket, IWebSocketMessage* message);
    DataSendRequestCmd(const DataSendRequestCmd&) = delete;
    DataSendRequestCmd& operator=(const DataSendRequestCmd& other) = delete;
    virtual ~DataSendRequestCmd();
    virtual void Execute();

private:
    IWebSocketMessage*	_message;
    WebSocket* _pWebSocket;
};

class ReceivedDataCmd : public AtUtils::Message
{
public:
    ReceivedDataCmd(WebSocketCommand* received_data);
    ~ReceivedDataCmd();
    virtual void Execute();

    ReceivedDataCmd(const ReceivedDataCmd& other) = delete;
    ReceivedDataCmd &operator=(const ReceivedDataCmd& other) = delete;

private:
    WebSocketCommand*	_received_data;
};

class IWebSocket: public IWebSocketService
{
public:
    virtual ~IWebSocket() {}

    virtual size_t GetCommandQueueLength() = 0;
    virtual bool GetClientReadyForMore(bool wait) = 0;
    virtual void SetClientReadyForMore(bool state) = 0;
    virtual void AddReceiveCommand(WebSocketCommand* receive_command) = 0;
    virtual void WaitUntilClosed() = 0;
    virtual std::shared_ptr<ISocket> GetSocket() = 0;
    virtual void ReceiveWebSocketText(std::vector<std::shared_ptr<std::vector<uint8_t>>>& text) = 0;
    virtual void ReceiveWebSocketBinary(std::vector<std::shared_ptr<std::vector<uint8_t>>>& binary_data, bool final_frame) = 0;
    virtual void SendNegotiationMessage() = 0;
    virtual bool IsLegacy() = 0;
};

class WebSocket : public IWebSocket
{
    friend class SocketReceiveThread;
    friend class SendNegotiationCmd;

public:
    WebSocket(std::shared_ptr<ISocket> socket,
              int version,
              std::string sub_protocol_string,
              bool is_binary);
    virtual ~WebSocket();

    // IWebSocketInternal implementation
    size_t GetCommandQueueLength() override;
    bool GetClientReadyForMore(bool wait) override;
    void SetClientReadyForMore(bool state) override;
    void AddReceiveCommand(WebSocketCommand* receive_command) override;
    void WaitUntilClosed() override;
    std::shared_ptr<ISocket> GetSocket() override;
    void ReceiveWebSocketText(std::vector<std::shared_ptr<std::vector<uint8_t>>>& text) override;
    void ReceiveWebSocketBinary(std::vector<std::shared_ptr<std::vector<uint8_t>>>& binary_data, bool final_frame) override;
    void SendNegotiationMessage() override;
    bool IsLegacy() override { return (GetVersion() < 13); }

    // IWebSocketService implementation
    void SetCallback(std::shared_ptr<IWebSocketCallback>& spCallback) override;
    int GetVersion() override;
    void GetHostname(char* buffer, int32_t& destination_length) override;
    std::string GetSubProtocolString() override;
    void SendToClient(IWebSocketMessage* message) override;

protected:
    bool ShutdownWebSocket();
    void DoSendNegotiationMessage(std::string version_str, bool service_available);

private:
    std::shared_ptr<ISocket>				_socket;
    std::shared_ptr<SocketReceiveThread>	_receive_thread;
    std::shared_ptr<AtUtils::MessageQueue> _command_queue;
    bool						_shutdown;
    AtUtils::Event				_client_ready_for_more;
    AtUtils::Event				_is_closed;
    int							_version;
    std::recursive_mutex		_callback_cs;
    std::recursive_mutex		_callback_destructor_cs;
    std::shared_ptr<IWebSocketCallback> _callback;
    std::string					_sub_protocol_string;
    bool						_is_binary;
};

class CallbackWebSocketTextMessage : public IWebSocketMessage
{
public:
    CallbackWebSocketTextMessage(std::vector<std::shared_ptr<std::vector<uint8_t>>>& text)
    :	_text(text)
    ,	_final_frame(true)
    {
    }

    virtual std::vector<std::shared_ptr<std::vector<uint8_t>>> GetData(bool& is_text, bool& final_frame)
    {
        is_text = true;
        final_frame = _final_frame;
        return _text;
    }

private:
    std::vector<std::shared_ptr<std::vector<uint8_t>>>& _text;
    bool _final_frame;
};

class CallbackWebSocketBinaryMessage : public IWebSocketMessage
{
public:
    CallbackWebSocketBinaryMessage(std::vector<std::shared_ptr<std::vector<uint8_t>>>& data, bool final_frame)
    :	_data(data)
    ,	_final_frame(final_frame)
    {
    }

    virtual std::vector<std::shared_ptr<std::vector<uint8_t>>> GetData(bool& is_text, bool& final_frame)
    {
        is_text = false;
        final_frame = _final_frame;
        return _data;
    }

private:
    std::vector<std::shared_ptr<std::vector<uint8_t>>>& _data;
    bool _final_frame;
};

