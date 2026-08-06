/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "PictureInPictureHandler.h"
#include "ColourPickerHandler.h"
#include "PictureCaptureHandler.h"

class IDialogFactory
{
public:
    virtual ~IDialogFactory() {}
    virtual std::shared_ptr<IDialogHandler> CreatePictureInPictureHandler(PiPLayerUpdateCB layerUpdateCB,
                                                                          std::shared_ptr<PiPDetails> spPiPDetails) = 0;
    virtual std::shared_ptr<IDialogHandler> CreateColourPickerHandler(std::string label, ColourPickerUpdateCB colourUpdateCB,
                                                                      AppToolkit::RgbPixel colourVals, bool preview) = 0;
    virtual std::shared_ptr<IDialogHandler> CreatePictureCaptureHandler(ModalDialogUiUpdate::DialogType dialogType,
                                                                        CapturePictureCB capturePictureCB) = 0;
};

class DialogFactory : public IDialogFactory
{
public:
    std::shared_ptr<IDialogHandler> CreatePictureInPictureHandler(PiPLayerUpdateCB layerUpdateCB,
                                                                  std::shared_ptr<PiPDetails> spPiPDetails) override
    {
        return std::make_shared<PictureInPictureHandler>(std::move(layerUpdateCB), std::move(spPiPDetails));
    }

    std::shared_ptr<IDialogHandler> CreateColourPickerHandler(std::string label, ColourPickerUpdateCB colourUpdateCB,
                                                              AppToolkit::RgbPixel colourVals, bool preview) override
    {
        return std::make_shared<ColourPickerHandler>(std::move(label), std::move(colourUpdateCB), std::move(colourVals), std::move(preview));
    }

    std::shared_ptr<IDialogHandler> CreatePictureCaptureHandler(ModalDialogUiUpdate::DialogType dialogType,
                                                                CapturePictureCB capturePictureCB) override
    {
        return std::make_shared<PictureCaptureHandler>(std::move(dialogType), std::move(capturePictureCB));
    }
};
