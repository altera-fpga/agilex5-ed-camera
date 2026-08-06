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
#include "WebSocket.h"
#include "IHttpServer.h"
#include "EmbeddedFile.h"
#include "HttpResponse.h"
#include <optional>

class CachedFile
{
public:
    CachedFile(const char* url,
               const char* contentType,
               bool clientCache,
               std::shared_ptr<std::vector<uint8_t>> spData);

    std::string        _url;
    std::string        _contentType;
    bool            _clientCache;
    std::shared_ptr<std::vector<uint8_t>> _spData;
};

#ifdef USE_OPENSSL
class SslContextLock
{
public:
    using Ptr = std::shared_ptr<SslContextLock>;

    SslContextLock(SSL_CTX* pSslContext, std::recursive_mutex* pSslContextCS);
    ~SslContextLock();

    operator SSL_CTX* () { return _pSslContext; }

private:
    SSL_CTX* _pSslContext;
    std::recursive_mutex* _pSslContextCS;
};
#endif

class IHttpServerInternal
{
public:
    virtual ~IHttpServerInternal() {}
    virtual std::shared_ptr<IHttpResponse> CreateHttpResponse(std::shared_ptr<ISocket> spSocket) = 0;
    virtual std::shared_ptr<IWebSocketResponse> CreateWebSocketResponse(std::shared_ptr<ISocket> spSocket, int index) = 0;
    virtual std::shared_ptr<IWebSocket> CreateWebSocket(std::shared_ptr<ISocket> spSocket, int version,
                                                        std::string sub_protocol_string, bool is_binary) = 0;
    virtual void SocketServerClosed(SocketServer* pSocketServer, bool isDeleted) = 0;
    virtual void UpdateUI() = 0;
    virtual bool ReceivedPutAllowed() = 0;
    virtual bool ReceivedPostAllowed() = 0;
    virtual bool ReceivedPatchAllowed() = 0;
    virtual IHttpServerHost*            GetHost() = 0;
    virtual std::filesystem::path       GetSearchFolder() = 0;
    virtual std::shared_ptr<CachedFile> GetServerCachedFile(const std::filesystem::path& url) = 0;
    virtual void                        AddToServerCache(std::shared_ptr<CachedFile> spCachedFile) = 0;
    virtual void                        LookupCssConstant(const std::string& constantName, std::string& constantValue) = 0;
    virtual std::string                 GetProductName() = 0;
#ifdef USE_OPENSSL
    virtual SslContextLock::Ptr         GetSslContext() = 0;
    virtual bool ShowSslErrors() = 0;
#endif
    virtual bool UsingEmbeddedFiles() = 0;
    virtual std::shared_ptr<EmbeddedFile> GetEmbeddedFile(std::string url) = 0;
};

struct PathHash
{
    std::size_t operator()(const std::optional<std::filesystem::path>& path) const
    {
        return path ? hash_value(path.value()) : 0;
    }
};

class HttpServer : public IHttpServer, public IHttpServerInternal, public AtUtils::Thread
{
    // Only these functions can create or delete the http server
    friend IHttpServer* CreateHttpServer(IHttpServerHost* pHost,
                                         int& port,
                                         const std::filesystem::path& workingFolder,
                                         const std::string& productName,
                                         const char* sslPrivateKeyFilename,
                                         const char* sslSignedCertificateFilename);
    friend void DeleteHttpServer(IHttpServer*& pHttpServer);

private:
    HttpServer(IHttpServerHost* pHost,
               int& port,
               const std::filesystem::path& working_folder,
               const std::string& product_name,
               const char* sslPrivateKeyFilename,
               const char* sslSignedCertificateFilename);
    virtual ~HttpServer();

public:
    // IHttpServer interface
    void UseCache(bool state) override;
    void UpdateCssConstant(const char* constantName, const char* constantValue) override;
    CssConstantsMap GetCssConstants() override;
    bool IsSecure() override { return _sslInitialized; }
    void EnablePut(bool state) override;
    void EnablePost(bool state) override;
    void EnablePatch(bool state) override;
    void UseEmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd) override;

    bool Startup(int phase);
    bool Shutdown(int phase);

    // AtUtils::Thread interface implementation
    void RunThread() override;

    // IHttpServerInternal interface
    std::shared_ptr<IHttpResponse> CreateHttpResponse(std::shared_ptr<ISocket> spSocket) override;
    std::shared_ptr<IWebSocketResponse> CreateWebSocketResponse(std::shared_ptr<ISocket> spSocket, int index) override;
    std::shared_ptr<IWebSocket> CreateWebSocket(std::shared_ptr<ISocket> spSocket, int version,
                                                std::string sub_protocol_string, bool is_binary) override;
    void SocketServerClosed(SocketServer* pSocketServer, bool isDeleted) override;
    void UpdateUI() override;
    bool ReceivedPutAllowed() override { return _putAllowed; }
    bool ReceivedPostAllowed() override { return _postAllowed; }
    bool ReceivedPatchAllowed() override { return _patchAllowed; }
    IHttpServerHost*            GetHost() override { return _pHost; }
    std::filesystem::path       GetSearchFolder() override;
    std::shared_ptr<CachedFile> GetServerCachedFile(const std::filesystem::path& url) override;
    void                        AddToServerCache(std::shared_ptr<CachedFile> spCachedFile) override;
    void                        LookupCssConstant(const std::string& constantName, std::string& constantValue) override;
    std::string                 GetProductName() override { return _productName; }
#ifdef USE_OPENSSL
    SslContextLock::Ptr         GetSslContext() override;
    bool ShowSslErrors() override;
#endif
    bool UsingEmbeddedFiles() override;
    std::shared_ptr<EmbeddedFile> GetEmbeddedFile(std::string url) override;

private:
    bool ClientConnectionsExceeded(SocketServer* pNewSocketServer);
    void AddSocketServer(SocketServer* pNewSocketServer);
    bool InitOpenSSL();

    IHttpServerHost*        _pHost;
    std::recursive_mutex    _socketServersCS;
    std::vector<SocketServer*>    _socketServers;
    std::vector<SocketServer*>    _zombieSocketServers;
    std::shared_ptr<ListeningSocket> _spListeningSocket;
    int&    _port;
    std::recursive_mutex    _searchFoldersCS;
    std::filesystem::path   _searchFolder;
    std::recursive_mutex    _serverCacheCS;
    std::unordered_map<std::filesystem::path, std::shared_ptr<CachedFile>, PathHash> _serverCache;
    bool                    _useCache;
    CssConstantsMap         _cssConstants;
    std::string             _productName;
#ifdef USE_OPENSSL
    std::recursive_mutex    _sslContextCS;
    SSL_CTX*                _pSslContext = nullptr;
#endif
    std::string             _sslPrivateKeyFilename;
    std::string              _sslCertificateFilename;
    bool                    _sslInitialized = false;
    bool                    _putAllowed = false;
    bool                    _postAllowed = false;
    bool                    _patchAllowed = false;
    std::unique_ptr<EmbeddedFiles> _embeddedFiles;
};
