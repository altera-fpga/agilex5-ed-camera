/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "HttpServer.h"
#include "HttpResponse.h"
#include "OSocket.h"
#include <iostream>

#ifdef USE_OPENSSL
#include "openssl/ssl.h"
#include "openssl/err.h"
#endif
#include <filesystem>

// Factory create and delete
IHttpServer* CreateHttpServer(IHttpServerHost* pHost,
                              int& port,
                              const std::filesystem::path& workingFolder,
                              const std::string& productName,
                              const char* sslPrivateKeyFilename,
                              const char* sslSignedCertificateFilename)
{
    if (port == -1)
    {
        if (sslPrivateKeyFilename == nullptr)
            port = 80;
        else
            port = 443;
    }

    HttpServer* pServer = new HttpServer(pHost,
                                         port,
                                         workingFolder,
                                         productName,
                                         sslPrivateKeyFilename,
                                         sslSignedCertificateFilename);
    if (pServer && pServer->Startup(0))
    {
        if (pServer->Startup(1))
            return pServer;
    }

    std::cout << "CreateHttpServer failed" << std::endl;

    delete pServer;
    return nullptr;
}

void DeleteHttpServer(IHttpServer*& pIHttepServer)
{
    HttpServer* pHttpServer = dynamic_cast<HttpServer*>(pIHttepServer);
    if (pHttpServer)
    {
        pHttpServer->Shutdown(0);
        pHttpServer->Shutdown(1);
        delete pHttpServer;
        pIHttepServer = 0;
    }
}

//////////

HttpServer::HttpServer(IHttpServerHost* pHost,
                       int& port,
                       const std::filesystem::path& workingFolder,
                       const std::string& productName,
                       const char* sslPrivateKeyFilename,
                       const char* sslSignedCertificateFilename)
:	Thread("HttpServer")
,	_pHost(pHost)
,	_port(port)
,	_useCache(true)
,	_productName(productName)
,	_sslPrivateKeyFilename(sslPrivateKeyFilename ? sslPrivateKeyFilename : "")
,	_sslCertificateFilename(sslSignedCertificateFilename ? sslSignedCertificateFilename : "")
,	_sslInitialized(false)
{
    _searchFolder = workingFolder;

#ifdef WIN32
    ::timeBeginPeriod(1);

    WSADATA info;
    if (WSAStartup(MAKEWORD(2, 0), &info))
    {
        throw "Could not start WSA";
    }
#endif

    _sslInitialized = InitOpenSSL();

    if (!_sslInitialized && (_port == 443))
        _port = 80;
}

#ifdef USE_OPENSSL
bool HttpServer::ShowSslErrors()
{
    BIO* bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);
    char* buf;
    size_t len = BIO_get_mem_data(bio, &buf);
    std::string errorString(buf, len);
    BIO_free(bio);

    ITrace("%s\n", errorString.c_str());
    return false;
}
#endif

// [Once]
// To create a CA and use it to sign a certificate:
// Create a key for the CA
// openssl genrsa -des3 -out ca_key.pem 2048

// Create a certificate for the CA from it's key
// openssl req -x509 -new -nodes -key ca_key.pem -sha256 -days 365 -out ca_certificate.pem

// [On each client]
// In windows: add ca_certificate.pem to Trusted Root Certificate Authorities

// [For each server]
// Create a key for the server
// openssl genrsa -out server_private_key.pem 2048

// Make a certificate request for the server:
// openssl req -new -key server_private_key.pem -out server_certificate_request.pem

// Sign the certificate request using the CA certificate and the CA key, outputting the server's signed certificate
// openssl x509 -req -in server_certificate_request.pem -CA ca_certificate.pem -CAkey ca_key.pem -CAcreateserial -out server_signed_certificate.pem -days 365 -sha256 -extfile v3.ext

// Keep the files server_private_key.pem and server_signed_certificate.pem on the individual server

// Contents of v3.ext - will need one for each server
//    thorityKeyIdentifier=keyid,issuer
//      basicConstraints=CA:FALSE
//      keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
//      subjectAltName = @alt_names
//      [alt_names]
//      DNS.1 = jmcgowan-MOBL
//      DNS.2 = localhost
//      IP.1 = 192.168.0.103
//      IP.2 = 127.0.0.1


// OpenSSL compile with OPENSSL_THREADS and OPENSSL_SYS_WINDOWS (windows) or
// just OPENSSL_THREADS for Linux
bool HttpServer::InitOpenSSL()
{
#ifdef USE_OPENSSL
    ITrace("HttpServer::InitOpenSSL()\n");
    if (_sslCertificateFilename.empty() || _sslPrivateKeyFilename.empty())
        return false;

    bool filesExist = true;

    std::filesystem::path sslPrivateKeyFilename = _sslPrivateKeyFilename;
    std::filesystem::path ssl_certificate_filename = _sslCertificateFilename;

    if (!std::filesystem::exists(sslPrivateKeyFilename))
    {
        filesExist = false;
        ITrace("SSL private key file '%s' not found\n", _sslPrivateKeyFilename.c_str());
    }

    if (!std::filesystem::exists(ssl_certificate_filename))
    {
        filesExist = false;
        ITrace("SSL signed certificate file '%s' not found\n", _sslCertificateFilename.c_str());
    }

    if (!filesExist)
        return false;

    SSL_library_init();
    //OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    _pSslContext = SSL_CTX_new(TLS_server_method());

    SSL_CTX_set_options(_pSslContext,
                        SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                        SSL_OP_NO_COMPRESSION |
                        SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
    SSL_CTX_set_min_proto_version(_pSslContext, TLS1_3_VERSION);
    SSL_CTX_set_ciphersuites(_pSslContext, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256");

    if (SSL_CTX_use_certificate_file(_pSslContext, _sslCertificateFilename.c_str(), SSL_FILETYPE_PEM) != 1)
        return ShowSslErrors();

    // Load private key
    if (SSL_CTX_use_PrivateKey_file(_pSslContext, _sslPrivateKeyFilename.c_str(), SSL_FILETYPE_PEM) != 1)
        return ShowSslErrors();

    return true;
#else
    // OpenSSL compiled out
    std::cout << "HttpServer not built with OpenSSL" << std::endl;
    return false;
#endif
}

HttpServer::~HttpServer()
{
    StopThread();

#ifdef USE_OPENSSL
    if (_pSslContext)
        SSL_CTX_free(_pSslContext);
#endif
}

static const int START_PHASE_ONE = 0;
static const int START_PHASE_TWO = 1;
static const int STOP_PHASE_ONE = 0;
static const int STOP_PHASE_TWO = 1;

bool HttpServer::Startup(int phase)
{
    bool result = true;

    if (phase == START_PHASE_TWO)
    {
        int n_connections = SOMAXCONN;
        _spListeningSocket = std::make_shared<ListeningSocket>(this, _port, n_connections);
        bool accessError = false;
        bool listeningSocketOK = _spListeningSocket->Create(accessError);
        if (!listeningSocketOK && (_port == 80) && accessError)
        {
            // Switch to try port 8080 if not running as root and
            // can't access port 80
            _port = 8080;
            _spListeningSocket.reset();
            accessError = false;
            _spListeningSocket = std::make_shared<ListeningSocket>(this, _port, n_connections);
            listeningSocketOK = _spListeningSocket->Create(accessError);
        }

        if (listeningSocketOK)
            result = StartThread();
        else
        {
            std::cout << "HttpServer::Startup failed to create listening socket" << std::endl;
            return false;
        }

    }

    return result;
}

bool HttpServer::Shutdown(int phase)
{
    bool result = true;

    if (phase == STOP_PHASE_ONE)
    {
        {
            std::lock_guard lock(_socketServersCS);

            // Kill the socket
            if (_spListeningSocket)
                _spListeningSocket->Close();

            // If we are shutting ourself down, then killing the sockets
            // will cause the threads to exit
            for (auto socketServer: _socketServers)
                socketServer->Close();
        }

        StopThread();
    }
    else if (phase == STOP_PHASE_TWO)
    {
        std::vector<SocketServer*> socketServers;
        std::vector<SocketServer*> zombieSocketServers;

        {
            std::lock_guard lock(_socketServersCS);
            socketServers = _socketServers;
            _socketServers.clear();

            zombieSocketServers = _zombieSocketServers;
            _zombieSocketServers.clear();
        }

        for (auto socketServer: socketServers)
            delete socketServer;

        for (auto zombieSocketServer: zombieSocketServers)
            delete zombieSocketServer;

        std::lock_guard lock(_serverCacheCS);
        _serverCache.clear();
    }

    return result;
}

void HttpServer::UseCache(bool state)
{
    std::lock_guard lock(_serverCacheCS);
    _useCache = state;
}

void HttpServer::UpdateCssConstant(const char* constantName, const char* constantValue)
{
    std::lock_guard lock(_serverCacheCS);
    std::string constantNameString(constantName);
    std::string constantValueString(constantValue);

    _cssConstants[constantNameString] = std::move(constantValueString);

    // Flush CSS files from cache
    auto i = _serverCache.begin();
    while (i != _serverCache.end())
    {
        std::string nameString = i->first.string();
        std::string ext = AtUtils::Right(nameString, 4);
        AtUtils::MakeLower(ext);
        auto this_i = i;
        i++;
        if (ext == ".css")
            _serverCache.erase(this_i);
    }
}

CssConstantsMap HttpServer::GetCssConstants()
{
    std::lock_guard lock(_serverCacheCS);
    CssConstantsMap cssConstantsCopy = _cssConstants;
    return cssConstantsCopy;
}

void HttpServer::LookupCssConstant(const std::string& constantName, std::string& constantValue)
{
    std::lock_guard lock(_serverCacheCS);
    AtUtils::Lookup<std::string, std::string>(_cssConstants, constantName, constantValue);
}

// HTTP POST is generally used to create new resources
void HttpServer::EnablePost(bool state)
{
    _postAllowed = state;
}

// HTTP PUT is generally used to update an existing resource
// Used by the Web API for queries, as you can use JSON in the payload,
// which is non-standard when used in GET
void HttpServer::EnablePut(bool state)
{
    _putAllowed = state;
}

// HTTP PATCH is generally used to partially update an existing resource
void HttpServer::EnablePatch(bool state)
{
    _patchAllowed = state;
}

void HttpServer::UseEmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd)
{
    _embeddedFiles = std::make_unique<EmbeddedFiles>(pBinaryBlobStart, pBinarayBlobEnd);
}

bool HttpServer::UsingEmbeddedFiles()
{
    return (_embeddedFiles != nullptr);
}

std::shared_ptr<EmbeddedFile> HttpServer::GetEmbeddedFile(std::string url)
{
    if (_embeddedFiles)
        return _embeddedFiles->GetFile(std::move(url));
    else
        return nullptr;
}

std::filesystem::path HttpServer::GetSearchFolder()
{
    std::lock_guard lock(_searchFoldersCS);
    return _searchFolder;
}

void HttpServer::RunThread()
{
    _threadRunning.Set();

    // Wait for connections
    while (true)
    {
        SocketServer* pNewSocketServer = _spListeningSocket->Accept();
        if (pNewSocketServer)
        {
            std::lock_guard lock(_socketServersCS);
            if (ClientConnectionsExceeded(pNewSocketServer))
            {
                // Too many connections
                delete pNewSocketServer, pNewSocketServer = 0;
            }
            else
            {
                AddSocketServer(pNewSocketServer);
                UpdateUI();
                pNewSocketServer->StartupWorkerThread();
            }
        }
        else
            break;
    }

    std::lock_guard lock(_socketServersCS);
    _spListeningSocket = nullptr;
}

void HttpServer::AddSocketServer(SocketServer* pNewSocketServer)
{
    std::lock_guard lock(_socketServersCS);
    _socketServers.push_back(pNewSocketServer);
}

bool HttpServer::ClientConnectionsExceeded(SocketServer* pNewSocketServer)
{
    std::lock_guard lock(_socketServersCS);
    return (_socketServers.size() >= 64);
}

std::shared_ptr<IHttpResponse> HttpServer::CreateHttpResponse(std::shared_ptr<ISocket> spSocket)
{
    return std::make_shared<HttpResponse>(this, std::move(spSocket), std::make_shared<AtUtils::FileSystem>());
}

std::shared_ptr<IWebSocketResponse> HttpServer::CreateWebSocketResponse(std::shared_ptr<ISocket> spSocket, int index)
{
    return std::make_shared<WebSocketResponse>(this, std::move(spSocket), std::make_shared<AtUtils::FileSystem>(), index);
}

std::shared_ptr<IWebSocket> HttpServer::CreateWebSocket(std::shared_ptr<ISocket> spSocket, int version,
                                                        std::string sub_protocol_string, bool is_binary)
{
    return std::make_shared<WebSocket>(std::move(spSocket), version, sub_protocol_string, is_binary);
}

// AtUtils::Remove from the list of active socket servers
void HttpServer::SocketServerClosed(SocketServer* pSocketServer, bool isDeleted)
{
    std::lock_guard lock(_socketServersCS);

    // Delete any zombies
    for (auto zombieSocketServer: _zombieSocketServers)
        delete zombieSocketServer;

    _zombieSocketServers.clear();

    for (size_t i = 0; i < _socketServers.size(); i++)
    {
        if (_socketServers[i] == pSocketServer)
        {
            pSocketServer->Close();

            // AtUtils::Remove element i
            _socketServers.erase(_socketServers.begin() + i);

            // If the socket was closed but the SocketServer was not
            // deleted, then it must be put in a zombie list
            // so it can be deleted at shutdown
            if (!isDeleted)
                _zombieSocketServers.push_back(pSocketServer);

            UpdateUI();
            return;
        }
    }
}

void HttpServer::UpdateUI()
{
}

#ifdef USE_OPENSSL
SslContextLock::Ptr HttpServer::GetSslContext()
{
    SslContextLock::Ptr spSslContext(new SslContextLock(_pSslContext, &_sslContextCS));
    return spSslContext;
}
#endif

std::shared_ptr<CachedFile> HttpServer::GetServerCachedFile(const std::filesystem::path& url)
{
    std::lock_guard lock(_serverCacheCS);
    if (_useCache)
    {
        auto i = _serverCache.find(url);
        if (i != _serverCache.end())
            return i->second;
    }

    return nullptr;
}

void HttpServer::AddToServerCache(std::shared_ptr<CachedFile> cached_file)
{
    std::lock_guard lock(_serverCacheCS);
    if (_useCache)
        _serverCache[cached_file->_url] = cached_file;
}

CachedFile::CachedFile(const char* url,
                       const char* contentType,
                       bool clientCache,
                       std::shared_ptr<std::vector<uint8_t>> spData)
:	_url(url)
,	_contentType(contentType)
,	_clientCache(clientCache)
,	_spData(std::move(spData))
{
}

#ifdef USE_OPENSSL
SslContextLock::SslContextLock(SSL_CTX* pSslContext, CritSec* pSslContextCS)
:	_pSslContext(pSslContext)
,	_sslContextCS(pSslContextCS)
{
    _sslContextCS->lock();
}

SslContextLock::~SslContextLock()
{
    _sslContextCS->unlock();
}
#endif
