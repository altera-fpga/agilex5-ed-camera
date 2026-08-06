/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "IHttpServer.h"
#include <filesystem>
#include <functional>
#include "CommonTypes.h"
#include "AppToolkitIJson.h"
#include "IWebAPI.h"

class IHttpServer;

class WebServer : public IHttpServerHost
{

public:
    WebServer(int port = 80, std::string httpWorkingDirectory = "",
              std::string validateClientID = "",
              IWebAPI* pIWebAPI = nullptr);
    ~WebServer();

    static WebServer* Get() { return _pInstance; }

    bool Startup(StartupPhase phase);
    bool Shutdown(ShutdownPhase phase);

    // IHttpServerHost interface
    bool WebSocketOpened(IWebSocketService* pWebSocket) override;
    bool NewClientConnectionStarted(const std::string& clientNetworkAddress) override;
    void ClientConnectionEnded(const std::string& clientNetworkAddress) override;
    bool ProcessGet(const std::string &url, const std::string &reqData, THostResponse &response) override;
    void ProcessPut(const std::string &clientIpAddress,
                    const std::string &rawUrl,
                    const std::string &dataType,
                    const uint8_t *pData,
                    size_t length,
                    THostResponse &response) override;    
    void ProcessPost(const std::string &clientIpAddress,
                    const std::string &rawUrl,
                    const std::string &dataType,
                    const uint8_t *pData,
                    size_t length,
                    THostResponse &response) override;
    void ProcessPatch(const std::string &clientIpAddress,
                    const std::string &rawUrl,
                    const std::string &dataType,
                    const uint8_t *pData,
                    size_t length,
                    THostResponse &response) override;                    
    std::filesystem::path GetHttpWorkingDirectory() { return _httpWorkingDirectory; }
    std::string GetURL(bool useHostname);
    void AddUiSettings(AtUtils::IJsonObjectPtr& spOmnitekUI);
    bool UseEmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd);
    uint32_t ActiveConnectionCount() { return _activeConnectionCount; }

private:
    void SetCssConstants();

    IHttpServer*	_pHttpServer;
    std::filesystem::path _httpWorkingDirectory;
    int				_port;
    char*           _pBinaryBlobStart = nullptr;
    char*           _pBinarayBlobEnd = nullptr;
    bool            _validateClient = false;
    std::string     _validateClientString;
    IWebAPI*        _pIWebAPI = nullptr;
    static WebServer* _pInstance;
    uint32_t _activeConnectionCount = 0;
};

class WebServerNetworkAddress
{
public:
    std::string _stringAddress;
};

std::vector<WebServerNetworkAddress> GetNetworkAddresses();
bool IsNetworkConnected();