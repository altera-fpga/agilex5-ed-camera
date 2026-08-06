/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "OSocket.h"
#include <iostream>
#include "HttpCommand.h"
#include "HttpServer.h"

#ifdef WIN32
#include "Ws2tcpip.h"
#endif

static const bool package_send_data = false;

#ifdef __linux__
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#endif

#include <fstream>

bool OSocket::_validCharacterTableInitialised = false;
bool OSocket::_validCharacter[0x80] = {};

OSocket::OSocket(HttpServer* http_server, TypeSocket socket_type)
:	_http_server(http_server)
,	_socket_id(0)
,	_type("Socket")
,	_is_web_socket(false)
,	_socket_type(socket_type)
,	_package_data(package_send_data)
{
    _socket_address = {};

    // UDP: use SOCK_DGRAM instead of SOCK_STREAM
    _socket_id = socket(AF_INET6, SOCK_STREAM, 0);

    if (_socket_id == AtUtils::InvalidSocket)
    {
        throw "AtUtils::InvalidSocket";
    }

    _shutdown_socket_fd.reset(new SelectEvent());

    //BOOL option = TRUE;
    //int r = ::setsockopt(_socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&option, sizeof(option));
    int option = TRUE;
    int r = ::setsockopt(_socket_id, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    (void)r;
}

OSocket::OSocket(HttpServer* http_server,
                 AtUtils::Socket new_socket,
                 sockaddr_in6* socket_address,
                 TypeSocket socket_type)
:	_http_server(http_server)
,	_socket_id(new_socket)
,	_shutdown_socket_fd(new SelectEvent())
,	_type("HTTP")
,	_is_web_socket(false)
,	_socket_type(socket_type)
,	_package_data(package_send_data)
{
    if (socket_address)
    {
        _socket_address = *socket_address;

        socklen_t struct_size = (socket_address->sin6_family == AF_INET) ? sizeof(struct sockaddr_in)
                                                    : sizeof(struct sockaddr_in6);
        char ipAddressName[NI_MAXHOST]{};
        int r = getnameinfo((const struct sockaddr*)socket_address,
                            struct_size,
                            ipAddressName, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        (void)r; // unused

        _ip_address_str = ipAddressName;
        if (AtUtils::Left(_ip_address_str, 7) == "::ffff:")
        {
            // IPv4-mapped IPv6 address
            _ip_address_str = AtUtils::Mid(_ip_address_str, 7);
        }

        size_t percentPos = _ip_address_str.find("%");
        if (percentPos != std::string::npos)
            _ip_address_str = AtUtils::Left(_ip_address_str, percentPos);

        _hostname = "Unknown";
    }
    else
        _socket_address = {};

    int option = TRUE;
    int r = ::setsockopt(_socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&option, sizeof(option));
    (void)r;
}

OSocket::OSocket(HttpServer* http_server,
                 OSocket* s,
                 sockaddr_in6* socket_address,
                 TypeSocket socket_type)
:	_http_server(http_server)
,	_socket_id(s->_socket_id)
,	_shutdown_socket_fd(s->_shutdown_socket_fd)
,	_type("HTTP")
,	_is_web_socket(false)
,	_socket_type(socket_type)
,	_package_data(package_send_data)
{
    _socket_address = {};
}

OSocket::OSocket(const OSocket& o)
: OSocket{o, std::lock_guard{o._member_access}}
{
}

OSocket::OSocket(const OSocket& o, const std::lock_guard<std::recursive_mutex>&)
{
    std::lock_guard lock(o._member_access);

    _http_server = o._http_server;
    _socket_id = o._socket_id;
    _shutdown_socket_fd = o._shutdown_socket_fd;
    _socket_address = o._socket_address;
    _ip_address_str = o._ip_address_str;
    _hostname = o._hostname;
    _type = o._type;
    _is_web_socket = o._is_web_socket;
    _socket_type = o._socket_type;
    _package_data = o._package_data;
}

OSocket::~OSocket()
{
    Close();
}

OSocket& OSocket::operator=(OSocket& o)
{
    if (this != &o)
    {
        std::lock_guard lock(_member_access);
        std::lock_guard lock_o(o._member_access);

        _http_server = o._http_server;
        _socket_id = o._socket_id;
        _shutdown_socket_fd = o._shutdown_socket_fd;
        _socket_address = o._socket_address;
        _ip_address_str = o._ip_address_str;
        _hostname = o._hostname;
        _type = o._type;
        _is_web_socket = o._is_web_socket;
        _socket_type = o._socket_type;
        _package_data = o._package_data;
    }
    return *this;
}

void OSocket::Close()
{
    // This will unblock threads waiting on _socket_id (Window only)
    if (_socket_id != -1)
        AtUtils::ShutdownSocket(_socket_id);
    if (_shutdown_socket_fd)
        _shutdown_socket_fd->Set();
}

bool OSocket::IsClosed()
{
    return (_socket_id == AtUtils::InvalidSocket);
}

bool OSocket::WaitForData(uint32_t timeout)
{
    AtUtils::FDEventHandle handles[2];
    int n_handles = 0;
    handles[n_handles++] = (int)_socket_id;

#ifdef __linux__
    if (_shutdown_socket_fd)
        handles[n_handles++] = (int)*_shutdown_socket_fd;
#endif

    int n = WaitForMultipleFileObjects(n_handles, handles, false, timeout);

    if (n == 1)
        return false;

    if (n < 0)
        return false;

    return true;
}

bool OSocket::ReceiveBytes(std::vector<uint8_t>& data, bool append, uint32_t timeout)
{
    if (!append)
        data.clear();
    char buf[1025];

    bool lost = false;

    AtUtils::FDEventHandle handles[2];
    int n_handles = 0;
    handles[n_handles++] = (int)_socket_id;

#ifdef __linux__
    if (_shutdown_socket_fd)
        handles[n_handles++] = (int)*_shutdown_socket_fd;
#endif

    if (!WaitForData(timeout))
    {
        //ITrace(TL_MSG, "OSocket::ReceiveBytes WaitForData(%d) timed out\n", timeout);

        return false;
    }

    size_t total_received = 0;

    while (true)
    {
        u_long arg = 0;
        if (SOCKET_IOCTL(_socket_id, FIONREAD, &arg) != 0)
        {
            lost = true;
            break;
        }

        if (arg == 0)
        {
            if (total_received == 0)
            {
                lost = true;
            }
            break;
        }

        if (arg > 1024)
            arg = 1024;

        int32_t num_bytes_received = Receive(buf, arg, 0);
        if (num_bytes_received == AtUtils::SocketError)
            lost = true;

        if ((num_bytes_received == 0) && (total_received == 0))
            lost = true;

        if (num_bytes_received <= 0)
            break;

        for (int32_t i = 0; i < num_bytes_received; i++)
            data.push_back(buf[i]);

        total_received += static_cast<size_t>(num_bytes_received);
    }

    return !lost;
}

void OSocket::InitialiseValidCharacterTable()
{
    _validCharacter[0x9] = true;
    _validCharacter[0xa] = true;
    _validCharacter[0xd] = true;

    for (uint8_t ch = 0x20; ch <= 0x7e; ch++)
        _validCharacter[ch] = true;

    _validCharacterTableInitialised = true;
}

std::string OSocket::ReceiveLine()
{
    if (!_validCharacterTableInitialised)
        InitialiseValidCharacterTable();

    std::string return_str;

    while (true)
    {
        char r;

        switch(Receive(&r, 1, 0))
        {
        case 0: // not connected anymore;
            // ... but last line sent
            // might not end in \n,
            // so return ret anyway.
            return return_str;

        case AtUtils::SocketError:
            {
#ifdef WIN32
            int error = WSAGetLastError();
            if (error == WSAETIMEDOUT)
            {
                return return_str;
            }
#endif
            return "";
            }
        }

        auto ru = static_cast<uint8_t>(r);

        // Filter printable ASCII characters only
        if (ru < 0x80)
        {
            if (_validCharacter[ru])
                return_str += ru;
            else
                std::cout << "!!\n";
        }

        if (r == '\n')
            return return_str;
    }

    return return_str; // Can't get here
}

bool OSocket::SendToBuffer(const uint8_t* pData, size_t length)
{
    std::lock_guard lock(_member_access);
    if (_socket_id == AtUtils::InvalidSocket)
        return false;

    if (_package_data)
    {
        std::lock_guard lock(_send_buffer_cs);
        size_t previous_len = _send_buffer.size();
        _send_buffer.resize(previous_len + length);
        AtUtils::CopyMemoryBuffer(&_send_buffer[previous_len], length, pData, length);
        return true;
    }
    else
    {
        int32_t n = Send((const char*)pData, length, 0);
        return (n >= 0);
    }
}

int32_t OSocket::Send(const char* buffer, size_t length, int flags)
{
    int32_t number_of_bytes_sent = ::send(_socket_id, buffer, (int)length, flags | MSG_NOSIGNAL);
    return number_of_bytes_sent;
}

int32_t OSocket::Receive(char* buffer, size_t length, int flags)
{
    int32_t num_bytes_received = ::recv(_socket_id, buffer, (int)length, flags);
    return num_bytes_received;
}

void OSocket::FlushSendBuffer()
{
    std::lock_guard lock(_member_access);
    if (_package_data)
    {
        std::lock_guard lock(_send_buffer_cs);
        if (_socket_id != AtUtils::InvalidSocket)
        {
            int32_t n = Send((const char*)&_send_buffer[0], _send_buffer.size(), 0);
            if (n < 0)
            {
                std::ofstream save_state{};
                save_state.copyfmt(std::cout);
                std::cout << "::flush send 0x" << std::hex << _socket_id << " " <<
                          _ip_address_str << " failed " << errno << " " << std::endl;
                std::cout.copyfmt(save_state);
            }
        }
        _send_buffer.clear();
    }
}

bool OSocket::SendLine(std::string s, bool flush)
{
    s += "\r\n";
    if (!SendToBuffer((const uint8_t*)s.data(), s.length()))
        return false;

    if (flush)
        FlushSendBuffer();

    return true;
}

bool OSocket::SendBytes(const std::vector<uint8_t>& data, bool flush)
{
    if (!SendToBuffer(&data[0], data.size()))
        return false;

    if (flush)
        FlushSendBuffer();

    return true;
}

bool OSocket::SendBytes(const uint8_t* data, int start_offset, size_t length, bool flush)
{
    if (!SendToBuffer(data + start_offset, length))
        return false;

    if (flush)
        FlushSendBuffer();

    return true;
}

void OSocket::GetSocketDetails(SocketDetails& details)
{
    std::lock_guard lock(_member_access);
    details._client_name = _hostname;
    details._ip_address = _ip_address_str;
    details._socket = AtUtils::ToString((int)_socket_id);
    details._type = _type;
}

std::string OSocket::GetIpAddress()
{
    std::lock_guard lock(_member_access);
    std::string result = _ip_address_str;
    return result;
}

std::string OSocket::GetHostname()
{
    std::lock_guard lock(_member_access);
    if (_hostname == "Unknown")
    {
        char hostname[NI_MAXHOST]{};
        char server_info[NI_MAXSERV];

        int result = getnameinfo((const struct sockaddr*)&_socket_address,
                                 sizeof (_socket_address),
                                 hostname,
                                 NI_MAXHOST, server_info, NI_MAXSERV, NI_NUMERICSERV);
        (void)result;
        _hostname = hostname;
    }

    return _hostname;
}

void OSocket::SetHttpSocket()
{
    std::lock_guard lock(_member_access);
    _is_web_socket = false;
    _type = "HTTP";
}

void OSocket::UpgradeToWebSocket()
{
    std::lock_guard lock(_member_access);
    _is_web_socket = true;
    _type = "Web Socket";
}

bool OSocket::IsWebSocket()
{
    std::lock_guard lock(_member_access);
    return _is_web_socket;
}

///////////////////

#ifdef USE_OPENSSL

OSecureSocket::OSecureSocket(HttpServer* http_server,
                             AtUtils::Socket new_socket,
                             sockaddr_in6* socket_address,
                             TypeSocket socket_type)
:	OSocket(http_server, new_socket, socket_address, socket_type)
{
    int a = 0;
    SslContextLock::Ptr ssl_context_lock = _http_server->GetSslContext();
    _secure_socket_layer = SSL_new(*ssl_context_lock);

    if (_secure_socket_layer)
    {
        auto bio = BIO_new_socket(static_cast<int>(new_socket), BIO_NOCLOSE);
        SSL_set_bio(_secure_socket_layer, bio, bio);

        //SSL_set_fd(_secure_socket_layer, _socket_id);
        int r = SSL_accept(_secure_socket_layer);
        if (r != 0)
            http_server->ShowSslErrors();
    }
}

OSecureSocket::~OSecureSocket()
{
    Close();
    if (_secure_socket_layer)
    {
        int r = SSL_shutdown(_secure_socket_layer);
        SSL_free(_secure_socket_layer);
        _secure_socket_layer = nullptr;
    }
}

void OSecureSocket::Close()
{
    OSocket::Close();
}

int32_t OSecureSocket::Send(const char* data, size_t length, int flags)
{
    (void)flags; // Unused
    if (_secure_socket_layer)
    {
        int32_t number_of_bytes_sent = SSL_write(_secure_socket_layer, data, (int)length);
        return number_of_bytes_sent;
    }
    else
        return 0;
}

int32_t OSecureSocket::Receive(char* buffer, size_t length, int flags)
{
    (void)flags; // Unused
    if (_secure_socket_layer)
    {
        int32_t num_bytes_received = SSL_read(_secure_socket_layer, buffer, (int)length);
        return num_bytes_received;
    }
    else
        return 0;
}

#endif // USE_OPENSSL

///////////////////


ListeningSocket::ListeningSocket(HttpServer* http_server,
                                 int port, int connections,
                                 TypeSocket type)
:	OSocket(http_server, type)
,	_port(port)
,	_connections(connections)
,	_http_server(http_server)
{
}

bool ListeningSocket::Create(bool& access_error)
{
    std::lock_guard lock(_member_access);
    sockaddr_in6 clientAddress{};

    clientAddress.sin6_family = AF_INET6;
    clientAddress.sin6_port = htons(_port);
    clientAddress.sin6_addr = in6addr_any;
    _socket_id = socket(AF_INET6, SOCK_STREAM, 0);
    if (_socket_id == AtUtils::InvalidSocket)
    {
        return false;
    }

    int option = TRUE;
    int r = ::setsockopt(_socket_id, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    (void)r;

    if (_socket_type == NonBlockingSocket)
    {
        u_long arg = 1;
        int r = SOCKET_IOCTL(_socket_id, FIONBIO, &arg);
        (void)r;
    }

    // Bind the socket to the internet address
    if (bind(_socket_id, (const struct sockaddr*)&clientAddress, sizeof(clientAddress)) == AtUtils::SocketError)
    {
        access_error = (errno == EACCES);
        Close();
        return false;
    }

    listen(_socket_id, _connections);

    return true;
}

SocketServer* ListeningSocket::Accept()
{
    sockaddr_in6 socketAddress{};
    socklen_t length = sizeof(socketAddress);

    AtUtils::Socket new_sock = accept(_socket_id, (struct sockaddr*)&socketAddress, &length);
    if (new_sock == AtUtils::InvalidSocket)
        return 0;

    SocketServer* socket_server = new SocketServer(_http_server, new_sock, &socketAddress);
    return socket_server;
}

SocketServer::SocketServer(HttpServer* http_server,
                           AtUtils::Socket new_socket,
                           sockaddr_in6* socket_address)
:	Thread("SocketServer")
,	_http_server(http_server)
,   _socket_id(new_socket)
,	_index(0)
,	_start_time(0)
,	_deleted(false)
{
#ifdef USE_OPENSSL
    if (http_server->IsSecure())
        _socket = std::make_shared<OSecureSocket>(_http_server, new_socket, socket_address, BlockingSocket);
    else
        _socket = std::make_shared<OSocket>(_http_server, new_socket, socket_address, BlockingSocket);
#else
    _socket = std::make_shared<OSocket>(_http_server, new_socket, socket_address, BlockingSocket);
#endif
    _socket->SetHttpSocket();
}

SocketServer::~SocketServer()
{
    _socket_id = AtUtils::InvalidSocket;
    _deleted = true;
    ShutdownWorkerThread();
}

void SocketServer::ShutdownWorkerThread()
{
    // Close the socket so that ReceiveHttpCommand returns
    if (_socket)
        _socket->Close();

    StopThread();
    _socket.reset();
}

void SocketServer::SetIndex(int index)
{
    _index = index;
}

void SocketServer::StartupWorkerThread()
{
    _start_time = AtUtils::OS::Get()->GetTimeMS();
    StartThread();
}

void SocketServer::RunThread()
{
    _threadRunning.Set();

    HttpCommand command(_socket, _http_server, std::make_shared<AtUtils::FileSystem>());
    while (_socket && !_socket->IsClosed() && _socket->ReceiveHttpCommand(command))
        command.Execute();

    // If the thread exited because the destructor was called,
    // then _deleted is true. If the thread exited because the
    // socket was closed by the remote client, then _deleted
    // is false
    _http_server->SocketServerClosed(this, _deleted);
}

void SocketServer::Close()
{
    if (_socket)
        _socket->Close();
}

void SocketServer::GetSocketDetails(SocketDetails& details)
{
    if (_socket)
        _socket->GetSocketDetails(details);
}

//////////////////////////////

#ifdef WIN32
SocketSelect::SocketSelect(OSocket const* const s1, OSocket const* const s2, TypeSocket type)
{
    if (s1 && s2)
    {
        AtUtils::Socket fd_1 = 0;
        AtUtils::Socket fd_2 = 0;

        FD_ZERO(&fds_);
        fd_1 = const_cast<OSocket*>(s1)->_socket_id;
        FD_SET(fd_1, &fds_);

        if(s2)
        {
            fd_2 = const_cast<OSocket*>(s2)->_socket_id;
            FD_SET(fd_2, &fds_);
        }

        timeval tval;
        tval.tv_sec  = 0;
        tval.tv_usec = 1;

        timeval *ptval;
        if (type == NonBlockingSocket)
        {
            ptval = &tval;
        }
        else
        {
            ptval = 0;
        }

        int int_fd_1 = (int)fd_1;
        int int_fd_2 = (int)fd_2;
        int max_fd = (int_fd_1 > int_fd_2) ? int_fd_1 : int_fd_2;
        int nfds = max_fd + 1; // Ignored
        if (::select(nfds, &fds_, (fd_set*) 0, (fd_set*) 0, ptval) == AtUtils::SocketError)
        {
            //throw "Error in select";
        }
    }
}
#else
SocketSelect::SocketSelect(OSocket const* const s1, OSocket const* const s2, TypeSocket type)
: fds_{ { .fd = -1 }, { .fd = -1 } }
{
    if (s1)
    {
        fds_[0].fd = const_cast<OSocket*>(s1)->_socket_id;
        fds_[0].revents = 0;
        fds_[0].events = POLLIN;

        fds_[1].fd = -1;
        if (s2)
        {
            fds_[1].fd = const_cast<OSocket*>(s2)->_socket_id;
            fds_[1].revents = 0;
            fds_[1].events = POLLIN;
        }

        uint32_t timeout_ms = 1;
        if (type == BlockingSocket)
        {   // blocking
            timeout_ms = -1;	// wait forever
        }

        int nfds = s2? 2 : 1;
        if (::poll(fds_, nfds, timeout_ms) == AtUtils::SocketError)
        {
            // Error in select
        }
    }
}
#endif

bool SocketSelect::Readable(OSocket const* const s)
{
#ifdef WIN32
    if (FD_ISSET(s->_socket_id,&fds_))
        return true;
#else
    if ((fds_[0].fd == s->_socket_id) && (fds_[0].revents & POLLIN))
        return true;
    if ((fds_[1].fd == s->_socket_id) && (fds_[1].revents & POLLIN))
        return true;
#endif
    return false;
}

//////

static constexpr size_t maximumHttpLineLength = 512;
static constexpr size_t maximumHttpLines = 64;

bool OSocket::ReceiveHttpCommand(HttpCommand& command)
{
    command.clear();
    while (true)
    {
        std::string r = ReceiveLine();

        if (r.size() > maximumHttpLineLength)
        {
            std::cout << "Excessive HTTP line length\n";
            return false;
        }

        if (r == "")
            break;

        if (r == "\r\n")
            break;

        AtUtils::Replace(r, "\r\n", "");
        command.push_back(std::move(r));
    }

    size_t nItems = command.size();
    if (nItems > maximumHttpLines)
    {
        std::cout << "Excessive lines in HTTP command\n";
        command.clear();
    }

    return (command.size() > 0);
}

