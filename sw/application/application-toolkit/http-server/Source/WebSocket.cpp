/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WebSocket.h"
#include "WebSocketCommands.h"
#include "WebSocketMessageIDs.h"

WebSocket::WebSocket(std::shared_ptr<ISocket> socket, int version,
                     std::string sub_protocol_string,
                     bool is_binary)
:	_socket(std::move(socket))
,	_shutdown(false)
,	_version(version)
,	_sub_protocol_string(std::move(sub_protocol_string))
,	_is_binary(is_binary)
{
    _client_ready_for_more.Set();
    _receive_thread = std::make_shared<SocketReceiveThread>(this);

    // Commands are received on _receive_thread and posted for
    // execution on the _command_queue thread.
    // Writes are posted for execution on the _command_queue.
    _command_queue = std::make_shared<AtUtils::MessageQueue>("WebSocket command queue");
    _receive_thread->StartThread();
}

WebSocket::~WebSocket()
{
    {
        std::shared_ptr<IWebSocketCallback> callback;
        {
            std::lock_guard lock(_callback_cs);
            callback = _callback;
            if (callback)
                callback->Shutdown();
            _callback.reset();
        }

        std::lock_guard destructor_lock(_callback_destructor_cs);
        callback.reset();
    }

    if (_command_queue)
        _command_queue->StopThread();
}

bool WebSocket::ShutdownWebSocket()
{
    bool closed_this_time = !_is_closed.IsSignalled();

    _is_closed.Set();
    return closed_this_time;
}

void WebSocket::WaitUntilClosed()
{
    _is_closed.Wait();
}

void WebSocket::SendNegotiationMessage()
{
    std::string version_str;
    bool service_available = false;

    std::string unavailable_reason;
    if (_callback && !_callback->CheckServiceStatus(unavailable_reason))
    {
        version_str = AtUtils::FormatString("<ServiceUnavailable reason='%s'/>", unavailable_reason.c_str());
    }
    else
    {
        // Tell the client the web socket version number
        version_str = AtUtils::FormatString("<WebSocketNegotiation version='%d' is_binary='%s'/>", _version,
                                            _is_binary ? "true" : "false");
        service_available = true;
    }

    // Make sure the message is sent on the command queue
    if (_command_queue)
        _command_queue->Add(new SendNegotiationCmd(this, std::move(version_str), service_available));
}

std::shared_ptr<ISocket> WebSocket::GetSocket()
{
    return _socket;
}

void WebSocket::DoSendNegotiationMessage(std::string version_str, bool service_available)
{
    TextWebSocketCommand cmd(this, version_str.c_str());
    cmd.Send();

    if (service_available)
    {
        // Don't do this before the negotiation has completed,
        // otherwise you could have two threads interleaving data
        // on the web socket, which will generate bad packets
        if (_callback)
            _callback->Ready();
    }
}

bool WebSocket::GetClientReadyForMore(bool wait)
{
    if (wait)
        _client_ready_for_more.IsSignalled(1000);

    return _client_ready_for_more.IsSignalled();
}

void WebSocket::SetClientReadyForMore(bool state)
{
    if (state)
        _client_ready_for_more.Set();
    else
        _client_ready_for_more.Reset();
}

void WebSocket::AddReceiveCommand(WebSocketCommand* receive_command)
{
    if (receive_command)
        _command_queue->Add(new ReceivedDataCmd(receive_command));
}

size_t WebSocket::GetCommandQueueLength()
{
    return _command_queue->GetNumMessages();
}

void WebSocket::SetCallback(std::shared_ptr<IWebSocketCallback>& spCallback)
{
    std::lock_guard lock(_callback_cs);
    _callback = spCallback;
    _callback->SetShutdownCritSec(&_callback_destructor_cs);
}

void WebSocket::ReceiveWebSocketText(std::vector<std::shared_ptr<std::vector<uint8_t>>>& data)
{
    std::lock_guard lock(_callback_cs);
    if (_callback)
    {
        CallbackWebSocketTextMessage message(data);
        _callback->ReceiveWebSocketMessage(&message);
    }
}

void WebSocket::ReceiveWebSocketBinary(std::vector<std::shared_ptr<std::vector<uint8_t>>>& binary_data,
                                       bool finalFrame)
{
    std::lock_guard lock(_callback_cs);
    if (_callback)
    {
        CallbackWebSocketBinaryMessage message(binary_data, finalFrame);
        _callback->ReceiveWebSocketMessage(&message);
    }
}

int WebSocket::GetVersion()
{
    return _version;
}

std::string WebSocket::GetSubProtocolString()
{
    return _sub_protocol_string;
}

void WebSocket::GetHostname(char* buffer, int32_t& destination_length)
{
    if (_socket)
    {
        std::string hostname = _socket->GetHostname();
        size_t string_length = hostname.length();
        if (buffer && (destination_length > 0))
        {
            size_t length = std::min(string_length, (size_t)(destination_length - 1));
            AtUtils::CopyMemoryBuffer(buffer, destination_length, hostname.data(), length);
            buffer[length] = 0;
        }
        else
            destination_length = (int)(string_length + 1);
    }
    else
        destination_length = 0;
}

// Send
void WebSocket::SendToClient(IWebSocketMessage* message)
{
    if (_command_queue)
        _command_queue->Add(new DataSendRequestCmd(this, message));
}

////////////

// The SocketReceiveThread is owner by the web_socket so can safely
// access WebSocket while SocketReceiveThread exists
SocketReceiveThread::SocketReceiveThread(WebSocket* web_socket)
:	Thread("SocketReceiveThread")
,	_web_socket(web_socket)
{
}

SocketReceiveThread::~SocketReceiveThread()
{
    StopThread();
}

void SocketReceiveThread::RunThread()
{
    _threadRunning.Set();
    bool done = false;

    SocketDataStream data_stream(_web_socket);

    while (!_shutdownEvent.IsSignalled())
    {
        data_stream.Flush();

        WebSocketCommand* receive_command = 0;

        // Will block until a message is receive or the socket is dropped
        if (data_stream.IsEmpty())
        {
            if (!_web_socket->GetSocket()->WaitForData(INFINITE))
            {
                // Socket has closed or shutdown
                done = true;
            }
        }

        if (!done)
            receive_command = WebSocketCommand::Receive(data_stream);

        // Has the socket closed?
        if (!receive_command)
        {
            //ITrace(TL_MSG, "SocketReceiveThread::RunThread\n");
            _web_socket->ShutdownWebSocket();
            return;
        }

        _web_socket->AddReceiveCommand(receive_command);
    }
}

////

SendNegotiationCmd::SendNegotiationCmd(WebSocket* web_socket,
                                       std::string version_str,
                                       bool service_available)
:	Message(SEND_NEGOTIATION_CMD)
,	_web_socket(web_socket)
,	_message(std::move(version_str))
,	_service_available(service_available)
{
}

void SendNegotiationCmd::Execute()
{
    _web_socket->DoSendNegotiationMessage(_message, _service_available);
}

////

DataSendRequestCmd::DataSendRequestCmd(WebSocket* pWebSocket, IWebSocketMessage* message)
:	Message(DATA_SEND_REQUEST_CMD)
,	_message(message)
,   _pWebSocket(pWebSocket)
{
}

DataSendRequestCmd::~DataSendRequestCmd()
{
    delete _message, _message = 0;
}

void DataSendRequestCmd::Execute()
{
    bool is_text = true;
    bool finalFrame = true;

    std::vector<std::shared_ptr<std::vector<uint8_t>>> data_array = _message->GetData(is_text, finalFrame);

    // We don't allow segmented binary web socket data for now
    assert(finalFrame);

    for (size_t i = 0; i < data_array.size(); i++)
    {
        std::shared_ptr<std::vector<uint8_t>> data = data_array[i];

        if (!data)
            break;

        if (is_text)
        {
            TextWebSocketCommand text_data(_pWebSocket, &data);
            text_data.Send();
        }
        else
        {
            finalFrame = (i == (data_array.size() - 1));

            // The binary data needs to be packed into WebSocket frames
            BinaryWebSocketCommand binary_data(_pWebSocket, data);
            binary_data.Send();
        }
    }
}

////


ReceivedDataCmd::ReceivedDataCmd(WebSocketCommand* received_data)
:	Message(RECEIVE_DATA_CMD)
,	_received_data(received_data)
{
}

ReceivedDataCmd::~ReceivedDataCmd()
{
    delete _received_data, _received_data = 0;
}

void ReceivedDataCmd::Execute()
{
    _received_data->ReceiveByWebSocket();
}

//////////////////


TextWebSocketMessage::TextWebSocketMessage(const char* message, size_t length)
{
    // Take a copy of the string
    if (length == 0)
        length = strlen(message);

    _data.push_back(std::make_shared<std::vector<uint8_t>>(length));

    if (length > 0)
        AtUtils::CopyMemoryBuffer(_data[0]->data(), _data[0]->size(), message, length);
}

TextWebSocketMessage::TextWebSocketMessage(std::shared_ptr<std::string> spString)
{
    if (spString)
    {
        // Take a copy of the string
        size_t length = spString->length();

        if (length > 0)
        {
            _data.push_back(std::make_shared<std::vector<uint8_t>>(length));
            AtUtils::CopyMemoryBuffer(_data[0]->data(), _data[0]->size(), spString->data(), length);
        }
    }
}

TextWebSocketMessage::~TextWebSocketMessage()
{
}

std::vector<std::shared_ptr<std::vector<uint8_t>>> TextWebSocketMessage::GetData(bool& is_text, bool& finalFrame)
{
    is_text = true;
    return _data;
}

