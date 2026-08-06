/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "PictureWebSocketCommandProcessor.h"
#include "AppToolkitRawRgbImage.h"
#include "DataSegment.h"
#include "CommonApplicationBase.h"

PictureWebSocketCommandProcessor::PictureWebSocketCommandProcessor(IWebSocketService* pIWebSocketService)
:	WebSocketCommandHandler(pIWebSocketService)
{
}

PictureWebSocketCommandProcessor::~PictureWebSocketCommandProcessor()
{
	Unregister();
}

void PictureWebSocketCommandProcessor::Ready()
{
	// Register for pictures
	ICommonApplicationBase* pAppBase = CommonApplicationBase::Get();
	if (pAppBase)
	{
    	auto spImageNotifier = pAppBase->GetImageNotifier();
		if (spImageNotifier)
			Register(spImageNotifier.get());
	}
}

void PictureWebSocketCommandProcessor::SendBuffer(std::shared_ptr<AppToolkit::RawRgbImage16> spImage)
{
	if (spImage)
	{
		// Send picture details as text command, followed by the binary data
		JsonDOM jsonDOM("PictureDetails");
		AtUtils::IJsonObjectPtr commandNode = jsonDOM.GetRoot();
		commandNode->AddValue("width", spImage->GetWidth());
		commandNode->AddValue("height", spImage->GetHeight());
		std::shared_ptr<std::string> spJsonString = jsonDOM.ToString();
		std::shared_ptr<DataSendRequest> spPictureData = std::make_shared<DataSendRequest>(spImage->GetData());

		_pIWebSocket->SendToClient(new TextWebSocketMessage(std::move(spJsonString)));
		_pIWebSocket->SendToClient(new BinaryWebSocketMessage(spPictureData));
		_spPendingImage.reset();
	}
}

bool PictureWebSocketCommandProcessor::Update(std::shared_ptr<AppToolkit::RawRgbImage16>& spImage)
{
	if (spImage)
	{
		std::lock_guard lock(_pendingBufferCS);
		if (_sendNext)
			SendBuffer(spImage);
		else
		{
			if (_spPendingImage)
				_dropCount++;

			_spPendingImage = spImage;
		}
	}

	return true;
}

void PictureWebSocketCommandProcessor::Process(std::vector<std::shared_ptr<ByteArray>>& binaryData,
											   bool finalFrame)
{
	// Receive binary data from client. Not expected.
}

// Core processor thread
bool PictureWebSocketCommandProcessor::ProcessCommand(const std::string& commandValue,
													  ONamedParamList& parameterList)
{
	if (commandValue == "GetImage")
	{
		std::lock_guard lock(_pendingBufferCS);
		if (_spPendingImage)
			SendBuffer(_spPendingImage);
		else
			_sendNext = true;
		return true;
	}
	else
		return false;
}
