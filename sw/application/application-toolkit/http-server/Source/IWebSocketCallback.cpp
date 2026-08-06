/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "IHttpServer.h"

IWebSocketCallback::IWebSocketCallback(IWebSocketService* pIWebSocket)
:	_pIWebSocket(pIWebSocket)
{
    if (_pIWebSocket)
    {
        _wsVersion = _pIWebSocket->GetVersion();
        _subProtocolString = _pIWebSocket->GetSubProtocolString();
    }
}

IWebSocketCallback::~IWebSocketCallback()
{
}

void IWebSocketCallback::SendToClient(IWebSocketMessage* pMessage)
{
    if (_pIWebSocket)
        _pIWebSocket->SendToClient(pMessage);
}

void IWebSocketCallback::ReceiveWebSocketMessage(IWebSocketMessage* pMessage)
{
}

void IWebSocketCallback::SetShutdownCritSec(std::recursive_mutex* pCriticalSection)
{
    _pShutdownCritSec = pCriticalSection;
}