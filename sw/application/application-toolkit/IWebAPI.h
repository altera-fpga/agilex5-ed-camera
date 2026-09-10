/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <string>
#include <cstdint>
#include "IHttpServerHost.h"

class IWebAPI
{
public:
    virtual std::string GetControlsJSON() = 0;
    virtual std::string GetControlJSON(uint32_t controlID) = 0;
    virtual uint32_t GetControlID(const std::string& panelName, const std::string& controlLabel) = 0;
    virtual uint32_t GetButtonID(const std::string& panelName, const std::string& buttonLabel) = 0;
    virtual void SetControlValue(const std::string& JSON) = 0;
    virtual bool WebSocketOpened(IWebSocketService* web_socket) = 0;
};