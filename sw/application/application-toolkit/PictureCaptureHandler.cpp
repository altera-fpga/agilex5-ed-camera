/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "PictureCaptureHandler.h"
#include "CommonApplicationBase.h"

PictureCaptureHandler::PictureCaptureHandler(ModalDialogUiUpdate::DialogType dialogType,
											 CapturePictureCB capturePictureCB)
:	_dialogType(dialogType)
,	_capturePictureCB(std::move(capturePictureCB))
{
    ICommandMap* pCommandMap = CommonApplicationBase::Get()->GetCommandMap();

    // Register a command to use as callback for the update picture dialog
	_updateCommand = AtUtils::FormatString("PictureDialogUpdateType%d", (uint32_t)dialogType);
    pCommandMap->Add(new UiServerCommand(_updateCommand, [this](ParamListPtr& parameterList) { UpdatePictureCB(parameterList); }));
}

PictureCaptureHandler::~PictureCaptureHandler()
{
	// Unregister command
    auto app = CommonApplicationBase::Get();

    if(app && app->GetCommandMap())
    {
        app->GetCommandMap()->Remove(_updateCommand);
    }
}

void PictureCaptureHandler::ShowDialog(uint32_t clientID)
{
    // Send command to UI to display the picture dialog
    ModalDialogUiUpdate uiUpdate(clientID, _dialogType);
    uiUpdate.Notify();
}

void PictureCaptureHandler::UpdatePictureCB(ParamListPtr& spParameterList)
{
	if (_capturePictureCB)
		_capturePictureCB();
}
