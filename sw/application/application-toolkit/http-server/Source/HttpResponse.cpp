/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "OSocket.h"
#include "HttpCommand.h"
#include "StreamReader.h"
#include "StringBuilder.h"
#include "OTime.h"
#include "HttpServer.h"
#include "EmbeddedFile.h"

HttpResponse::HttpResponse(IHttpServerInternal* pHttpServer, std::shared_ptr<ISocket> spSocket,
                           std::shared_ptr<AtUtils::IFileSystem> spFileSystem)
: _pHttpServer(pHttpServer)
, _spSocket(std::move(spSocket))
, _spFileSystem{std::move(spFileSystem)}
, _contentType("text/html")
, _statusCode(HttpStatusCode::OK)
, _statusDescription(GetStatusString(HttpStatusCode::OK))
, _encoding(UTF8)
, _noCache(false)
, _closeSocketAfterResponse(false)
{
    if (!pHttpServer)
    {
        throw std::runtime_error("Null HTTP Server");
    }

    if (!_spSocket)
    {
        throw std::runtime_error("Null socket");
    }
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::SetCacheSetting(bool state)
{
    _noCache = !state;
}

void HttpResponse::FileNotFound(std::string url)
{
    std::cout << "HTTP Server: file not found " << url << std::endl;
    if (url.empty())
    {
        SetStatusCode(HttpStatusCode::Not_Found);
        SetStatusDescription("The file was not found.");
        SetCacheSetting(false);
        return;
    }

    StringBuilder message;
    std::string notFoundMessage = AtUtils::FormatString("File not found: '%s'", url.c_str());
    CreateErrorPage(message, notFoundMessage);
    SetStatusCode(HttpStatusCode::Not_Found);
    SetStatusDescription("The file was not found.");
    SetCacheSetting(false);
    Output(message);
}

void HttpResponse::ServiceUnavailable(std::string reason)
{
    std::cout << "HTTP Server: service unavailable " << reason << std::endl;
    StringBuilder message;
    std::string notFoundMessage = AtUtils::FormatString("Service unavailable. <br/><br/>%s", reason.c_str());
    CreateErrorPage(message, notFoundMessage);
    SetStatusCode(HttpStatusCode::Service_Unavailable);
    SetStatusDescription("Service unavailable. The server is at capacity");
    SetCacheSetting(false);
    Output(message);
    _closeSocketAfterResponse = true;
}

void HttpResponse::Close()
{
    std::string statusLine = AtUtils::FormatString("HTTP/1.1 %d %s", (int)_statusCode, _statusDescription.c_str());
    Reply(statusLine);
    Reply("Server: Intel-HTTP-Server");

    std::string contentString = AtUtils::FormatString("Content-Type: %s", _contentType.c_str());
    Reply(contentString);

    std::string lengthString = AtUtils::FormatString("Content-Length: %zu", _spData ? _spData->size() : 0);
    Reply(lengthString);

    OTime theTimeNow;
    std::string date_time_str = AtUtils::FormatString("Date: %s", theTimeNow.GetRFC1123().c_str());
    Reply(date_time_str);

    if (_noCache)
    {
        Reply("Pragma: no-cache");
        Reply("Cache-Control: no-cache");
    }
    else
    {
        Reply("Cache-Control: public");
        OTime expires_time = theTimeNow + OTimeSpan(24, 0, 0); // Expires in 1 day
        std::string expiresString = AtUtils::FormatString("Expires: %s", expires_time.GetRFC1123().c_str());
        Reply(expiresString);
    }

    Reply("Access-Control-Allow-Origin: *");
    Reply("Content-Security-Policy: img-src 'self' data:; default-src 'self'");

    Reply("");
    if (_spData)
    {
        if (_spData->size() > 0)
            _spSocket->SendBytes(*_spData, false);
    }
    else
        Reply("");

    _spSocket->FlushSendBuffer();

    if (_closeSocketAfterResponse)
        _spSocket->Close();
}

std::string HttpResponse::GetStatusString(HttpStatusCode status_code)
{
    if (status_code == HttpStatusCode::OK)
        return "OK";
    else if (status_code == HttpStatusCode::Switching_Protocols)
        return "Web Socket Protocol Handshake";
    else
        return "Error";
}

void HttpResponse::Reply(const std::string& reply)
{
    // T39: Refuse carriage returns and line feeds in HTTP response headers
    for (auto ch : reply)
    {
        if ((ch == 0xd) || (ch == 0xa))
            return;
    }

    _spSocket->SendLine(reply, false);
}

void HttpResponse::SetStatusDescription(const char* text)
{
    _statusDescription = text;
}

void HttpResponse::SetStatusCode(HttpStatusCode status_code)
{
    _statusCode = status_code;
    _statusDescription = GetStatusString(status_code);
}

void HttpResponse::SetContentType(const char* content_type)
{
    _contentType = content_type;
}

void HttpResponse::Output(const std::string& text)
{
    size_t stringLength = text.length();
    if (stringLength > 0)
    {
        _spData.reset(new std::vector<uint8_t>(stringLength));
        AtUtils::CopyMemoryBuffer(&_spData->at(0), _spData->size(), text.data(), stringLength);
    }
}

void HttpResponse::Output(std::shared_ptr<std::vector<uint8_t>> data)
{
    _spData = std::move(data);
}

struct FileTypeDetails
{
    const char*    _extension;
    std::string    _contentType;
    bool        _cache; // Client cache
    bool        _serverCache;
};

static FileTypeDetails fileTypes[] =
{
    { ".js",   "application/javascript", false, true },
    { ".css",  "text/css", false, true },
    { ".png",  "image/png", true, false },
    { ".html", "text/html", false, true },
    { ".xml",  "application/octet-stream", false, false },
    { ".json", "application/json", false, true },
    { ".bmp",  "image/bmp", true, false },
    { ".jpg",  "image/jpeg", true, false },
    { ".ico",  "image/x-icon", true, false },
    { ".raw",  "application/octet-stream", true, false },
    { ".oaf",  "application/oaf", false, false },
    { ".yuv",  "application/octet-stream", false, false },
    { ".dpx",  "application/octet-stream", false, false },
    { ".yuv10",  "application/octet-stream", false, false },
    { ".tif",  "application/tiff", false, false },
    { ".tiff",  "application/tiff", false, false },
    { ".pid",  "application/octet-stream", false, false },
    { ".mib",  "application/octet-stream", false, false },
    { ".lic",  "application/octet-stream", false, false },
    { ".bin",  "application/octet-stream", false, false },
    { ".woff",  "application/octet-stream", true, true },
    { ".woff2",  "application/octet-stream", true, true },
    { ".ttf",  "application/octet-stream", true, true },
    { ".txt",  "text/plain", false, false },
    { ".csv",  "text/csv", false, false },
    { ".xsl",  "application/octet-stream", false, false },
};

void HttpResponse::ReturnFile(const std::filesystem::path& filename, std::string extension, std::string url)
{
    int numFileTypes = sizeof(fileTypes) / sizeof(fileTypes[0]);
    for (int e = 0; e < numFileTypes; e++)
    {
        if (fileTypes[e]._extension == extension)
        {
            SetContentType(fileTypes[e]._contentType.c_str());
            SetCacheSetting(fileTypes[e]._cache);

            OHttpServer::BinaryStreamReader reader(_spFileSystem->MakeFile(), filename);

            std::shared_ptr<std::vector<uint8_t>> spFileContents = reader.GetData();
            std::shared_ptr<std::vector<uint8_t>> spReturnData = spFileContents;
            if (fileTypes[e]._contentType == "text/css")
                spReturnData = CssConstantInsertion(std::move(spFileContents));

            Output(spReturnData);

            if (fileTypes[e]._serverCache && !url.empty())
            {
                std::shared_ptr<CachedFile> spCachedFile(new CachedFile(url.c_str(),
                                                                        fileTypes[e]._contentType.c_str(),
                                                                        fileTypes[e]._cache,
                                                                        std::move(spReturnData)));
                _pHttpServer->AddToServerCache(std::move(spCachedFile));
            }

            return;
        }
    }

    // Assume binary
    SetContentType("application/octet-stream");
    SetCacheSetting(false);

    OHttpServer::BinaryStreamReader reader(_spFileSystem->MakeFile(), filename);
    std::shared_ptr<std::vector<uint8_t>> spFileContents = reader.GetData();
    Output(std::move(spFileContents));
}

void HttpResponse::ReturnFile(std::shared_ptr<EmbeddedFile> spEmbeddedFile,
                              std::string extension, std::string url)
{
    std::string contentType = "application/octet-stream"; // Default to binary
    bool cache = true;
    bool serverCache = true;

    int numFileTypes = sizeof(fileTypes) / sizeof(fileTypes[0]);
    for (int e = 0; e < numFileTypes; e++)
    {
        if (fileTypes[e]._extension == extension)
        {
            contentType = fileTypes[e]._contentType;
            cache = fileTypes[e]._cache;
            serverCache = fileTypes[e]._serverCache;
            break;
        }
    }

    SetContentType(contentType.c_str());
    SetCacheSetting(cache);

    auto spFileContents = spEmbeddedFile->CopyData();
    auto spReturnData = spFileContents;
    if (contentType == "text/css")
        spReturnData = CssConstantInsertion(std::move(spFileContents));

    Output(spReturnData);

    if (serverCache && !url.empty())
    {
        std::shared_ptr<CachedFile> spCachedFile(new CachedFile(url.c_str(),
                                                                contentType.c_str(),
                                                                cache,
                                                                std::move(spReturnData)));
        _pHttpServer->AddToServerCache(std::move(spCachedFile));
    }
}

void HttpResponse::ReturnCache(std::shared_ptr<CachedFile> spCachedFile)
{
    SetContentType(spCachedFile->_contentType.c_str());
    SetCacheSetting(spCachedFile->_clientCache);
    Output(spCachedFile->_spData);
}

void HttpResponse::ReturnString(std::string string, std::string extension)
{
    int numFileTypes = sizeof(fileTypes) / sizeof(fileTypes[0]);
    for (int e = 0; e < numFileTypes; e++)
    {
        if (fileTypes[e]._extension == extension)
        {
            SetContentType(fileTypes[e]._contentType.c_str());
            StringBuilder message;
            message.Append(string);
            Output(message);
            break;
        }
    }
}

std::shared_ptr<std::vector<uint8_t>> HttpResponse::CssConstantInsertion(std::shared_ptr<std::vector<uint8_t>> spFileContents)
{
    std::string contentsString{ reinterpret_cast<char*>(spFileContents->data()), spFileContents->size() };
    std::string filteredString;
    bool changed = false;

    while (true)
    {
        size_t constantStartPosition = contentsString.find("<!--#");
        if (constantStartPosition == std::string::npos)
            break;

        size_t constantEndPosition = contentsString.find("#-->", constantStartPosition + 5);
        if (constantEndPosition != std::string::npos)
        {
            std::string constantName = AtUtils::Mid(contentsString, constantStartPosition + 5, (constantEndPosition - (constantStartPosition  + 5)));
            std::string constantValue = LookupCssConstant(constantName); // "#181828";
            filteredString += AtUtils::Left(contentsString, constantStartPosition) + constantValue;

            changed = true;
            contentsString = AtUtils::Mid(contentsString, constantEndPosition + 4);
        }
        else
            break;
    }

    if (!changed)
        return spFileContents;

    filteredString += contentsString;

    auto spFilteredContents = std::make_shared<std::vector<uint8_t>>(filteredString.length());
    AtUtils::CopyMemoryBuffer(&spFilteredContents->at(0), spFilteredContents->size(), filteredString.data(), filteredString.length());
    return spFilteredContents;
}

std::string HttpResponse::LookupCssConstant(std::string& constantName)
{
    std::string constantValue;
    _pHttpServer->LookupCssConstant(constantName, constantValue);
    return constantValue;
}

void HttpResponse::CreateErrorPage(StringBuilder& stringBuilder,
                                   std::string& message)
{
    const char* htmlTemplate =
"<html>"
"<head>"
"    <title>Intel Web Server</title>"
"    <style>"
"    body"
"    {"
"        background:#ffffff;"
"        font-family: Segoe UI;"
"        font-size: 16px;"
"        color:#555555;"
"        user-select: none;"
"        margin:20px;"
"        padding-left:30px;"
"        padding-right:30px;"
"    }"
"    "
"    #title_bar"
"    {"
"        width:380px;"
"        color:#0071c5;"
"    }"
"    "
"    #message_area"
"    {"
"        position:relative;"
"        top:5px;"
"        height:auto;"
"    }"
"    #details_area"
"    {"
"        position:relative;"
"    }"
"    "
"    .horizontal_line"
"    {"
"        border-color:#0071c5;"
"        border-width:2px;"
"        border-style:solid;"
"        border-bottom-style:none;"
"        margin-top:3px;"
"    }"
"    </style>"
"</head>"
"<body>"
"<p />"
"<div id='title_bar'>"
"PRODUCT_NAME : SERVER_NAME"
"</div>"
"<div class='horizontal_line'></div>"
"<div id='message_area'>"
"THE_MESSAGE"
"</div>"
"<div id='details_area'>"
"THE_DETAILS"
"</div>"
"<div class='horizontal_line'></div>"
"</body>"
"</html>";
    std::string ctemplate(htmlTemplate);

    char computerName[0x200];
    uint32_t size = 0x200;
    std::string serverName{};
    if (AtUtils::OS::Get()->GetComputerName(computerName, &size))
    {
        serverName = computerName;
    }

    std::string details = "<br/>";
    std::string productName = _pHttpServer->GetProductName();
    AtUtils::Replace(ctemplate, "PRODUCT_NAME", productName.c_str());
    AtUtils::Replace(ctemplate, "SERVER_NAME",  serverName.c_str());
    AtUtils::Replace(ctemplate, "THE_MESSAGE",  message.c_str());
    AtUtils::Replace(ctemplate, "THE_DETAILS",  details.c_str());
    stringBuilder.Append(ctemplate);
}

///////////////////

WebSocketResponse::WebSocketResponse(IHttpServerInternal* pHttpServer,
                                     std::shared_ptr<ISocket> spSocket,
                                     std::shared_ptr<AtUtils::IFileSystem> spFileSystem,
                                     int wsIndex)
: _httpResponse(pHttpServer, std::move(spSocket), std::move(spFileSystem))
, _acceptKey{}
, _wsIndex{wsIndex}
, _legacyProtocol{false}
, _legacyHandshakeHash{}
, _legacyOriginHandshake{}
, _legacyLocationHandshake{}
, _subProtocol{}
{
}

void WebSocketResponse::SetStatusCode(HttpStatusCode statusCode)
{
    _httpResponse.SetStatusCode(statusCode);
}

void WebSocketResponse::Accept(std::string acceptKey)
{
    _acceptKey = std::move(acceptKey);
}

void WebSocketResponse::SetSubProtocol(std::string subProtocol)
{
    _subProtocol = std::move(subProtocol);
}

void WebSocketResponse::Close()
{
    std::string statusLine = AtUtils::FormatString("HTTP/1.1 %d %s", (int)_httpResponse.GetStatusCode(),
                                                   _httpResponse.GetStatusDescription().c_str());
    _httpResponse.Reply(statusLine);
    _httpResponse.Reply("Upgrade: Websocket");
    _httpResponse.Reply("Connection: Upgrade");

    if (_subProtocol != "")
    {
        statusLine = AtUtils::FormatString("Sec-WebSocket-Protocol: %s", _subProtocol.c_str());
        _httpResponse.Reply(statusLine);
    }

    if (_legacyProtocol)
    {
        _httpResponse.Reply(_legacyOriginHandshake); // "Sec-WebSocket-Origin: http://192.92.109.132:8080");
        _httpResponse.Reply(_legacyLocationHandshake); // "Sec-WebSocket-Location: ws://192.92.109.132:8080/");

    }
    else
    {
        std::string acceptString = AtUtils::FormatString("Sec-WebSocket-Accept: %s", _acceptKey.c_str());
        _httpResponse.Reply(acceptString);
    }

    //_httpResponse.Reply("Sec-WebSocket-Protocol: chat");
    _httpResponse.Reply("");

    std::shared_ptr<ISocket> spSocket = _httpResponse.GetSocket();

    if (_legacyProtocol)
    {
        spSocket->SendBytes(_legacyHandshakeHash, false);
    }

    spSocket->FlushSendBuffer();
}

void WebSocketResponse::SetLegacyHandshakeHash(const uint8_t* data, int length,
                                               const std::string& origin, const std::string& url)
{
    _legacyHandshakeHash.resize(length);
    AtUtils::CopyMemoryBuffer(&_legacyHandshakeHash[0], _legacyHandshakeHash.size(), data, length);
    _legacyProtocol = true;

    _legacyOriginHandshake = AtUtils::FormatString("Sec-WebSocket-Origin: %s", origin.c_str());
    std::string wsLocation = origin;
    AtUtils::Replace(wsLocation, "http", "ws");

    _legacyLocationHandshake = AtUtils::FormatString("Sec-WebSocket-Location: %s%s", wsLocation.c_str(), url.c_str());
}
