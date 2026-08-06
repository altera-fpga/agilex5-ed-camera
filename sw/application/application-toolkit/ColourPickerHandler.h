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

#include <functional>
#include "AppToolkitRawRgbImage.h"
#include "ICommand.h"

using ColourPickerUpdateCB = std::function<void(uint32_t clientId, AppToolkit::RgbPixel rgbVal)>;

class ColourPickerHandler : public IDialogHandler
{
public:
    ColourPickerHandler(std::string label, ColourPickerUpdateCB colourPickerCB, AppToolkit::RgbPixel original, bool preview);
    ~ColourPickerHandler();

    // IDialogHandler
    void ShowDialog(uint32_t clientID) override;

private:
    void ColourSelectionUpdateCB(ParamListPtr& params);

    ColourPickerUpdateCB _updateCB;
    AppToolkit::RgbPixel _selectedColour;
    std::string _label;
    bool _preview;
};
