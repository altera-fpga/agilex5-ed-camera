/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "IDialogHandler.h"

#include <vector>
#include <memory>
#include <functional>
#include "ICommand.h"
#include "CommonUiUpdate.h"
#include "UiElements.h"

class PictureCaptureHandler : public IDialogHandler
{
public:
    PictureCaptureHandler(ModalDialogUiUpdate::DialogType dialogType,
                          CapturePictureCB capturePictureCB);
    ~PictureCaptureHandler();

    // IDialogHandler
    void ShowDialog(uint32_t clientID) override;

private:
    void UpdatePictureCB(ParamListPtr& spParameterList);
    ModalDialogUiUpdate::DialogType _dialogType;
    CapturePictureCB _capturePictureCB;
	std::string _updateCommand;
};
