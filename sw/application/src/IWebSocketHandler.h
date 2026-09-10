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

typedef std::function<bool(IWebSocketService* web_socket)> WebSocketOpenedCB;

class IWebSocketHandler
{
public:
    virtual ~IWebSocketHandler() {}
    virtual size_t RegisterWebSocketHandler(WebSocketOpenedCB web_socket_handler) = 0;
    virtual void UnRegisterWebSocketHandler(size_t web_socket_handler_handle) = 0;
};
