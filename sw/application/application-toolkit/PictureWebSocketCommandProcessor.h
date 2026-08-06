/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "WebSocketCommandProcessor.h"
#include "AppToolkitRawRgbImage.h"
#include "ObserverPattern.h"

class PictureWebSocketCommandProcessor : public WebSocketCommandHandler,
										 public Observer<std::shared_ptr<AppToolkit::RawRgbImage16>>
{
public:
	PictureWebSocketCommandProcessor(IWebSocketService* pIWebSocketService);
	~PictureWebSocketCommandProcessor() override;
	void Ready() override;

	// WebSocketCommandHandler interface
    bool ProcessCommand(const std::string& commandValue, ONamedParamList& parameterList) override;
    void Process(std::vector<std::shared_ptr<ByteArray>>& binaryData, bool finalFrame) override;

	// Observer interface
	bool Update(std::shared_ptr<AppToolkit::RawRgbImage16>& spImage) override;
	void DoUnregister() override {}

private:
	void SendBuffer(std::shared_ptr<AppToolkit::RawRgbImage16> spImage);

	std::recursive_mutex _pendingBufferCS;
	std::shared_ptr<AppToolkit::RawRgbImage16> _spPendingImage;
	bool _sendNext = true;
	int _dropCount = 0;
};
