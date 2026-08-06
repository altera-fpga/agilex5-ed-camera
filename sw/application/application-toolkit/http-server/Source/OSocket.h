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
#include "SelectEvent.h"
#include <mutex>

#ifdef USE_OPENSSL
#include "openssl/ssl.h"
#endif

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#endif

class OSocket;
class HttpCommand;
class HttpServer;

enum TypeSocket
{
    BlockingSocket,
    NonBlockingSocket
};

class SocketDetails
{
public:
    std::string    _client_name;
    std::string    _ip_address;
    std::string    _socket;
    std::string    _type;
};

class ISocket
{
public:
    virtual ~ISocket() {}

    virtual void Close() = 0;
    virtual bool IsClosed() = 0;
    virtual std::string ReceiveLine() = 0;
    virtual bool ReceiveBytes(std::vector<uint8_t>& data, bool append, uint32_t timeout = INFINITE) = 0;
    virtual void FlushSendBuffer() = 0;
    virtual bool WaitForData(uint32_t timeout) = 0;
    virtual bool ReceiveHttpCommand(HttpCommand& command) = 0;
    virtual bool SendLine (std::string send, bool flush) = 0;
    virtual bool SendBytes(const std::vector<uint8_t>& data, bool flush) = 0;
    virtual bool SendBytes(const uint8_t* data, int start_offset, size_t length, bool flush) = 0;
    virtual AtUtils::Socket GetSocket() = 0;
    virtual void GetSocketDetails(SocketDetails& details) = 0;
    virtual std::string GetIpAddress() = 0;
    virtual void UpgradeToWebSocket() = 0;
    virtual void SetHttpSocket() = 0;
    virtual bool IsWebSocket() = 0;
    virtual std::string GetHostname() = 0;
};


class OSocket : public ISocket
{
public:
    OSocket(HttpServer* http_server,
            OSocket* s,
            sockaddr_in6* socket_address,
            TypeSocket type = BlockingSocket);
    OSocket(HttpServer* http_server,
            AtUtils::Socket new_socket,
            sockaddr_in6* socket_address,
            TypeSocket type = BlockingSocket);
    OSocket(HttpServer* http_server,
            TypeSocket type = BlockingSocket);
    virtual ~OSocket();
    OSocket(const OSocket&);
    OSocket& operator=(OSocket&);

    void Close() override;
    bool IsClosed() override;
    std::string ReceiveLine() override;
    bool ReceiveBytes(std::vector<uint8_t>& data, bool append, uint32_t timeout = INFINITE) override;
    void FlushSendBuffer() override;
    bool WaitForData(uint32_t timeout) override;
    bool ReceiveHttpCommand(HttpCommand& command) override;
    bool SendLine (std::string send, bool flush) override;
    bool SendBytes(const std::vector<uint8_t>& data, bool flush) override;
    bool SendBytes(const uint8_t* data, int start_offset, size_t length, bool flush) override;
    AtUtils::Socket GetSocket() override { return _socket_id; }
    void GetSocketDetails(SocketDetails& details) override;
    std::string GetIpAddress() override;
    void UpgradeToWebSocket() override;
    void SetHttpSocket() override;
    bool IsWebSocket() override;
    std::string GetHostname() override;

protected:
    friend class SocketServer;
    friend class SocketSelect;

    OSocket(const OSocket& o, const std::lock_guard<std::recursive_mutex>&);

    bool SendToBuffer(const uint8_t* data, size_t length);
    void InitialiseValidCharacterTable();

    virtual int32_t Send(const char* data, size_t length, int flags);
    virtual int32_t Receive(char* data, size_t length, int flags);

    std::recursive_mutex    _send_buffer_cs;
    std::vector<uint8_t>    _send_buffer;

public:
    HttpServer*     _http_server;
    AtUtils::Socket _socket_id;
    std::shared_ptr<SelectEvent> _shutdown_socket_fd;
    std::string     _type;
    bool            _is_web_socket;
    TypeSocket      _socket_type;
    bool            _package_data;
    sockaddr_in6    _socket_address;
    std::string     _ip_address_str;
    std::string     _hostname;
    mutable std::recursive_mutex _member_access;
    static bool     _validCharacterTableInitialised;
    static bool     _validCharacter[0x80];
};

#ifdef USE_OPENSSL

class OSecureSocket : public OSocket
{
public:
    ~OSecureSocket();
    void Close() override;

    OSecureSocket(const OSecureSocket& other) = delete;
    OSecureSocket& operator=(const OSecureSocket& other) = delete;
    OSecureSocket(OSecureSocket&& other) = delete;
    OSecureSocket& operator=(OSecureSocket&& other) = delete;

protected:
    friend class SocketServer;

    OSecureSocket(HttpServer* http_server,
                  Socket new_socket,
                  sockaddr_in6* socket_address,
                  TypeSocket type = BlockingSocket);

    int32_t Send(const char* data, size_t length, int flags) override;
    int32_t Receive(char* data, size_t length, int flags) override;

private:
    SSL* _secure_socket_layer = nullptr;
};

#endif // USE_OPENSSL

class SocketServer : protected AtUtils::Thread // protected so WorkerThread::ShutdownThread is not available to public
{
public:
    SocketServer(HttpServer* http_server,
                 AtUtils::Socket new_socket,
                 sockaddr_in6* socket_address);
    virtual ~SocketServer();

    void StartupWorkerThread();
    void ShutdownWorkerThread();

    // WorkerThread virtual functions
    virtual void RunThread();

    // For development
    void SetIndex(int index);
    uint32_t GetDeltaTime() { return (AtUtils::OS::Get()->GetTimeMS() - _start_time); }
    void Close();
    void GetSocketDetails(SocketDetails& details);
    std::shared_ptr<OSocket> GetSocket()    { return _socket; }

private:
    std::shared_ptr<OSocket>    _socket;
    HttpServer*    _http_server;
    AtUtils::Socket _socket_id;
    int            _index;
    uint32_t    _start_time;
    bool        _deleted;
};

class ListeningSocket : public OSocket
{
public:
    ListeningSocket(HttpServer* http_server,
                    int port, int connections,
                    TypeSocket type = BlockingSocket);
    bool Create(bool& access_error);
    SocketServer* Accept() ;

private:
    int            _port;
    int            _connections;
    HttpServer*    _http_server;
};

class SocketSelect
{
public:
    SocketSelect(OSocket const* const s1, OSocket const* const s2=NULL, TypeSocket type=BlockingSocket);
    bool Readable(OSocket const* const s);

private:
#ifdef WIN32
        fd_set fds_;
#else
        struct pollfd fds_[2];
#endif
};
