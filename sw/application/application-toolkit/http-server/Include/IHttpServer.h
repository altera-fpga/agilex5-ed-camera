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
#include <unordered_map>
#include <filesystem>
#include <string>

using CssConstantsMap = std::unordered_map<std::string, std::string>;

class IHttpServer
{
public:
    virtual void UseCache(bool state) = 0;
    virtual void UpdateCssConstant(const char* constantName, const char* constantValue) = 0;
    virtual CssConstantsMap GetCssConstants() = 0;
    virtual bool IsSecure() = 0;
    virtual void EnablePut(bool state) = 0;
    virtual void EnablePost(bool state) = 0;
    virtual void EnablePatch(bool state) = 0;
    virtual void UseEmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd) = 0;

    protected: virtual ~IHttpServer() {}
};

IHttpServer* CreateHttpServer(IHttpServerHost* host,
                              int& port, // Set to -1 to auto select
                              const std::filesystem::path& workingFolder,
                              const std::string& productName,
                              const char* sslPrivateKeyFilename = nullptr,
                              const char* sslSignedCertificateFilename = nullptr);
void DeleteHttpServer(IHttpServer*& http_server);
