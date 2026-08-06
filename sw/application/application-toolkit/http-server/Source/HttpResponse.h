/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HttpStatusCode.h"
#include "Encoding.h"
#include "StringBuilder.h"
#include <memory>
#include <vector>
#include <filesystem>

using namespace OHttpServer;

namespace AtUtils { class IFileSystem; }

class ISocket;
class IHttpServerInternal;
class CachedFile;
class EmbeddedFile;

class IHttpResponse
{
public:
    virtual ~IHttpResponse() {}
    virtual void Close() = 0;
    virtual void Output(const std::string& text) = 0;
    virtual void Output(std::shared_ptr<std::vector<uint8_t>> spData) = 0;
    virtual void SetStatusCode(HttpStatusCode statusCode) = 0;
    virtual void SetStatusDescription(const char* text) = 0;
    virtual void SetContentType(const char* content_type) = 0;
    virtual std::shared_ptr<ISocket> GetSocket() = 0;
    virtual void ReturnFile(const std::filesystem::path& filename, std::string extension, std::string url) = 0;
    virtual void ReturnFile(std::shared_ptr<EmbeddedFile> spEmbeddedFile, std::string extension, std::string url) = 0;
    virtual void ReturnCache(std::shared_ptr<CachedFile> spCachedFile) = 0;
    virtual void ReturnString(std::string string, std::string extension) = 0;
    virtual void SetCacheSetting(bool cacheState) = 0;
    virtual void FileNotFound(std::string url) = 0;
    virtual void ServiceUnavailable(std::string url) = 0;
};

class HttpResponse : public IHttpResponse
{
public:
    HttpResponse(IHttpServerInternal* pHttpServer, std::shared_ptr<ISocket> spSocket,
                 std::shared_ptr<AtUtils::IFileSystem> spFileSystem);
    ~HttpResponse() override;
    void Close() override;
    void Output(const std::string& text) override;
    void Output(std::shared_ptr<std::vector<uint8_t>> spData) override;
    void SetStatusCode(HttpStatusCode statusCode) override;
    void SetStatusDescription(const char* text) override;
    void SetContentType(const char* content_type) override;
    static std::string GetStatusString(HttpStatusCode statusCode);
    std::shared_ptr<ISocket> GetSocket() override { return _spSocket; }
    void ReturnFile(const std::filesystem::path& filename, std::string extension, std::string url) override;
    void ReturnFile(std::shared_ptr<EmbeddedFile> spEmbeddedFile,
                    std::string extension, std::string url) override;
    void ReturnCache(std::shared_ptr<CachedFile> spCachedFile) override;
    void ReturnString(std::string string, std::string extension) override;
    void SetCacheSetting(bool cacheState) override;
    void FileNotFound(std::string url) override;
    void ServiceUnavailable(std::string url) override;

private:
    friend class WebSocketResponse;

    HttpStatusCode GetStatusCode() { return _statusCode; }
    std::string GetStatusDescription() { return _statusDescription; }
    void Reply(const std::string& reply);
    void CreateErrorPage(StringBuilder& stringBuilder,
                         std::string& message);
    std::shared_ptr<std::vector<uint8_t>> CssConstantInsertion(std::shared_ptr<std::vector<uint8_t>> spFileContents);
    std::string LookupCssConstant(std::string& constantName);

    IHttpServerInternal*     _pHttpServer;
    std::shared_ptr<ISocket> _spSocket;
    std::shared_ptr<AtUtils::IFileSystem> _spFileSystem;
    std::string              _contentType;
    HttpStatusCode           _statusCode;
    std::string              _statusDescription;
    Encoding                 _encoding;
    std::shared_ptr<std::vector<uint8_t>> _spData;
    bool                     _noCache;
    bool                     _closeSocketAfterResponse;
};

class IWebSocketResponse
{
public:
    virtual ~IWebSocketResponse() {}
    virtual void Close() = 0;
    virtual void SetStatusCode(HttpStatusCode statusCode) = 0;
    virtual void Accept(std::string accept_key) = 0;
    virtual void SetLegacyHandshakeHash(const uint8_t* pData, int length, const std::string& origin, const std::string& url) = 0;
    virtual void SetSubProtocol(std::string subProtocol) = 0;
};

class WebSocketResponse : public IWebSocketResponse
{
public:
    WebSocketResponse(IHttpServerInternal* pHttpServer, std::shared_ptr<ISocket> spSocket,
                      std::shared_ptr<AtUtils::IFileSystem> spFileSystem, int wsIndex);
    void Close() override;
    void SetStatusCode(HttpStatusCode statusCode) override;
    void Accept(std::string accept_key) override;
    void SetLegacyHandshakeHash(const uint8_t* pData, int length, const std::string& origin, const std::string& url) override;
    void SetSubProtocol(std::string subProtocol) override;

private:
    HttpResponse         _httpResponse;
    std::string          _acceptKey;
    int                  _wsIndex;
    bool                 _legacyProtocol;
    std::vector<uint8_t> _legacyHandshakeHash;
    std::string          _legacyOriginHandshake;
    std::string          _legacyLocationHandshake;
    std::string          _subProtocol;
};
