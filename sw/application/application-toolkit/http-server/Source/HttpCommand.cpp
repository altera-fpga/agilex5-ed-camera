/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "HttpCommand.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include "OSocket.h"
#include "AppToolkitFile.h"
#include "StreamReader.h"
#include "SHA1.h"
#include "WebSocketCommands.h"
#include "WebSocket.h"

HttpCommand::HttpCommand(std::shared_ptr<ISocket> spSocket, IHttpServerInternal* pHttpServer,
                         std::shared_ptr<AtUtils::IFileSystem> spFileSystem)
: _spSocket{std::move(spSocket)}
, _pHttpServer{pHttpServer}
, _spFileSystem{std::move(spFileSystem)}
, _originStr{}
{
}

HttpCommand::~HttpCommand()
{
}

std::recursive_mutex dump_cs;

void HttpCommand::DumpCommand(std::ostream& os)
{
    static bool showAll = true;
    size_t count = 1;
    if (showAll)
        count = size();

    os << "Received HttpCommand, " << size() << " lines" << std::endl;

    std::lock_guard lock(dump_cs);
    for (size_t i = 0; i < count; i++)
    {
        os << "[" << (int)_spSocket->GetSocket() << "/" << AtUtils::OS::Get()->GetCurrentThreadId() << " - " << i << "] " << at(i).c_str() << std::endl;
    }

    os << std::endl;
}

void HttpCommand::Execute()
{
    if (!_pHttpServer || !_spSocket)
        return;

    std::shared_ptr<IHttpResponse> spResponse = _pHttpServer->CreateHttpResponse(_spSocket);

    // 'GET / HTTP/1.1'
    std::string command = (*this)[0];
    //DumpCommand(std::cout);

    if (AtUtils::Left(command, 3) == "GET")
        ProcessGet(spResponse);
    else if (_pHttpServer->ReceivedPutAllowed() && (AtUtils::Left(command, 3) == "PUT"))
        ProcessPut(spResponse);
    else if (_pHttpServer->ReceivedPostAllowed() && (AtUtils::Left(command, 4) == "POST"))
        ProcessPost(spResponse);        
    else if (_pHttpServer->ReceivedPatchAllowed() && (AtUtils::Left(command, 5) == "PATCH"))
        ProcessPatch(spResponse);
    else
        spResponse->SetStatusCode(HttpStatusCode::Forbidden);

    if (spResponse)
        spResponse->Close();
}

void HttpCommand::ProcessGet(std::shared_ptr<IHttpResponse>& spResponse)
{
    AtUtils::Socket debugSocket = _spSocket->GetSocket();
    (void)debugSocket;

    // Use the string before it was made lowercase
    std::string test = AtUtils::Mid(at(0), 4);
    size_t endPosition = test.find(" ");
    std::string rawUrl = (endPosition != std::string::npos) ? AtUtils::Left(std::move(test), endPosition) : std::move(test);
    ReplaceControlChars(rawUrl);
    std::string url = std::move(rawUrl);

    if ((url.size() == 0) || (url[0] != '/'))
    {
        return spResponse->FileNotFound(std::move(url));
    }

    std::string params;
    size_t paramsPosition = url.find("?");
    if (paramsPosition != std::string::npos)
    {
        params = AtUtils::Mid(url, paramsPosition + 1);
        url = AtUtils::Left(url, paramsPosition);
    }

    _originStr = GetEntry("origin");

    // Check for WebSocket upgrade
    if (size() > 1)
    {
        std::string upgradeString = GetEntry("upgrade");
        AtUtils::MakeLower(upgradeString);
        if (upgradeString == "websocket")
        {
            // Change the type for the ui
            _spSocket->UpgradeToWebSocket();
            _pHttpServer->UpdateUI();

            return ProcessWebSocket(spResponse, url);
        }
    }

    // Relative file paths are not permitted to avoid potential
    // access to unauthorized local resources
    if (url.find("..") != std::string::npos)
        return spResponse->FileNotFound(std::move(url));

    THostResponse rsp;
    bool handledByHost = _pHttpServer->GetHost()->ProcessGet(url, params, rsp);

    if (handledByHost)
    {
        if (rsp._fileNotFound)
            spResponse->FileNotFound("");
        else
        {
            spResponse->SetContentType(rsp._type.c_str());

            if (rsp._spDataBuffer)
                spResponse->Output(rsp._spDataBuffer);
            else
                spResponse->Output(rsp._data);
        }
    }
    else
    {
        // Mark the socket as a regular HTTP socket (not WebSocket)
        // so we can count connections
        _spSocket->SetHttpSocket();

        bool exportFile = false;

        if (url == "/")
            url = "/index.html";
        else if (AtUtils::Left(url, 13) == "/export-files")
            exportFile = true;

#if NEW_CLIENT_ON_INDEX
        // Notify host if a new web page request comes in
        if (AtUtils::Right(url, 10) == "index.html")
        {
            auto ipAddress = _spSocket->GetIpAddress();
            _pHttpServer->GetHost()->NewClientConnectionStarted(ipAddress);
        }
#endif

        // If using embedded files then they are the only file source
        if (_pHttpServer->UsingEmbeddedFiles() && !exportFile)
        {
            std::shared_ptr<EmbeddedFile> spEmbeddedFile = _pHttpServer->GetEmbeddedFile(url);
            if (spEmbeddedFile)
            {
                std::filesystem::path filePath(url);
                std::string ext = filePath.extension().string();
                return spResponse->ReturnFile(std::move(spEmbeddedFile), std::move(ext), std::move(url));
            }
        }

        std::shared_ptr<CachedFile> spCachedFile = _pHttpServer->GetServerCachedFile(url);
        if (spCachedFile)
        {
            spResponse->ReturnCache(std::move(spCachedFile));
        }
        else
        {
            std::filesystem::path filePath = _pHttpServer->GetSearchFolder();
            std::string useURL;
            if (AtUtils::Left(url, 1) == "/")
                useURL = AtUtils::Mid(url, 1);
            else
                useURL = url;
            std::filesystem::path urlPart(useURL);
            filePath /= urlPart;
            filePath.make_preferred();
            std::string ext = filePath.extension().string();

            // File paths with symbolic links are not permitted to avoid potential
            // access to unauthorized local resources
            if (_spFileSystem->FileHasSymbolicLink(filePath))
                return spResponse->FileNotFound(std::move(url));

            if (_spFileSystem->Exists(filePath) && !_spFileSystem->IsDirectory(filePath))
                spResponse->ReturnFile(std::move(filePath), std::move(ext), std::move(url));
            else
                spResponse->FileNotFound(std::move(url));
        }
    }
}

void HttpCommand::ProcessPut(std::shared_ptr<IHttpResponse>& spResponse)
{
    auto hostProcessFn = [this](const std::string& ipAddress,
                                const std::string& rawUrl,
                                const std::vector<uint8_t>& theData,
                                THostResponse& hostResponse)
    {
        _pHttpServer->GetHost()->ProcessPut(ipAddress, rawUrl, GetEntry("content-type"),
                                            &theData[0], theData.size(), hostResponse);
    };

    ProcessPutPostAndPatch("PUT", spResponse, hostProcessFn);
}

void HttpCommand::ProcessPost(std::shared_ptr<IHttpResponse>& spResponse)
{
    auto hostProcessFn = [this](const std::string& ipAddress,
                                const std::string& rawUrl,
                                const std::vector<uint8_t>& theData,
                                THostResponse& hostResponse)
    {
        _pHttpServer->GetHost()->ProcessPost(ipAddress, rawUrl, GetEntry("content-type"),
                                             &theData[0], theData.size(), hostResponse);
    };

    ProcessPutPostAndPatch("POST", spResponse, hostProcessFn);
}

void HttpCommand::ProcessPatch(std::shared_ptr<IHttpResponse>& spResponse)
{
    auto hostProcessFn = [this](const std::string& ipAddress,
                                const std::string& rawUrl,
                                const std::vector<uint8_t>& theData,
                                THostResponse& hostResponse)
    {
        _pHttpServer->GetHost()->ProcessPatch(ipAddress, rawUrl, GetEntry("content-type"),
                                              &theData[0], theData.size(), hostResponse);
    };

    ProcessPutPostAndPatch("PATCH", spResponse, hostProcessFn);
}

void HttpCommand::ProcessPutPostAndPatch(const std::string& httpMethod,
                                         std::shared_ptr<IHttpResponse>& spResponse,
                                         HostProcessFn hostProcessFn)
{
    std::string contentLength = GetEntry("content-length");
    size_t length = 0;
    AtUtils::FromString(contentLength, length);

    std::vector<uint8_t> theData;

    size_t previousSize = 0;
    int blockCount = 0;
    while (theData.size() < length)
    {
        if (!_spSocket->ReceiveBytes(theData, true))
            break;

        size_t newSize = theData.size();
        if (newSize == length)
            break;

        size_t increase = newSize - previousSize;
        previousSize = newSize;

        if (increase == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            blockCount++;
            if (blockCount > 100)
                break;
        }
        else
            blockCount = 0;
    }

    if ((length > 0) && (theData.size() == length))
    {
        std::string test = AtUtils::Mid(at(0), httpMethod.length() + 1);
        size_t endPosition = test.find(" ");
        std::string rawUrl = (endPosition != std::string::npos) ? AtUtils::Left(std::move(test), endPosition) : std::move(test);
        ReplaceControlChars(rawUrl);

        if ((rawUrl.size() == 0) || (rawUrl[0] != '/'))
        {
            return spResponse->FileNotFound(std::move(rawUrl));
        }

        {
            auto ipAddress = _spSocket->GetIpAddress();

            // Let the HttpServerHost handle the upload, call
            // either _pHttpServer->GetHost()->ProcessPost or
            // _pHttpServer->GetHost()->ProcessPatch via the lambda fn
            THostResponse hostResponse;
            hostProcessFn(ipAddress, rawUrl, theData, hostResponse);

            spResponse->SetContentType(hostResponse._type.c_str());

            if (hostResponse._spDataBuffer)
                spResponse->Output(hostResponse._spDataBuffer);
            else
                spResponse->Output(hostResponse._data);

        }
    }
}

void HttpCommand::ReplaceControlChars(std::string& inString)
{
    size_t percent_pos = inString.find("%");
    if (percent_pos == std::string::npos)
        return; // No control chars

    std::string mod_string;
    size_t startPosition = 0;
    do
    {
        mod_string += AtUtils::Mid(inString, startPosition, (percent_pos - startPosition));
        std::string control_char = AtUtils::Mid(inString, percent_pos + 1, 2);
        char ch = (char)strtol(control_char.c_str(), 0, 16);
        mod_string += ch;

        startPosition = (percent_pos + 3);
        percent_pos = inString.find("%", startPosition);
    } while (percent_pos != std::string::npos);

    mod_string += AtUtils::Mid(inString, startPosition);
    inString = std::move(mod_string);
}

uint32_t GetDeprecatedValue(std::string key)
{
    int numSpaces = 0;
    int64_t value = 0;
    std::string numbers;
    for (size_t i = 0, ni = key.length(); i < ni; i++)
    {
        char ch = key[i];
        if (ch == ' ')
            numSpaces++;
        else if ((ch >= '0') && (ch <= '9'))
            numbers += ch;
    }

#ifdef WIN32
    value = _atoi64(numbers.c_str());
#else
    value = strtoull(numbers.c_str(), 0, 10);
#endif

    if (numSpaces != 0)
        value /= numSpaces;

    return (uint32_t)value;
}

// Little Endian to Big Endian long

#define LE_TO_BE_SHORT(SHORT)\
    (((SHORT >> 8) & 0x00FF) | ((SHORT << 8) & 0xFF00))

#define LE_TO_BE_LONG(int32_t)\
    (((LE_TO_BE_SHORT(int32_t >> 16)) | \
     ((LE_TO_BE_SHORT((int32_t & 0xFFFF)) << 16))))


void HttpCommand::ProcessWebSocket(std::shared_ptr<IHttpResponse>& spResponse,
                                   std::string& url)
{
    if (!_pHttpServer)
        return;

    static int ws_index = 0;
    std::shared_ptr<IWebSocketResponse> spWsResponse = _pHttpServer->CreateWebSocketResponse(spResponse->GetSocket(), ws_index++);

    if (!spWsResponse)
        return;

    spResponse = nullptr;

    std::vector<WebSocketSubProtocol> sub_protocols = GetSubProtocolRequests();

    WebSocketSubProtocol subProtocol;

    int sub_protocol_index = _pHttpServer->GetHost()->GetPreferredWebSocketProtocol(sub_protocols);
    if (sub_protocol_index >= 0)
        subProtocol = sub_protocols[sub_protocol_index];

    std::string key_1 = GetEntry("Sec-WebSocket-Key1");
    std::string version_str = GetEntry("Sec-WebSocket-Version");
    int32_t version = 0;
    AtUtils::FromString(version_str, version);

    if (key_1.empty())
    {
        std::string key = GetEntry("Sec-WebSocket-Key");
        std::string val = ComputeWebSocketHandshakeSecurityHash09(std::move(key));
        spWsResponse->SetStatusCode(HttpStatusCode::Switching_Protocols);
        spWsResponse->Accept(std::move(val));

        auto ipAddress = _spSocket->GetIpAddress();
#ifndef NEW_CLIENT_ON_INDEX
        _pHttpServer->GetHost()->NewClientConnectionStarted(ipAddress);
#endif

        // Now handle messages to/from the socket
        std::shared_ptr<IWebSocket> spWebSocket = _pHttpServer->CreateWebSocket(_spSocket, version,
                                                                                subProtocol._stringName, subProtocol._isBinary);
        bool handled = _pHttpServer->GetHost()->WebSocketOpened(spWebSocket.get());

        // If we can't handle this protocol type, drop the web socket by not accepting the protocol
        spWsResponse->SetSubProtocol(handled ? subProtocol._stringName : "");
        spWsResponse->Close();
        spWsResponse = nullptr;

        spWebSocket->SendNegotiationMessage();
        spWebSocket->WaitUntilClosed();

        _pHttpServer->GetHost()->ClientConnectionEnded(ipAddress);
    }
}

std::vector<WebSocketSubProtocol> HttpCommand::GetSubProtocolRequests()
{
    std::vector<WebSocketSubProtocol> protocols;

    std::string requestedProtocols = GetEntry("Sec-WebSocket-Protocol");
    // "one, two"

    size_t comma_pos = std::string::npos;
    do
    {
        comma_pos = requestedProtocols.find(",");
        if (comma_pos != std::string::npos)
        {
            std::string protocolString = AtUtils::Left(requestedProtocols, comma_pos);
            AtUtils::Replace(protocolString, " ", "");
            AddProtocolRequest(protocols, std::move(protocolString));

            requestedProtocols = AtUtils::Mid(requestedProtocols, comma_pos + 1);
        }
        else
        {
            std::string protocolString = requestedProtocols;
            AtUtils::Replace(protocolString, " ", "");
            AddProtocolRequest(protocols, std::move(protocolString));
        }

    } while (comma_pos != std::string::npos);

    return protocols;
}

void HttpCommand::AddProtocolRequest(std::vector<WebSocketSubProtocol>& protocols, std::string protocolString)
{
    WebSocketSubProtocol subProtocol;
    subProtocol._isBinary = false;
    subProtocol._stringName = std::move(protocolString);
    protocols.push_back(std::move(subProtocol));
}
/////////////////////////////////////////////////////////

std::string HttpCommand::ToBase16String(uint8_t* pData, int length)
{
    static const char* alphabet = "0123456789ABCDEF";
    char* str = new char[length * 2 + 1];
    char* strScan = str;

    for (int i = 0; i < length; i++)
    {
        *strScan++ = alphabet[(pData[i] & 0xf0) >> 4];
        *strScan++ = alphabet[pData[i] & 0xf];
    }
    str[length * 2] = 0;

    std::string result(str);
    delete [] str, str = 0;

    return result;
}

std::string HttpCommand::ToBase64String(uint8_t* pData, int length)
{
    static const char* char64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string str;
    uint8_t buffer1[3]{};
    uint8_t buffer2[4]{};
    int i = 0;

    for (int n = 0; n < length; n++)
    {
        buffer1[i++] = *pData++;
        if (i == 3)
        {
            buffer2[0] =  (buffer1[0] & 0b11111100) >> 2;
            buffer2[1] = ((buffer1[0] & 0b00000011) << 4) | ((buffer1[1] & 0b11110000) >> 4);
            buffer2[2] = ((buffer1[1] & 0b00001111) << 2) | ((buffer1[2] & 0b11000000) >> 6);
            buffer2[3] =  (buffer1[2] & 0b00111111);

            for (int j = 0; j < 4; j++)
            {
                uint8_t charIndex = buffer2[j];
                str += char64[charIndex];
            }

            i = 0;
        }
    }

    if (i > 0)
    {
        for (int j = i; j < 3; j++)
            buffer1[j] = 0;

        buffer2[0] =  (buffer1[0] & 0b11111100) >> 2;
        buffer2[1] = ((buffer1[0] & 0b00000011) << 4) + ((buffer1[1] & 0b11110000) >> 4);
        buffer2[2] = ((buffer1[1] & 0b00001111) << 2) + ((buffer1[2] & 0b11000000) >> 6);
        buffer2[3] =  (buffer1[2] & 0b00111111);

        for (int j = 0; j < (i + 1); j++)
        {
            uint8_t charIndex = buffer2[j];
            str += char64[charIndex];
        }

        int n_equals = (3 - i);
        for (int j = 0; j < n_equals; j++)
            str += '=';
    }

    return str;
}

std::string HttpCommand::ComputeWebSocketHandshakeSecurityHash09(std::string sec_web_socket_key)
{
    static const std::string magicKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // Combine the request sec_web_socket_key with magic key.
    std::string str = sec_web_socket_key + magicKey;

    // Compute the SHA1 hash
    SHA1 sha1;
    SHA1Hash sha1Hash = sha1.CalculateHash(str);

    // Base64 encode the hash
    std::string secWebSocketAccept = ToBase64String(sha1Hash, 20);
    return secWebSocketAccept;
}

std::string HttpCommand::GetEntry(std::string entryName)
{
    AtUtils::MakeLower(entryName);

    size_t len = entryName.length();
    for (size_t i = 0; i < size(); i++)
    {
        std::string commandItem = at(i);
        AtUtils::MakeLower(commandItem);

        if (entryName == AtUtils::Left(commandItem, len))
        {
            // 012: 5
            std::string right = AtUtils::Mid(at(i), len + 1);
            AtUtils::TrimLeft(right);
            return right;
        }
    }

    return "";
}
