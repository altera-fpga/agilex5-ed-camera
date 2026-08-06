/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "IHttpServerHost.h"
#include <iostream>
#include <functional>

class SocketServer;
class IHttpResponse;
class IHttpServerInternal;
class ISocket;

namespace AtUtils { class IFileSystem; }
namespace OHttpServer { class StringBuilder; }

class HttpCommand : public std::vector<std::string>
{
    using HostProcessFn = std::function<void(const std::string& ipAddress,
                                             const std::string& rawUrl,
                                             const std::vector<uint8_t>& theData,
                                             THostResponse& hostResponse)>;
public:
    HttpCommand(std::shared_ptr<ISocket> spSocket, IHttpServerInternal* pHttpServer,
                std::shared_ptr<AtUtils::IFileSystem> spFileSystem);
    ~HttpCommand();
    void Execute();
    static std::string ToBase64String(uint8_t* pData, int length);
    static std::string ToBase16String(uint8_t* pData, int length);

    void DumpCommand(std::ostream& os);

private:
    std::vector<WebSocketSubProtocol> GetSubProtocolRequests();
    void ReplaceControlChars(std::string& inString);
    void ProcessWebSocket(std::shared_ptr<IHttpResponse>& spResponse, std::string& url);
    void ProcessGet(std::shared_ptr<IHttpResponse>& spResponse);
    void ProcessPut(std::shared_ptr<IHttpResponse>& spResponse);
    void ProcessPost(std::shared_ptr<IHttpResponse>& spResponse);
    void ProcessPatch(std::shared_ptr<IHttpResponse>& spResponse);
    void ProcessPutPostAndPatch(const std::string& httpMethod,
                                std::shared_ptr<IHttpResponse>& spResponse,
                                HostProcessFn hostProcessFn);
    void AddProtocolRequest(std::vector<WebSocketSubProtocol>& protocols, std::string protocol_string);
    std::string ComputeWebSocketHandshakeSecurityHash09(std::string secWebSocketKey);
    std::string GetEntry(std::string entryName);

    std::shared_ptr<ISocket> _spSocket;
    IHttpServerInternal* _pHttpServer;
    std::shared_ptr<AtUtils::IFileSystem> _spFileSystem;
    std::string _originStr;
};
