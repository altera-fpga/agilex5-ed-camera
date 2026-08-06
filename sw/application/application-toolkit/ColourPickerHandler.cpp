/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "ColourPickerHandler.h"
#include "CommonApplicationBase.h"

static const char* colourPickerUpdateCommand = "ColourPickerUpdate";

ColourPickerHandler::ColourPickerHandler(std::string label,
                                         ColourPickerUpdateCB colourPickerCB,
                                         AppToolkit::RgbPixel original,
                                         bool preview)
:   _updateCB(std::move(colourPickerCB))
,   _selectedColour(original)
,   _label(std::move(label))
,   _preview(preview)
{
    ICommandMap* pCommandMap = CommonApplicationBase::Get()->GetCommandMap();
    pCommandMap->Add(new UiServerCommand(colourPickerUpdateCommand, [this](ParamListPtr& parameterList) { ColourSelectionUpdateCB(parameterList); }));
}

ColourPickerHandler::~ColourPickerHandler()
{
    ICommonApplicationBase* base = CommonApplicationBase::Get();
    if (base == nullptr)
    {
        return;
    }
    ICommandMap* commands = base->GetCommandMap();

    std::string command(colourPickerUpdateCommand);
    commands->Remove(command);
}

void ColourPickerHandler::ShowDialog(uint32_t clientID)
{
    UiUpdate::ShowColorPicker(clientID, _selectedColour, _label, _preview);
}

void ColourPickerHandler::ColourSelectionUpdateCB(ParamListPtr& spParameterList)
{
    if (spParameterList && spParameterList->CheckNumParams(3))
    {
        uint32_t temp = 0;
        AtUtils::FromString(spParameterList->at(0), temp);
        _selectedColour._red = temp;
        AtUtils::FromString(spParameterList->at(1), temp);
        _selectedColour._green = temp;
        AtUtils::FromString(spParameterList->at(2), temp);
        _selectedColour._blue = temp;

        if (_updateCB)
        {
            _updateCB(spParameterList->GetClientID(), _selectedColour);
        }
    }
}
