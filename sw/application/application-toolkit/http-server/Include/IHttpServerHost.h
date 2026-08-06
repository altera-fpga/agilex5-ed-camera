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
#include <stdint.h>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>

class IWebSocketMessage
{
public:
    virtual ~IWebSocketMessage() {}
    virtual std::vector<std::shared_ptr<std::vector<uint8_t>>> GetData(bool& isText, bool& finalFrame) = 0;
};

class IWebSocketCallback;

class WebSocketSubProtocol
{
public:
    WebSocketSubProtocol() : _isBinary(false) {}
    std::string _stringName;
    bool _isBinary;
};

// The library creates these when a new WebSocket is created
class IWebSocketService
{
public:
    virtual void SetCallback(std::shared_ptr<IWebSocketCallback>& spCallback) = 0;
    virtual int GetVersion() = 0;
    virtual void GetHostname(char* buffer, int32_t& destinationLength) = 0;
    virtual std::string GetSubProtocolString() = 0;

    // Send data to the client via the web socket.
    virtual void SendToClient(IWebSocketMessage* pMessage) = 0;
};

struct THostResponse
{
    bool _fileNotFound = false;
    std::string _type;
    std::string _data;
    std::shared_ptr<std::vector<uint8_t>> _spDataBuffer;
};

class IHttpServerHost
{
public:
    // A new web socket connection has been established
    virtual bool WebSocketOpened(IWebSocketService* pWebSocket) = 0;

    // Return false if service is unavailable
    virtual bool NewClientConnectionStarted(const std::string& clientNetworkAddress) = 0;
    virtual void ClientConnectionEnded(const std::string& clientNetworkAddress) {}

    virtual bool ProcessGet(const std::string& url, const std::string& reqData, THostResponse& response)
    {
        return false;
    }

    virtual void ProcessPut(const std::string& clientIpAddress,
                            const std::string& rawUrl,
                            const std::string& dataType,
                            const uint8_t* pData,
                            size_t length,
                            THostResponse& response) {}
    virtual void ProcessPost(const std::string& clientIpAddress,
                             const std::string& rawUrl,
                             const std::string& dataType,
                             const uint8_t* pData,
                             size_t length,
                             THostResponse& response) {}
    virtual void ProcessPatch(const std::string& clientIpAddress,
                              const std::string& rawUrl,
                              const std::string& dataType,
                              const uint8_t* pData,
                              size_t length,
                              THostResponse& response) {}

    // If a web socket is created with multiple protocols options, select the favourite
    virtual int GetPreferredWebSocketProtocol(std::vector<WebSocketSubProtocol>& subProtocols)
    {
        // Choose the first by default. Override if you want to select one
        if (subProtocols.size() > 0)
            return 0;
        else
            return -1;
    }

    virtual ~IHttpServerHost(){};
};

// The application should derive a class from this and create one
// in reponse to a IHttpServerHost::WebSocketOpened call.
// It will be destroyed by the library when the web socket closes.
class IWebSocketCallback : public std::enable_shared_from_this<IWebSocketCallback>
{
public:
    IWebSocketCallback(IWebSocketService* pIWebSocket);
    virtual ~IWebSocketCallback();

    void SendToClient(IWebSocketMessage* pMessage);

    // You should derive from IWebSocketCallback and override this
    virtual void ReceiveWebSocketMessage(IWebSocketMessage* pMessage);
    virtual void Ready() = 0; // Don't send anything until you have been called with Ready
    virtual void Shutdown() = 0; // Called before destruction
    virtual bool CheckServiceStatus(std::string& service_unavailable_reason) { return true; }
    void SetShutdownCritSec(std::recursive_mutex* pCriticalSection);

protected:
    IWebSocketService*		_pIWebSocket = nullptr;
    int						_wsVersion = 0;
    std::string				_subProtocolString;
    std::recursive_mutex*	_pShutdownCritSec = nullptr;
};

class TextWebSocketMessage : public IWebSocketMessage
{
public:
    TextWebSocketMessage(const char* pMessage, size_t length = 0);
    TextWebSocketMessage(std::shared_ptr<std::string> spString);
    virtual ~TextWebSocketMessage();
    std::vector<std::shared_ptr<std::vector<uint8_t>>> GetData(bool& isText, bool& finalFrame) override;

private:
    std::vector<std::shared_ptr<std::vector<uint8_t>>> _data;
};

