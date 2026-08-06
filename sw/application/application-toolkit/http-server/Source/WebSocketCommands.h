/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <vector>
#include <memory>

class WebSocket;
class IWebSocket;
class ISocket;

enum WebSocketFrameType
{
    WST_Continuation = 0x0,
    WST_Text = 0x1,
    WST_Binary = 0x2,
    WST_ConnectionClose = 0x8,
    WST_Ping = 0x9,
    WST_Pong = 0xa
};

class ISocketDataStream
{
public:
    virtual ~ISocketDataStream() {}

    virtual bool Read(std::vector<uint8_t>& receive_buffer, bool peek = false, size_t num_bytes = 0) = 0;
    virtual IWebSocket* GetWebSocket() = 0;
    virtual std::shared_ptr<ISocket> GetSocket() = 0;
    virtual bool IsLegacy() = 0;
    virtual void Flush() = 0;
    virtual bool IsEmpty() = 0;
};

class SocketDataStream : public ISocketDataStream
{
public:
    SocketDataStream(IWebSocket* web_socket);

    bool Read(std::vector<uint8_t>& receive_buffer, bool peek = false, size_t num_bytes = 0) override;
    virtual IWebSocket* GetWebSocket() override { return _web_socket; }
    std::shared_ptr<ISocket> GetSocket() override { return _socket; }
    bool IsLegacy() override { return _is_legacy; }
    void Flush() override;
    bool IsEmpty() override;

private:

    IWebSocket* _web_socket;
    std::shared_ptr<ISocket> _socket;
    std::shared_ptr<std::vector<uint8_t>> _internal_buffer;
    size_t _next_index;
    bool _is_legacy;
};


class WebSocketCommand
{
public:
    WebSocketCommand(IWebSocket* socket);
    virtual ~WebSocketCommand();
    static WebSocketCommand* Receive(ISocketDataStream& data_stream);

    //bool Receive();
    virtual bool Send() = 0;

    virtual bool IsPing()     { return false; }
    virtual bool IsPong()     { return false; }
    virtual bool IsText()     { return false; }
    virtual bool IsBinary()   { return false; }
    IWebSocket* GetWebSocket() { return _web_socket; }
    virtual void ReceiveByWebSocket() {}
    bool IsValid()            { return _is_valid; }

protected:
    IWebSocket* _web_socket;
    std::shared_ptr<std::vector<uint8_t>>    _input_data;
    bool                _is_valid;
};

class PingWebSocketCommand : public WebSocketCommand
{
public:
    PingWebSocketCommand(ISocketDataStream& data_stream);
    virtual bool IsPing()    { return true; }
    virtual bool Send();
};

class PongWebSocketCommand : public WebSocketCommand
{
public:
    PongWebSocketCommand(IWebSocket* socket);
    virtual bool IsPong()    { return true; }
    virtual bool Send();
};

class TextWebSocketCommand : public WebSocketCommand
{
public:
    TextWebSocketCommand(IWebSocket* socket, const char* command, int length = -1);
    TextWebSocketCommand(IWebSocket* socket, std::shared_ptr<std::vector<uint8_t>>* data);
    TextWebSocketCommand(ISocketDataStream& data_stream);
    virtual bool IsText()    { return true; }
    virtual bool Send();
    virtual void ReceiveByWebSocket();

    std::shared_ptr<std::vector<uint8_t>> GetText(int index);
    int GetCount() { return (int)_texts.size(); }
    void SetBinaryEncodedText() { _binary_encoded_text = true; }
    void CheckUtf8();

protected:
    TextWebSocketCommand(IWebSocket* socket);
    std::shared_ptr<std::vector<uint8_t>> ConvertToUtf8(std::shared_ptr<std::vector<uint8_t>>& source);

    bool SendLegacy();
    std::vector<std::shared_ptr<std::vector<uint8_t>>> _texts;
    bool _binary_encoded_text; // For legacy web sockets
};

class LegacyWebSocketCommand : public TextWebSocketCommand
{
public:
    LegacyWebSocketCommand(IWebSocket* socket, std::shared_ptr<std::vector<uint8_t>>* data);
    LegacyWebSocketCommand(ISocketDataStream& data_stream);
    virtual bool IsText()    { return true; }
    virtual bool Send();
};

class BinaryWebSocketCommand : public WebSocketCommand
{
public:
    BinaryWebSocketCommand(IWebSocket* socket, int count);
    BinaryWebSocketCommand(IWebSocket* socket, std::shared_ptr<std::vector<uint8_t>>& data);
    BinaryWebSocketCommand(ISocketDataStream& data_stream);

    virtual bool IsBinary()    { return true; }
    virtual bool Send();
    virtual void ReceiveByWebSocket();

private:
    bool ReadFrame(ISocketDataStream& stream,
                    bool& final_frame,
                    WebSocketFrameType& frame_type);

    bool SendLegacy();
    std::vector<std::shared_ptr<std::vector<uint8_t>>> _data;
};
