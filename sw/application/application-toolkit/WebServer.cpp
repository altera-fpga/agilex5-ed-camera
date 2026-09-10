/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WebServer.h"
#include "WebSocketCommandProcessor.h"
#include "CommonApplicationBase.h"

#ifdef WIN32
#include <ws2tcpip.h>
#endif

#ifdef __linux__
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#endif

namespace
{

template<typename T, typename D>
T GetValueOrDefault(AtUtils::IJsonObjectPtr spObject, const char* name, D defaultValue)
{
    auto pValue = spObject->GetValue(name);
    return pValue ? pValue->GetValue<T>() : static_cast<T>(defaultValue);
}

} // namespace

WebServer* WebServer::_pInstance = nullptr;

WebServer::WebServer(int port,
                     std::string httpWorkingDirectory,
                     std::string validateClientID,
                     IWebAPI* pIWebAPI)
:	_pHttpServer(nullptr)
,	_port(port)
,   _pIWebAPI(pIWebAPI)
{
    _pInstance = this;

    if (!validateClientID.empty())
    {
        _validateClient = true;
        _validateClientString = std::move(validateClientID);
    }

    if (httpWorkingDirectory.empty())
    {
        _httpWorkingDirectory = std::filesystem::current_path();
        _httpWorkingDirectory /= "WebToolkit";
    }
    else if (AtUtils::Left(httpWorkingDirectory, 1) == "/")
    {
        _httpWorkingDirectory = httpWorkingDirectory;
    }
    else
    {
        _httpWorkingDirectory = std::filesystem::current_path();
        _httpWorkingDirectory /= httpWorkingDirectory;
    }

    std::string temp = _httpWorkingDirectory.string();
    std::cout << "HTTP working dir = " << temp << std::endl;
}

WebServer::~WebServer()
{
    _pInstance = nullptr;
}

bool WebServer::Startup(StartupPhase phase)
{
    if (phase == StartupPhase::TWO)
    {
        const char* sslPrivateKeyFilename = "";
        const char* sslSignedCertificateFilename = "";

        _pHttpServer = ::CreateHttpServer(this,
                                          _port,
                                          _httpWorkingDirectory,
                                          "Intel HTTPS/WS Server",
                                          sslPrivateKeyFilename,
                                          sslSignedCertificateFilename);
        if (_pHttpServer)
        {
            if ((_pBinaryBlobStart != nullptr) && (_pBinarayBlobEnd != nullptr))
                _pHttpServer->UseEmbeddedFiles(_pBinaryBlobStart, _pBinarayBlobEnd);

            _pHttpServer->UseCache(false);

            // Enable HTTP PATCH and POST for Web API
            _pHttpServer->EnablePut(true);
            _pHttpServer->EnablePost(true);
            _pHttpServer->EnablePatch(true);

            SetCssConstants();
            return true;
        }

        return false;
    }

    return true;
}

bool WebServer::Shutdown(ShutdownPhase phase)
{
    if (phase == ShutdownPhase::ONE)
    {
        return true;
    }
    else if (phase == ShutdownPhase::TWO)
    {
        ::DeleteHttpServer(_pHttpServer);
        return true;
    }

    return true;
}

bool WebServer::UseEmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd)
{
    if (_pHttpServer)
        return false;

    _pBinaryBlobStart = pBinaryBlobStart;
    _pBinarayBlobEnd = pBinarayBlobEnd;
    return true;
}

bool WebServer::WebSocketOpened(IWebSocketService* web_socket)
{
    // The client has created a new web socket.
    // We can get the protocol request from the IWebSocketService
    bool haveHandler = false;
    if(_pIWebAPI)
    {
        haveHandler = _pIWebAPI->WebSocketOpened(web_socket);
    }
    if(!haveHandler)
    {
        haveHandler = WebSocketCommandHandler::Create(web_socket);
    }
    return haveHandler;
}

bool WebServer::NewClientConnectionStarted(const std::string& clientNetworkAddress)
{
    std::cout << "UI client connected from IP address " << clientNetworkAddress << std::endl;
    _activeConnectionCount++;
    return true;
}

void WebServer::ClientConnectionEnded(const std::string& clientNetworkAddress)
{
    std::cout << "UI client disconnected from IP address " << clientNetworkAddress << std::endl;
    if (_activeConnectionCount > 0)
        _activeConnectionCount--;
}

bool WebServer::ProcessGet(const std::string& url, const std::string& reqData, THostResponse &response)
{
    if (_validateClient)
    {
        if ((url == "/index.html") or (url == "index.html") or (url == "/"))
        {
            bool valid = (reqData == _validateClientString);
            if (valid)
                return false; // Allow the system to find the url file

            response._fileNotFound = true;
            return true; // Don't file the url
        }
    }

    return false;
}

void WebServer::ProcessPut(const std::string &clientIpAddress,
                           const std::string &rawUrl,
                           const std::string &dataType,
                           const uint8_t *pData,
                           size_t length,
                           THostResponse &response)
{
    if (!_pIWebAPI)
    {
        response._fileNotFound = true;
        return;
    }

    // ProcessPost used for Web API
    std::string patchString(length, ' ');
    for (size_t i = 0; i < length; i++)
        patchString[i] = pData[i];

    auto spJson = AtUtils::IJson::Create(patchString);
    AtUtils::IJsonObjectPtr spObject;

    if (spJson)
        spObject = spJson->Parse();

    if (rawUrl == "/get_controls")
    {
        response._data = _pIWebAPI->GetControlsJSON();
        response._type = "application/json";
    }
    else if (rawUrl == "/get_button_id")
    {
        response._data = "";
        response._type = "text/plain";

        if (spObject)
        {
            auto panelName = GetValueOrDefault<std::string>(spObject, "panel_name", "");
            auto buttonLabel = GetValueOrDefault<std::string>(std::move(spObject), "button_label", "");
            uint32_t buttonID = _pIWebAPI->GetButtonID(panelName, buttonLabel);
            response._data = std::to_string(buttonID);
        }
    }
    else if (rawUrl == "/get_control_id")
    {
        response._data = "";
        response._type = "text/plain";

        if (spObject)
        {
            auto panelName = GetValueOrDefault<std::string>(spObject, "panel_name", "");
            auto controlLabel = GetValueOrDefault<std::string>(std::move(spObject), "control_label", "");
            uint32_t controlID = _pIWebAPI->GetControlID(panelName, controlLabel);
            response._data = std::to_string(controlID);
        }
    }    
    else if (rawUrl == "/get_control")
    {
        response._data = "";
        response._type = "application/json";
        if (spObject)
        {
            auto controlID = GetValueOrDefault<uint32_t>(std::move(spObject), "control_id", 0);
            response._data = _pIWebAPI->GetControlJSON(controlID);
        }
    }
}

void WebServer::ProcessPost(const std::string &clientIpAddress,
                            const std::string &rawUrl,
                            const std::string &dataType,
                            const uint8_t *pData,
                            size_t length,
                            THostResponse &response)
{
    // ProcessPost used for Web API
    response._fileNotFound = true;
}

void WebServer::ProcessPatch(const std::string &clientIpAddress,
                             const std::string &rawUrl,
                             const std::string &dataType,
                             const uint8_t *pData,
                             size_t length,
                             THostResponse &response)
{
    // ProcessPatch used for Web API
    if (!_pIWebAPI)
    {
        response._fileNotFound = true;
        return;
    }

    if (rawUrl == "/set_control")
    {
        std::string patchString(length, ' ');
        for (size_t i = 0; i < length; i++)
            patchString[i] = pData[i];

        _pIWebAPI->SetControlValue(patchString);
    }
}

std::string WebServer::GetURL(bool useHostname)
{
    std::string address;

    if (useHostname)
    {
        char hostName[0x100];
        int r = ::gethostname(hostName, 0x100);
        (void)r; // unused
        address = hostName;
    }
    else
    {
        auto addresses = GetNetworkAddresses();
        if (addresses.empty())
            return "";

        address = addresses[0]._stringAddress;
    }

    std::string url;
    if (_port == 80)
        url = AtUtils::FormatString("http://%s", address.c_str());
    else
        url = AtUtils::FormatString("http://%s:%d", address.c_str(), _port);

    return url;
}

void WebServer::AddUiSettings(AtUtils::IJsonObjectPtr& spParent)
{
    // This adds String IDs for ease of access from JavaScript
    AtUtils::IJsonObjectPtr spUiSettings = spParent->AddObject("UiSettings");

    CssConstantsMap css_constants = _pHttpServer->GetCssConstants();

    for (auto& constant : css_constants)
        spUiSettings->AddValue(constant.first.c_str(), constant.second);
}

void WebServer::SetCssConstants()
{
    if (!_pHttpServer)
        return;

    const char* font_family = "'GT Walsheim Regular'";
    static const char* altera_light_blue = "#0054ae";  // altera code blue
    static const char* altera_dark_blue = "#00377c";   // altera mid blue
    static const char* altera_background = "#ffffff";
    static const char* altera_child_window_background = "#f0f0f0";
    static const char* altera_text_colour = "#505050";
    static const char* altera_inactive_text_colour = "#909090";

    static const char* unused = "#ff0000";

    _pHttpServer->UpdateCssConstant("altera_light_blue", altera_light_blue);
    _pHttpServer->UpdateCssConstant("altera_dark_blue", altera_dark_blue);
    _pHttpServer->UpdateCssConstant("altera_background", altera_background);
    _pHttpServer->UpdateCssConstant("altera_child_window_background", altera_child_window_background);
    _pHttpServer->UpdateCssConstant("altera_text_colour", altera_text_colour);
    _pHttpServer->UpdateCssConstant("alt_white_text_colour", "#ffffff");

    _pHttpServer->UpdateCssConstant("background_medium", unused);
    _pHttpServer->UpdateCssConstant("text_colour", altera_text_colour);
    _pHttpServer->UpdateCssConstant("font_family", font_family);
    _pHttpServer->UpdateCssConstant("background_low", unused);
    _pHttpServer->UpdateCssConstant("background_high", altera_background);
    _pHttpServer->UpdateCssConstant("background_hover", altera_dark_blue);
    _pHttpServer->UpdateCssConstant("text_inactive_colour", altera_inactive_text_colour);
    _pHttpServer->UpdateCssConstant("border_colour", altera_text_colour);
    _pHttpServer->UpdateCssConstant("window_unselected", altera_child_window_background);
    _pHttpServer->UpdateCssConstant("window_selected", altera_child_window_background);
    _pHttpServer->UpdateCssConstant("window_selected_highlight", altera_dark_blue);
    _pHttpServer->UpdateCssConstant("window_header_hover", altera_dark_blue);
    _pHttpServer->UpdateCssConstant("window_border", "#e0e0e0");
    _pHttpServer->UpdateCssConstant("clicked_colour", "#0062aa");
    _pHttpServer->UpdateCssConstant("mouse_over_colour", "#0078a7");
}

static std::recursive_mutex _lockGetIpAddress;

#ifdef __linux__

bool CheckNIC(const std::string& nicName)
{
    bool isRunning = false;
    ifreq request{};
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFD >= 0)
    {
        ::strncpy(request.ifr_name, nicName.c_str(), IFNAMSIZ - 1);
        if (::ioctl(socketFD, SIOCGIFFLAGS, &request) >= 0)
            isRunning = (request.ifr_flags & IFF_UP);

        ::close(socketFD);
    }

    return isRunning;
}


std::vector<WebServerNetworkAddress> GetNetworkAddresses()
{
    std::lock_guard lock(_lockGetIpAddress);
    std::vector<WebServerNetworkAddress> addresses;
    ifaddrs* pAddressInfo = nullptr;
    char hostName[NI_MAXHOST]{};

    int r = ::getifaddrs(&pAddressInfo);

    if (r == 0)
    {
        for (ifaddrs* i = pAddressInfo; i != nullptr; i = i->ifa_next)
        {
            if (i->ifa_addr)
            {
                std::string nicName(i->ifa_name);
                int family = i->ifa_addr->sa_family;

                if (family == AF_INET || family == AF_INET6)
                {
                    if (CheckNIC(nicName))
                    {
                        WebServerNetworkAddress address{};

                        socklen_t struct_size = (family == AF_INET) ? sizeof(struct sockaddr_in)
                                                                    : sizeof(struct sockaddr_in6);
                        r = getnameinfo(i->ifa_addr,
                                        struct_size,
                                        hostName, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                        if (r == 0)
                        {
                            address._stringAddress = hostName;

                            if (AtUtils::Left(address._stringAddress, 7) == "::ffff:")
                            {
                                // IPv4-mapped IPv6 address
                                address._stringAddress = AtUtils::Mid(address._stringAddress, 7);
                            }

                            // Remove the NIC part of IPV6 name
                            if (family == AF_INET6)
                            {
                                size_t percentPos = address._stringAddress.find("%");
                                if (percentPos != std::string::npos)
                                    address._stringAddress = "[" + AtUtils::Left(address._stringAddress, percentPos) + "]";
                                else
                                    address._stringAddress = "[" + address._stringAddress + "]";
                            }

                            // Ignore IPV4 and IPV6 loopback
                            if ((address._stringAddress != "127.0.0.1") && (address._stringAddress != "[::1]"))
                                addresses.push_back(std::move(address));
                        }
                    }
                }
            }
        }

        ::freeifaddrs(pAddressInfo);
    }

    return addresses;
}

#else
std::vector<WebServerNetworkAddress> GetNetworkAddresses()
{
    std::lock_guard lock(_lockGetIpAddress);
    std::vector<WebServerNetworkAddress> addresses;

    char hostName[0x100]{};
    int r = ::gethostname(hostName, 0x100);
    if (r == AtUtils::SocketError)
    {
        // WSANOTINITIALISED
        r = WSAGetLastError();
    }

    // We will have multiple address. Use the last in the list
    struct hostent* phe = gethostbyname(hostName);
    if (phe)
    {
        for (int i = 0; phe->h_addr_list[i] != 0; ++i)
        {
            struct in_addr addr;
            memcpy_s(&addr, sizeof(addr), phe->h_addr_list[i], sizeof(struct in_addr));
            WebServerNetworkAddress address;
            address._stringAddress = inet_ntoa(addr);
            addresses.push_back(address);
        }
    }

    return addresses;
}

#endif

bool IsNetworkConnected()
{
    return (!GetNetworkAddresses().empty());
}

