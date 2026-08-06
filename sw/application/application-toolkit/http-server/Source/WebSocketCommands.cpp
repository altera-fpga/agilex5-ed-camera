/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WebSocketCommands.h"
#include "OSocket.h"
#include "WebSocket.h"
#include <assert.h>
#include "HttpCommand.h"
#include <string.h>

namespace
{

template<typename Ptr>
Ptr ThrowIfNull(Ptr p, const char* description)
{
    if (!p)
    {
        throw std::runtime_error("Null " + std::string{description} + " pointer");
    }

    return std::move(p);
}

struct PayloadParameters
{
    bool final;
    bool text_frame;
    bool connection_close;
    bool masked;
    size_t payload_length;
    char* str;
};

std::vector<uint8_t> CreateWebSocketPayload(const PayloadParameters& p)
{
    std::vector<uint8_t> data;

    if (p.payload_length < 126)
    {
        uint8_t b0 = (p.final ? 0x80 : 0x0) |
                     (p.text_frame ? 0x1 : 0x0) |
                     (p.connection_close ? 0x8 : 0x0);
        data.push_back(b0);

        uint8_t b1 = (p.masked ? 0x80 : 0x0) |
                     (p.payload_length & 0x7f);
        data.push_back(b1);
    }
    else if (p.payload_length < 0x10000)
    {
        // 16 bit size
        uint8_t b0 = (p.final ? 0x80 : 0x0) |
                     (p.text_frame ? 0x1 : 0x0) |
                     (p.connection_close ? 0x8 : 0x0);
        data.push_back(b0);

        uint8_t b1 = (p.masked ? 0x80 : 0x0) | 0x7e;
        data.push_back(b1);

        uint8_t b2 = (p.payload_length >> 8) & 0xff;
        uint8_t b3 = (p.payload_length >> 0) & 0xff;
        data.push_back(b2);
        data.push_back(b3);
    }
    else
    {
        // 64 bit size
        uint8_t b0 = (p.final ? 0x80 : 0x0) |
                     (p.text_frame ? 0x1 : 0x0) |
                     (p.connection_close ? 0x8 : 0x0);
        data.push_back(b0);

        uint8_t b1 = (p.masked ? 0x80 : 0x0) | 0x7f;
        data.push_back(b1);

        uint64_t ull_payload_length = p.payload_length;

        data.push_back((uint8_t)((ull_payload_length >> 56) & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 48) & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 40) & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 32) & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 24) & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 16) & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 8)  & 0xff));
        data.push_back((uint8_t)((ull_payload_length >> 0)  & 0xff));
    }

    for (size_t i = 0; i < p.payload_length; i++)
    {
        data.push_back(p.str[i]);
    }

    return data;
}

} // namespace

WebSocketCommand::WebSocketCommand(IWebSocket* web_socket)
:	_web_socket{ ThrowIfNull(web_socket, "WebSocketCommand websocket") }
,	_is_valid(false)
{
}

WebSocketCommand::~WebSocketCommand()
{
}

/////////////////////////

//WebSocketCommand* WebSocketCommand::Receive(WebSocket* web_socket)
WebSocketCommand* WebSocketCommand::Receive(ISocketDataStream& data_stream)
{
    WebSocketCommand* command = 0;

    if (data_stream.IsLegacy())
    {
        command = new LegacyWebSocketCommand(data_stream);
    }
    else
    {
        std::vector<uint8_t> header(2);

        // Peak the header
        if (!data_stream.Read(header, true))
            return 0;

        uint8_t b0 = header[0];
        bool final = ((b0 & 0x80) != 0);
        (void)final;

        int opcode = (b0 & 0xf);
        WebSocketFrameType frame_type = (WebSocketFrameType)opcode;

        // If we construct an object, the ownership of data is passed to the object
        if (frame_type == WST_ConnectionClose)
        {
            // Reflect the close message
            uint8_t close_response[2];
            close_response[0] = 0x88;
            close_response[1] = 0x00;
            data_stream.GetSocket()->SendBytes(close_response, 0, 2, true);
            data_stream.GetSocket()->Close();
        }
        else if (frame_type == WST_Continuation)
        {
        }
        else if (frame_type == WST_Text)
        {
            command = new TextWebSocketCommand(data_stream);
        }
        else if (frame_type == WST_Binary)
        {
            command = new BinaryWebSocketCommand(data_stream);
        }
        else if (frame_type == WST_Ping)
            command = new PingWebSocketCommand(data_stream);
    }

    if (command)
    {
        if (!command->IsValid())
            delete command, command = 0;
        else
            command->_web_socket = data_stream.GetWebSocket();
    }

    return command;
}

//////////////////////////////////

// Write
TextWebSocketCommand::TextWebSocketCommand(IWebSocket* socket, const char* command, int length)
:	WebSocketCommand(socket)
,	_binary_encoded_text(false)
{
    if (command)
    {
        if (length == -1)
        {
            // Already null terminated
            length = (int)strlen(command);
        }

        auto spByteArray = std::make_shared<std::vector<uint8_t>>(length);
        AtUtils::CopyMemoryBuffer(spByteArray->data(), spByteArray->size(), command, length);
        _texts.push_back(std::move(spByteArray));
    }

    CheckUtf8();
}

// Send
TextWebSocketCommand::TextWebSocketCommand(IWebSocket* socket, std::shared_ptr<std::vector<uint8_t>>* data)
:	WebSocketCommand(socket)
,	_binary_encoded_text(false)
{
    if (!data)
        return;

    _texts.push_back(*data);
    CheckUtf8();
}


void TextWebSocketCommand::CheckUtf8()
{
    for (size_t i = 0; i < _texts.size(); i++)
    {
        std::shared_ptr<std::vector<uint8_t>> text = _texts[i];
        size_t n_characters = text->size();
        for (size_t n = 0; n < n_characters; n++)
        {
            uint8_t ch = text->at(n);
            if ((ch & 0x80) > 0)
            {
                _texts[i] = ConvertToUtf8(text);
                break;
            }
        }
    }
}

// Legacy read
TextWebSocketCommand::TextWebSocketCommand(IWebSocket* socket)
: WebSocketCommand(socket)
, _binary_encoded_text(false)
{
}

std::shared_ptr<std::vector<uint8_t>> TextWebSocketCommand::ConvertToUtf8(std::shared_ptr<std::vector<uint8_t>>& source)
{
    std::shared_ptr<std::vector<uint8_t>> out(new std::vector<uint8_t>(0));
    size_t n_characters = source->size();
    for (size_t n = 0; n < n_characters; n++)
    {
        uint8_t ch = source->at(n);
        if ((ch & 0x80) > 0)
        {
            out->push_back(0xc0 + (ch >> 6));
            out->push_back(0x80 + (ch & 0x3f));
        }
        else
            out->push_back(ch);
    }

    return out;
}

// Receive
TextWebSocketCommand::TextWebSocketCommand(ISocketDataStream& data_stream)
:	WebSocketCommand(data_stream.GetWebSocket())
,	_binary_encoded_text(false)
{
    std::vector<uint8_t> header(2);
    if (!data_stream.Read(header))
        return;

    uint8_t b1 = header[1];
    bool masked = ((b1 & 0x80) != 0);
    (void)masked;
    int payload_length = b1 & 0x7f;

    if (payload_length == 126)
    {
        // Medium size
        std::vector<uint8_t> length_buffer(2);
        if (!data_stream.Read(length_buffer))
            return;

        uint16_t len_hi = length_buffer[0];
        uint16_t len_lo = length_buffer[1];
        payload_length = (len_hi << 8) | len_lo;
    }
    else if (payload_length == 127)
    {
        // Long text not supported yet
        throw std::runtime_error{"Not implemented"};
    }

    std::vector<uint8_t> mask(4);
    if (!data_stream.Read(mask))
        return;

    std::shared_ptr<std::vector<uint8_t>> str(new std::vector<uint8_t>(payload_length + 1));
    if (!data_stream.Read(*str, false, payload_length))
        return;

    uint8_t* buffer = &str->at(0);
    for (int i = 0; i < payload_length; i++)
        buffer[i] = buffer[i] ^ mask[i % 4];

    // Terminate string
    buffer[payload_length] = 0;
    _texts.push_back(std::move(str));

    _is_valid = true;
}

bool TextWebSocketCommand::Send()
{
    if (_texts.empty())
    {
        return false;
    }

    if (_web_socket->GetVersion() < 13)
        return SendLegacy();

    struct PayloadParameters parameters =
    {
        .final = true,
        .text_frame = true,
        .connection_close = false,
        .masked = false,
        .payload_length = _texts[0]->size(),
        .str = (char*) &_texts[0]->at(0),
    };

    std::vector<uint8_t> data = CreateWebSocketPayload(parameters);

    return _web_socket->GetSocket()->SendBytes(std::move(data), true);
}

bool TextWebSocketCommand::SendLegacy()
{
    std::shared_ptr<ISocket> socket = _web_socket->GetSocket();

    std::vector<uint8_t> data(1);
    data[0] = 0;
    if (!socket->SendBytes(data, true))
        return false;

    const char* true_text = "<t/>";
    const char* encoded_binary = "<b/>";

    // Prepend the actual encoding type so we can differentiate text and
    // binary messages in legacy protocol
    if (_binary_encoded_text)
        socket->SendBytes((uint8_t*)encoded_binary, 0, (int)strlen(encoded_binary), false);
    else
        socket->SendBytes((uint8_t*)true_text, 0, (int)strlen(true_text), false);

    if (!socket->SendBytes(*_texts[0], true))
        return false;

    data[0] = 0xff;
    if (!socket->SendBytes(data, true))
        return false;

    return true;
}

std::shared_ptr<std::vector<uint8_t>> TextWebSocketCommand::GetText(int index)
{
    return _texts.empty() ? nullptr : _texts[index];
}

void TextWebSocketCommand::ReceiveByWebSocket()
{
    GetWebSocket()->ReceiveWebSocketText(_texts);
}

/// LegacyWebSocketCommand ///////////////////////////////

// Send
LegacyWebSocketCommand::LegacyWebSocketCommand(IWebSocket* socket, std::shared_ptr<std::vector<uint8_t>>* data)
:	TextWebSocketCommand(socket, data)
{
}

// Receive
LegacyWebSocketCommand::LegacyWebSocketCommand(ISocketDataStream& data_stream)
:	TextWebSocketCommand(data_stream.GetWebSocket())
{
    std::vector<uint8_t> single_byte(1);
    if (!data_stream.Read(single_byte))
    {
        return;
    }

    if (single_byte[0] == 0)
    {
        std::shared_ptr<std::vector<uint8_t>> str(new std::vector<uint8_t>());

        while (true)
        {
            if (!data_stream.Read(single_byte))
                return;

            if (single_byte[0] == 0xff)
            {
                str->push_back(0);
                _texts.push_back(std::move(str));
                break;
            }
            else
            {
                str->push_back(single_byte[0]);
            }
        }
    }

    _is_valid = true;
}

bool LegacyWebSocketCommand::Send()
{
    return TextWebSocketCommand::Send();
}

/// BinaryWebSocketCommand ///////////////////////////////
BinaryWebSocketCommand::BinaryWebSocketCommand(IWebSocket* socket, int count)
:	WebSocketCommand(socket)
{
    std::shared_ptr<std::vector<uint8_t>> new_data(new std::vector<uint8_t>(count));
    _data.push_back(std::move(new_data));
}

// Send
BinaryWebSocketCommand::BinaryWebSocketCommand(IWebSocket* socket,
                                               std::shared_ptr<std::vector<uint8_t>>& data)
:	WebSocketCommand(socket)
{
    if (!data)
        return;

    _data.push_back(data);
}

// Receive
BinaryWebSocketCommand::BinaryWebSocketCommand(ISocketDataStream& data_stream)
:	WebSocketCommand(data_stream.GetWebSocket())
{
    // There could be multiple packets in the data
    //int data_size = data->size();
    //int packet_offset = 0;

    bool final_frame = false;
    WebSocketFrameType frame_type = WST_Continuation;

    if (!ReadFrame(data_stream, final_frame, frame_type))
        return;

    assert(frame_type == WST_Binary);

    // Read frames until we get a final frame
    while (!final_frame)
    {
        if (!ReadFrame(data_stream, final_frame, frame_type))
            return;

        if (frame_type != WST_Continuation)
            return;
    }

    _is_valid = true;
}

bool BinaryWebSocketCommand::ReadFrame(ISocketDataStream& stream,
                                       bool& final_frame,
                                       WebSocketFrameType& frame_type)
{
    std::vector<uint8_t> header(2);
    if (!stream.Read(header))
        return false;

    uint8_t b0 = header[0];
    final_frame = ((b0 & 0x80) != 0);

    int opcode = (b0 & 0xf);
    frame_type = (WebSocketFrameType)opcode;

    uint8_t b1 = header[1];
    bool masked = ((b1 & 0x80) != 0);
    (void)masked;

    uint64_t payload_length = b1 & 0x7f;

    if (payload_length == 126)
    {
        // Medium size
        std::vector<uint8_t> payload_length_buffer(2);
        if (!stream.Read(payload_length_buffer))
            return false;

        uint16_t len_hi = payload_length_buffer[0];
        uint16_t len_lo = payload_length_buffer[1];
        payload_length = (len_hi << 8) | len_lo;
    }
    else if (payload_length == 127)
    {
        std::vector<uint8_t> payload_length_buffer(8);
        if (!stream.Read(payload_length_buffer))
            return false;

        uint8_t* size_ptr = &payload_length_buffer[0];

        payload_length = ((uint64_t)size_ptr[0]) << 54 |
                         ((uint64_t)size_ptr[1]) << 48 |
                         ((uint64_t)size_ptr[2]) << 40 |
                         ((uint64_t)size_ptr[3]) << 32 |
                         ((uint64_t)size_ptr[4]) << 24 |
                         ((uint64_t)size_ptr[5]) << 16 |
                         ((uint64_t)size_ptr[6]) << 8 |
                         ((uint64_t)size_ptr[7]);
    }

    std::vector<uint8_t> mask(4);
    if (!stream.Read(mask))
        return false;

    // We limit to a maximum of 128MB
    if (payload_length > 0x8000000)
        return false;

    std::shared_ptr<std::vector<uint8_t>> data_segment(new std::vector<uint8_t>((int)payload_length));
    _data.push_back(data_segment);

    if (payload_length == 0)
    {
        return true;
    }

    if (!stream.Read(*data_segment))
        return false;

    // Apply the mask
    size_t n_bytes = data_segment->size();
    uint8_t* data = &data_segment->at(0);

    for (size_t i = 0; i < n_bytes; i++)
        data[i] = data[i] ^ mask[i % 4];

    return true;
}

bool BinaryWebSocketCommand::Send()
{
    if (_data.empty())
    {
        return false;
    }

    if (_web_socket->GetVersion() < 13)
        return SendLegacy();

    size_t payload_length = _data[0]->size();
    if (payload_length == 0)
        return false;

    uint8_t* data = &_data[0]->at(0);

    if (payload_length <= 0x7d)
    {
        // Short block
        std::vector<uint8_t> header(2);

        uint8_t binary_frame = 0x2;
        uint8_t final = 0x80;

        header[0] = final | binary_frame;
        header[1] = (payload_length & 0x7f);

        // Send header
        _web_socket->GetSocket()->SendBytes(header, false);

        // Now send the payload
        return _web_socket->GetSocket()->SendBytes(data, 0, payload_length, true);
    }
    else //if (payload_length <= 0xffff)
    {
        // Medium block
        int n_frames = (((int)payload_length - 1) / 512) + 1;

        if (n_frames == 1)
        {
            // Send one frame
            std::vector<uint8_t> header(4);

            uint8_t binary_frame = 0x2;
            uint8_t final = 0x80;

            // Need to split into multiple frames
            header[0] = final | binary_frame;
            header[1] = 0x7e;

            uint16_t len = (uint16_t)payload_length;
            header[2] = (len >> 8) & 0xff;
            header[3] = (len >> 0) & 0xff;

            _web_socket->GetSocket()->SendBytes(header, false);

            // Now send the payload
            return _web_socket->GetSocket()->SendBytes(data, 0, payload_length, true);
        }
        else
        {
            // Send fragmented, frames of size 512
            std::vector<uint8_t> header(4);

            uint8_t binary_frame = 0x2; // Opcode
            uint8_t final = 0x80;

            // Need to split into multiple frames
            header[0] = binary_frame;
            header[1] = 0x7e;

            uint16_t len = (uint16_t)512;
            header[2] = (len >> 8) & 0xff;
            header[3] = (len >> 0) & 0xff;

            for (int p = 0; p < (n_frames - 1); p++)
            {
                _web_socket->GetSocket()->SendBytes(header, false);

                // Now send the payload
                _web_socket->GetSocket()->SendBytes(data, 512 * p, 512, false);

                header[0] = 0x0; // No FIN bit or opcode for next frame
            }

            header[0] = final; // FIN bit, no opcode

            // Send final frame
            len = (uint16_t)(payload_length - (512 * (n_frames - 1)));
            if (len <= 0x7d)
            {
                // Short final frame
                header[1] = (len & 0x7f);
                _web_socket->GetSocket()->SendBytes(&header[0], 0, 2, false);
            }
            else
            {
                header[2] = (len >> 8) & 0xff;
                header[3] = (len >> 0) & 0xff;

                _web_socket->GetSocket()->SendBytes(header, false);
            }

            return _web_socket->GetSocket()->SendBytes(data, 512 * (n_frames - 1), len, true);
        }
    }

    return true;
}

bool BinaryWebSocketCommand::SendLegacy()
{
    int payload_length = (int)_data[0]->size();
    if (payload_length == 0)
        return false;

    uint8_t* data = _data[0]->data();

    std::string base64_str = HttpCommand::ToBase16String(data, payload_length);
    TextWebSocketCommand send_as_text(_web_socket, base64_str.c_str());
    send_as_text.SetBinaryEncodedText();
    return send_as_text.Send();
}

void BinaryWebSocketCommand::ReceiveByWebSocket()
{
    if (!_data.empty())
        GetWebSocket()->ReceiveWebSocketBinary(_data, true);
}

/// PongWebSocketCommand ///////////////////////////////
PongWebSocketCommand::PongWebSocketCommand(IWebSocket* socket)
:	WebSocketCommand(socket)
{
    _is_valid = true;
}

bool PongWebSocketCommand::Send()
{
    return false;
}


/// PingWebSocketCommand ///////////////////////////////

PingWebSocketCommand::PingWebSocketCommand(ISocketDataStream& data_stream)
:	WebSocketCommand(data_stream.GetWebSocket())
{
    std::vector<uint8_t> header(2);
    bool ignore = data_stream.Read(header);
    (void)ignore;

    _is_valid = true;
}

bool PingWebSocketCommand::Send()
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SocketDataStream::SocketDataStream(IWebSocket* web_socket)
:	_web_socket( ThrowIfNull(web_socket, "SocketDataStream websocket") )
,	_socket(web_socket->GetSocket())
,	_next_index(0)
,	_is_legacy(web_socket->IsLegacy())
{
    _internal_buffer.reset(new std::vector<uint8_t>());
}

bool SocketDataStream::Read(std::vector<uint8_t>& receive_buffer, bool peek, size_t num_bytes)
{
    size_t n_bytes = receive_buffer.size();

    if ((num_bytes > 0) && (num_bytes < n_bytes))
        n_bytes = num_bytes;

    size_t available_bytes = 0;

    while (true)
    {
        available_bytes = _internal_buffer->size() - _next_index;

        if (available_bytes >= n_bytes)
            break;

        // We need to read more data from the socket
        bool r = _socket->ReceiveBytes(*_internal_buffer, true, 2000);
        if (!r)
        {
            //ITrace(TL_MSG, "SocketDataStream::Read _socket->ReceiveBytes returned false\n");
            return false;
        }
    }

    uint8_t* source = &_internal_buffer->at(_next_index);
    uint8_t* destination = receive_buffer.data();
    AtUtils::CopyMemoryBuffer(destination, receive_buffer.size(), source, n_bytes);

    if (!peek)
        _next_index += n_bytes;

    return true;
}

void SocketDataStream::Flush()
{
    size_t buffer_size = _internal_buffer->size();

    if (buffer_size == 0)
        _next_index = 0;
    else
    {
        size_t n_remaining = buffer_size - _next_index;

        std::shared_ptr<std::vector<uint8_t>> flushed_buffer(new std::vector<uint8_t>(n_remaining));

        if (n_remaining > 0)
            AtUtils::CopyMemoryBuffer(flushed_buffer->data(), flushed_buffer->size(), &_internal_buffer->at(_next_index), n_remaining);

        _next_index = 0;
        _internal_buffer = std::move(flushed_buffer);
    }
}

bool SocketDataStream::IsEmpty()
{
    size_t buffer_size = _internal_buffer->size();
    size_t n_remaining = buffer_size - _next_index;
    return (n_remaining == 0);
}
