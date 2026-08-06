/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "UiElements.h"
#include "ParamList.h"
#include "ICommand.h"
#include "CommonApplicationBase.h"
#include <sstream>
#include "PictureCaptureHandler.h"
#include <cmath>

uint32_t UiElement::_next_unique_id = 1000;

// Specialised enum hashes required for GCC
#if defined(__GNUC__)
namespace std
{
    template <>
    struct hash<ANCHOR_TYPE>
    {
        std::size_t operator()(const ANCHOR_TYPE& enum_value) const noexcept
        {
            return (std::size_t)enum_value;
        }
    };
}
#endif


std::unordered_map<ANCHOR_TYPE, std::string, HashAnchorType> UiElement::_attachment_type_string;

UiElement::UiElement(const char* elementType)
:    _elementType(elementType)
,    _unique_id(_next_unique_id++)
,    _pLogStream{ &std::cout }
{
    if (_attachment_type_string.empty())
    {
        _attachment_type_string[ANCHOR_TYPE::PARENT_NEAR]  = "parent_near";
        _attachment_type_string[ANCHOR_TYPE::PARENT_FAR]   = "parent_far";
        _attachment_type_string[ANCHOR_TYPE::SIBLING_NEAR] = "sibling_near";
        _attachment_type_string[ANCHOR_TYPE::SIBLING_FAR]  = "sibling_far";
        _attachment_type_string[ANCHOR_TYPE::FIT_CHILDREN] = "fit_children";
        _attachment_type_string[ANCHOR_TYPE::FIXED_SIZE]   = "fixed_size";
    }
}

UiElement::~UiElement()
{
}

void UiElement::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("element_type", _elementType);
    FillJson(spJsonObject);
}

void UiElement::SetLogStream(std::ostream* pLogStream)
{
    _pLogStream = pLogStream;
}

void UiElement::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("unique_id", _unique_id);
    _existsInClient = true;
}

void UiElement::LogDataTampering(const char* functionName, uint32_t clientID)
{
    if ((_pLogStream) && (clientID != UiUpdate::invalidClient))
    {
        *_pLogStream << functionName << " - out of range value detected from client " <<
            clientID << ", possible data tampering detected\n";
    }
}

std::string UiElement::CsvExtract(std::string& sourceString)
{
    auto pos = sourceString.find(",");

    std::string value;

    if (pos == sourceString.npos)
    {
        value = sourceString;
        sourceString = "";
    }
    else
    {
        value = AtUtils::Left(sourceString, pos);
        sourceString = AtUtils::Mid(sourceString, pos + 1);
    }
    return value;
}

///////////////////////////////////////////////////////////////////////////////

UiControlContainer::UiControlContainer(const std::string& containerTitle,
                                       const std::string& settingSectionName)
:   UiElement("UiControlContainer")
,   _containerTitle(containerTitle)
,   _leftAttachment{ANCHOR_TYPE::PARENT_NEAR, 10}
,   _rightAttachment{ANCHOR_TYPE::FIXED_SIZE, 624}
,   _topAttachment{ANCHOR_TYPE::PARENT_NEAR, 10}
,   _settingSectionName(settingSectionName)
{
    if (_settingSectionName.find(" ") != _settingSectionName.npos)
    {
        // Spaces in the setting section name are not allowed.
        // Check your GetSettingsSectionName()
        assert(0);

        AtUtils::Replace(_settingSectionName, " ", "_");
    }

    if (!_settingSectionName.empty())
    {
        if (std::isdigit(_settingSectionName[0]))
        {
            // Setting section name cannot begin with a digit
            // Check your GetSettingsSectionName()
            assert(0);

            _settingSectionName = "_" + _settingSectionName;
        }
    }
}

void UiControlContainer::AddHeaderButtons(const std::vector<ControlContainerHeaderButton>& header_buttons)
{
    _header_buttons = header_buttons;

    for (auto& button: _header_buttons)
    {
        // Register a command to use as the panel reset callback
        ICommandMap* map = CommonApplicationBase::Get()->GetCommandMap();
        std::string serverCallbackName;
        if (!button._name.empty())
            serverCallbackName = AtUtils::FormatString("%s_%d", button._name.c_str(), _unique_id);
        map->Add(new UiServerCommand(serverCallbackName,
                [callback = button.GetCallback()](ParamListPtr& parameterList) { ControlContainerHeaderButton::Callback(callback, parameterList); }));
    }
}

// Forces manual positioning of the container

void UiControlContainer::SetLeftAnchor(AttachmentDetails attachmentDetails)
{
    _leftAttachment = attachmentDetails;
    _manualPositioning = true;
}

void UiControlContainer::SetTopAnchor(AttachmentDetails attachmentDetails)
{
    _topAttachment = attachmentDetails;
    _manualPositioning = true;
}

void UiControlContainer::SetRightAnchor(AttachmentDetails attachmentDetails)
{
    _rightAttachment = attachmentDetails;
    _manualPositioning = true;
}

void UiControlContainer::SetBottomAnchor(AttachmentDetails attachmentDetails)
{
    _bottomAttachment = attachmentDetails;
    _manualPositioning = true;
}

void UiControlContainer::LoadSettings(bool restoreDefaults)
{
    for (auto& spControl : _controls)
        spControl->LoadSettings(restoreDefaults);
}

AtUtils::JsonValueVariant UiControlContainer::PushValueToUI(uint32_t excludeClientID)
{
    for (auto& spControl : _controls)
    {
        spControl->PushValueToUI(excludeClientID);
    }

    return "";
}

std::shared_ptr<UiElement> UiControlContainer::Add(UiElement* pControlElement)
{
    std::shared_ptr<UiElement> add_element(pControlElement);
    _controls.push_back(add_element);
    return add_element;
}

std::shared_ptr<UiElement> UiControlContainer::Add(std::shared_ptr<UiElement> spControlElement,
                                                   const std::string& settingName,
                                                   const std::string& defaultValue)
{
    _controls.push_back(spControlElement);

    if (!settingName.empty())
    {
        auto spWarpPointsBase = std::dynamic_pointer_cast<WarpPointsBase>(spControlElement);
        if (spWarpPointsBase)
        {
            auto spSettingValue = std::make_shared<SettingValue<std::string>>(
                _settingSectionName.c_str(), settingName.c_str(), defaultValue);
            spWarpPointsBase->SetSetting(std::move(spSettingValue));
        }
    }

    auto spSeparator = std::dynamic_pointer_cast<UiControlSeparator>(spControlElement);
    if (spSeparator)
        _numSeparators++;
    else
        _numControls++;

    return spControlElement;
}

size_t UiControlContainer::GetNumControlsForHeight()
{
    return _numControls + (_numSeparators / 4);
}

void UiControlContainer::Show(bool state, bool updateNow)
{
    _show = state;

    if (updateNow)
        UiUpdate::ControlContainerUpdateShow(this);
}

// The border state can't be changed dynamically
void UiControlContainer::EnableBorder(bool state)
{
    _enableBorder = state;
}

void UiControlContainer::SetTransparent(bool state)
{
    _transparentPanel = state;
}

void UiControlContainer::SetEnabled(bool state)
{
    for(const auto& spChildControl: _controls)
    {
        std::shared_ptr<UiControlItem> spUiControlItem = std::dynamic_pointer_cast<UiControlItem>(spChildControl);

        if(spUiControlItem)
            spUiControlItem->Enable(state);
    }
}

void UiControlContainer::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    // Add container attributes
    UiElement::FillJson(spJsonObject);
    spJsonObject->AddValue("container_title", _containerTitle);
    spJsonObject->AddValue("settings_section_name", _settingSectionName);

    auto CreateAttachmentNode = [this](AtUtils::IJsonObjectPtr spParentNode,
                                       const char* nodeName,
                                       AttachmentDetails& attachmentDetails)
    {
        auto spAttachmentNode = spParentNode->AddObject(nodeName);
        spAttachmentNode->AddValue("type", _attachment_type_string[attachmentDetails._type]);
        spAttachmentNode->AddValue("sibling_id", attachmentDetails._siblingID);
        spAttachmentNode->AddValue("size_or_offset", attachmentDetails._sizeOrOffset);
        spAttachmentNode->AddValue("dynamic_position", attachmentDetails._dynamicPosition);
    };

    CreateAttachmentNode(spJsonObject, "LeftAttachment", _leftAttachment);
    CreateAttachmentNode(spJsonObject, "TopAttachment", _topAttachment);
    CreateAttachmentNode(spJsonObject, "RightAttachment", _rightAttachment);
    CreateAttachmentNode(spJsonObject, "BottomAttachment", _bottomAttachment);

    spJsonObject->AddValue("show", _show);
    spJsonObject->AddValue("enable_border", _enableBorder);
    spJsonObject->AddValue("transparent_panel", _transparentPanel);
    spJsonObject->AddValue("spacing", _spacing);

    // Add the child elements
    auto spControlsArray = spJsonObject->AddArray("Controls");
    for (auto& spChildControl : _controls)
    {
        auto spJsonChildObject = spControlsArray->AddElement()->AddObject();
        spChildControl->GetJSON(spJsonChildObject);
    }

    if (!_header_buttons.empty())
    {
        auto spHeaderButtonsArray = spJsonObject->AddArray("HeaderButtons");

        for (const auto& button : _header_buttons)
        {
            auto spHeaderButtonObject = spHeaderButtonsArray->AddElement()->AddObject();
            std::string serverCallbackName;
            if (!button._name.empty())
                serverCallbackName = AtUtils::FormatString("%s_%d", button._name.c_str(), _unique_id);
            spHeaderButtonObject->AddValue("callback_name", serverCallbackName.c_str());
            spHeaderButtonObject->AddValue("tooltip", button._tooltip.c_str());
            spHeaderButtonObject->AddValue("img", button._img.c_str());
            spHeaderButtonObject->AddValue("client_callback", button._clientCallback.c_str());
            spHeaderButtonObject->AddValue("client_callback_control_id", button._clientCallbackControlID);
        }
    }
}

void UiControlContainer::GetUiControlsFlat(AtUtils::IJsonArrayPtr& spControlsArray)
{
    for (auto& spChildControl : _controls)
    {
        auto spJsonChildObject = spControlsArray->AddElement()->AddObject();
        spChildControl->GetJSON(spJsonChildObject);
    }
}

std::shared_ptr<UiElement> UiControlContainer::GetControl(const std::string& controlLabel)
{
    for (auto& spChildControl : _controls)
    {
        std::shared_ptr<UiControlItem> spUiControlItem = std::dynamic_pointer_cast<UiControlItem>(spChildControl);
        if (spUiControlItem and (spUiControlItem->GetLabel() == controlLabel))
            return spChildControl;
    }

    return nullptr;
}

std::shared_ptr<UiElement> UiControlContainer::GetButton(const std::string& buttonLabel)
{
    for (auto& spChildControl : _controls)
    {
        std::shared_ptr<UiControlItemButton> spButton = std::dynamic_pointer_cast<UiControlItemButton>(spChildControl);
        if (spButton and (spButton->GetButtonLabel() == buttonLabel))
            return spButton;
    }

    return nullptr;
}

std::shared_ptr<UiElement> UiControlContainer::GetControl(uint32_t controlID)
{
    for (auto& spChildControl : _controls)
    {
        if (spChildControl->GetID() == controlID)
            return spChildControl;
    }

    return nullptr;
}

std::shared_ptr<UiControlSeparator> UiControlContainer::AddSeparator()
{
    auto spSeparator = std::make_shared<UiControlSeparator>();
    Add(spSeparator);
    return spSeparator;
}

std::shared_ptr<ISettingsSection> UiControlContainer::GetSettings()
{
    return CommonApplicationBase::Get()->GetSettings()->GetSection(_settingSectionName);
}

std::shared_ptr<UiControlItemBoolean> UiControlContainer::AddLedControl(const std::string& label,
                                                                         bool value,
                                                                         BooleanControlCB callback,
                                                                         eUILedMode ledMode)
{
    auto spSettingValue = std::make_shared<SettingValue<bool>>(value);
    auto spBoolControl = std::make_shared<UiControlItemBoolean>(label, callback, spSettingValue, ledMode);
    Add(spBoolControl);
    return spBoolControl;
}

std::shared_ptr<UiControlItemBoolean> UiControlContainer::AddLedControl(const std::string& label,
                                                                         BooleanControlCB callback,
                                                                         const char* settingName,
                                                                         bool defaultValue,
                                                                         eUILedMode ledMode)
{
    auto spSettingValue = std::make_shared<SettingValue<bool>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spBoolControl = std::make_shared<UiControlItemBoolean>(label, callback, spSettingValue, ledMode);
    Add(spBoolControl);
    return spBoolControl;
}

std::shared_ptr<UiControlItemBoolean> UiControlContainer::AddBoolControl(const std::string& label,
                                                                         bool value,
                                                                         BooleanControlCB callback)
{
    auto spSettingValue = std::make_shared<SettingValue<bool>>(value);
    auto spBoolControl = std::make_shared<UiControlItemBoolean>(label, callback, spSettingValue);
    Add(spBoolControl);
    return spBoolControl;
}

std::shared_ptr<UiControlItemBoolean> UiControlContainer::AddBoolControl(const std::string& label,
                                                                         BooleanControlCB callback,
                                                                         const char* settingName,
                                                                         bool defaultValue)
{
    auto spSettingValue = std::make_shared<SettingValue<bool>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spBoolControl = std::make_shared<UiControlItemBoolean>(label, callback, spSettingValue);
    Add(spBoolControl);
    return spBoolControl;
}

std::shared_ptr<UiControlItemSlider> UiControlContainer::AddSliderControl(const std::string& label,
                                                                          int32_t value,
                                                                          int32_t minimumValue,
                                                                          int32_t maximumValue,
                                                                          SliderControlCB callback)
{
    auto spSettingValue = std::make_shared<SettingValue<int32_t>>(value);
    auto spSliderControl = std::make_shared<UiControlItemSlider>(label, minimumValue, maximumValue, callback, spSettingValue);
    Add(spSliderControl);
    return spSliderControl;
}

std::shared_ptr<UiControlItemSlider> UiControlContainer::AddSliderControl(const std::string& label,
                                                                          int32_t minimumValue,
                                                                          int32_t maximumValue,
                                                                          SliderControlCB callback,
                                                                          const char* settingName,
                                                                          int32_t defaultValue)
{
    auto spSettingValue = std::make_shared<SettingValue<int32_t>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spSliderControl = std::make_shared<UiControlItemSlider>(label,
                                                                 minimumValue,
                                                                 maximumValue,
                                                                 callback,
                                                                 spSettingValue);
    Add(spSliderControl);
    return spSliderControl;
}

std::shared_ptr<UiControlItemSlider> UiControlContainer::AddSliderControl(const std::string& label,
                                                                          float value,
                                                                          float minimumValue,
                                                                          float maximumValue,
                                                                          FloatControlCB callback,
                                                                          uint32_t showDecimalPlaces)
{
    auto spSettingValue = std::make_shared<SettingValue<float>>(value);
    auto spSliderControl = std::make_shared<UiControlItemSlider>(label,
                                                                 minimumValue,
                                                                 maximumValue,
                                                                 showDecimalPlaces,
                                                                 callback,
                                                                 spSettingValue);
    Add(spSliderControl);
    return spSliderControl;
}

std::shared_ptr<UiControlItemSlider> UiControlContainer::AddSliderControl(const std::string& label,
                                                                          float minimumValue,
                                                                          float maximumValue,
                                                                          FloatControlCB callback,
                                                                          const char* settingName,
                                                                          float defaultValue,
                                                                          uint32_t showDecimalPlaces)
{
    auto spSettingValue = std::make_shared<SettingValue<float>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spSliderControl = std::make_shared<UiControlItemSlider>(label,
                                                                 minimumValue,
                                                                 maximumValue,
                                                                 showDecimalPlaces,
                                                                 callback,
                                                                 spSettingValue);
    Add(spSliderControl);
    return spSliderControl;
}

std::shared_ptr<UiControlItemEnum> UiControlContainer::AddEnumControl(const std::string& label,
                                                                      uint32_t selectedItemValue,
                                                                      const std::vector<UiEnumOption>& options,
                                                                      EnumControlCB callback,
                                                                      uint32_t controlUserData)
{
    auto spSettingValue = std::make_shared<SettingValue<uint32_t>>(selectedItemValue);
    auto spEnumControl = std::make_shared<UiControlItemEnum>(label,
                                                             options,
                                                             callback,
                                                             spSettingValue,
                                                             controlUserData);
    Add(spEnumControl);
    return spEnumControl;
}

std::shared_ptr<UiControlItemEnum> UiControlContainer::AddEnumControl(const std::string& label,
                                                                      const std::vector<UiEnumOption>& options,
                                                                      EnumControlCB callback,
                                                                      const char* settingName,
                                                                      uint32_t defaultValue,
                                                                      uint32_t controlUserData)
{
    auto spSettingValue = std::make_shared<SettingValue<uint32_t>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spEnumControl = std::make_shared<UiControlItemEnum>(label, options, callback, spSettingValue, controlUserData);
    Add(spEnumControl);
    return spEnumControl;
}

std::shared_ptr<UiControlItemEnumRadio> UiControlContainer::AddEnumRadioControl(const std::vector<UiEnumOption>& options,
                                                                                EnumControlCB callback,
                                                                                const char* settingName,
                                                                                uint32_t defaultValue,
                                                                                uint32_t controlUserData)
{
    auto spSettingValue = std::make_shared<SettingValue<uint32_t>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spEnumRadioControl = std::make_shared<UiControlItemEnumRadio>(options, callback, spSettingValue, controlUserData);
    Add(spEnumRadioControl);
    return spEnumRadioControl;
}

std::shared_ptr<UiControlItemInteger> UiControlContainer::AddIntegerControl(const std::string& label,
                                                                            int32_t value,
                                                                            int32_t minimumValue,
                                                                            int32_t maximumValue,
                                                                            IntegerControlCB callback,
                                                                            bool hex,
                                                                            uint32_t increment)
{
    auto spSettingValue = std::make_shared<SettingValue<int32_t>>(value);
    auto spIntegerControl = std::make_shared<UiControlItemInteger>(label, "", "",
                                                                   minimumValue, maximumValue,
                                                                   callback,
                                                                   spSettingValue,
                                                                   hex,
                                                                   increment);
    Add(spIntegerControl);
    return spIntegerControl;
}

std::shared_ptr<UiControlItemInteger> UiControlContainer::AddIntegerControl(const std::string& label,
                                                                            int32_t minimumValue,
                                                                            int32_t maximumValue,
                                                                            IntegerControlCB callback,
                                                                            const char* settingName,
                                                                            int32_t defaultValue,
                                                                            bool hex,
                                                                            uint32_t increment)
{
    auto spSettingValue = std::make_shared<SettingValue<int32_t>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spIntegerControl = std::make_shared<UiControlItemInteger>(label, "", "",
                                                                   minimumValue, maximumValue,
                                                                   callback,
                                                                   spSettingValue,
                                                                   hex,
                                                                   increment);
    Add(spIntegerControl);

    return spIntegerControl;
}

std::shared_ptr<UiControlItemInteger> UiControlContainer::AddIntegerControl(const std::string& label,
                                                                            const std::string& valuePrefix,
                                                                            const std::string& valuePostfix,
                                                                            int32_t value,
                                                                            int32_t minimumValue,
                                                                            int32_t maximumValue,
                                                                            IntegerControlCB callback,
                                                                            bool hex,
                                                                            uint32_t increment)
{
    auto spSettingValue = std::make_shared<SettingValue<int32_t>>(value);
    auto spIntegerControl = std::make_shared<UiControlItemInteger>(label, valuePrefix, valuePostfix,
                                                                   minimumValue, maximumValue,
                                                                   callback,
                                                                   spSettingValue,
                                                                   hex,
                                                                   increment);
    Add(spIntegerControl);
    return spIntegerControl;
}

std::shared_ptr<UiControlItemInteger> UiControlContainer::AddIntegerControl(const std::string& label,
                                                                            const std::string& valuePrefix,
                                                                            const std::string& valuePostfix,
                                                                            int32_t minimumValue,
                                                                            int32_t maximumValue,
                                                                            IntegerControlCB callback,
                                                                            const char* settingName,
                                                                            int32_t defaultValue,
                                                                            bool hex,
                                                                            uint32_t increment)
{
    auto spSettingValue = std::make_shared<SettingValue<int32_t>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spIntegerControl = std::make_shared<UiControlItemInteger>(label, valuePrefix, valuePostfix,
                                                                   minimumValue, maximumValue,
                                                                   callback,
                                                                   spSettingValue,
                                                                   hex,
                                                                   increment);
    Add(spIntegerControl);

    return spIntegerControl;
}

std::shared_ptr<UiControlItemUInteger> UiControlContainer::AddUIntegerControl(const std::string& label,
                                                                              uint32_t value,
                                                                              uint32_t minimumValue,
                                                                              uint32_t maximumValue,
                                                                              UIntegerControlCB callback,
                                                                              bool hex,
                                                                              uint32_t increment)
{
    std::string prefix;
    if (hex)
        prefix = "0x";
    auto spSettingValue = std::make_shared<SettingValue<uint32_t>>(value);
    auto spUIntegerControl = std::make_shared<UiControlItemUInteger>(label, prefix, "",
                                                                     minimumValue, maximumValue,
                                                                     callback,
                                                                     spSettingValue,
                                                                     hex,
                                                                     increment);
    Add(spUIntegerControl);
    return spUIntegerControl;
}

std::shared_ptr<UiControlItemUInteger> UiControlContainer::AddUIntegerControl(const std::string& label,
                                                                              uint32_t minimumValue,
                                                                              uint32_t maximumValue,
                                                                              UIntegerControlCB callback,
                                                                              const char* settingName,
                                                                              uint32_t defaultValue,
                                                                              bool hex,
                                                                              uint32_t increment)
{
    auto spSettingValue = std::make_shared<SettingValue<uint32_t>>(_settingSectionName.c_str(), settingName, defaultValue);
    std::string prefix;
    if (hex)
        prefix = "0x";
    auto spUIntegerControl = std::make_shared<UiControlItemUInteger>(label, prefix, "",
                                                                     minimumValue, maximumValue,
                                                                     callback,
                                                                     spSettingValue,
                                                                     hex,
                                                                     increment);
    Add(spUIntegerControl);

    return spUIntegerControl;
}

std::shared_ptr<UiControlItemFloat> UiControlContainer::AddFloatControl(const std::string& label,
                                                                        float value,
                                                                        float minimumValue,
                                                                        float maximumValue,
                                                                        FloatControlCB callback)
{
    auto spSettingValue = std::make_shared<SettingValue<float>>(value);
    auto spFloatControl = std::make_shared<UiControlItemFloat>(label,
                                                               minimumValue,
                                                               maximumValue,
                                                               callback,
                                                               spSettingValue);
    Add(spFloatControl);
    return spFloatControl;
}

std::shared_ptr<UiControlItemFloat> UiControlContainer::AddFloatControl(const std::string& label,
                                                                        float minimumValue,
                                                                        float maximumValue,
                                                                        FloatControlCB callback,
                                                                        const char* settingName,
                                                                        float defaultValue)
{
    auto spSettingValue = std::make_shared<SettingValue<float>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spFloatControl = std::make_shared<UiControlItemFloat>(label,
                                                               minimumValue,
                                                               maximumValue,
                                                               callback,
                                                               spSettingValue);
    Add(spFloatControl);
    return spFloatControl;
}


std::shared_ptr<UiControlItemText> UiControlContainer::AddTextControl(const std::string& label,
                                                                      const std::string& value,
                                                                      TextControlCB callback)
{
    auto spSettingValue = std::make_shared<SettingValue<std::string>>(value);
    auto spTextControl = std::make_shared<UiControlItemText>(label, callback, spSettingValue);
    Add(spTextControl);
    return spTextControl;
}

std::shared_ptr<UiControlItemText> UiControlContainer::AddTextControl(const std::string& label,
                                                                      const std::string& valuePrefix,
                                                                      const std::string& valuePostfix,
                                                                      const std::string& value,
                                                                      TextControlCB callback)
{
    auto spSettingValue = std::make_shared<SettingValue<std::string>>(value);
    auto spTextControl = std::make_shared<UiControlItemText>(label, valuePrefix, valuePostfix, callback, spSettingValue);
    Add(spTextControl);
    return spTextControl;
}

// Full width value, no label
std::shared_ptr<UiControlItemText> UiControlContainer::AddTextControl(const std::string& value,
                                                                      TextControlCB callback)
{
    auto spSettingValue = std::make_shared<SettingValue<std::string>>(value);
    auto spTextControl = std::make_shared<UiControlItemText>(callback, spSettingValue);
    Add(spTextControl);
    return spTextControl;
}

std::shared_ptr<UiControlItemText> UiControlContainer::AddTextControl(const std::string& label,
                                                                      TextControlCB callback,
                                                                      const char* settingName,
                                                                      const std::string& defaultValue)
{
    auto spSettingValue = std::make_shared<SettingValue<std::string>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spTextControl = std::make_shared<UiControlItemText>(label,
                                                             callback,
                                                             spSettingValue);
    Add(spTextControl);
    return spTextControl;
}

std::shared_ptr<UiControlItemText> UiControlContainer::AddTextControl(const std::string& label,
                                                                      const std::string& valuePrefix,
                                                                      const std::string& valuePostfix,
                                                                      TextControlCB callback,
                                                                      const char* settingName,
                                                                      const std::string& defaultValue)
{
    auto spSettingValue = std::make_shared<SettingValue<std::string>>(_settingSectionName.c_str(), settingName, defaultValue);
    auto spTextControl = std::make_shared<UiControlItemText>(label,
                                                             valuePrefix,
                                                             valuePostfix,
                                                             callback,
                                                             spSettingValue);
    Add(spTextControl);
    return spTextControl;
}


std::shared_ptr<UiControlItemButton> UiControlContainer::AddButtonControl(const std::string& title,
                                                                          const std::string& buttonLabel,
                                                                          ButtonControlCB callback)
{
    auto spButtonControl = std::make_shared<UiControlItemButton>(title, buttonLabel, callback);
    Add(spButtonControl);
    return spButtonControl;
}

std::shared_ptr<UiControlItemButton> UiControlContainer::AddButtonControl(const std::string& buttonLabel,
                                                                          ButtonControlCB callback)
{
    return AddButtonControl("", buttonLabel, std::move(callback));
}

std::shared_ptr<UiControlItemLabel> UiControlContainer::AddLabelControl(const std::string& label)
{
    auto spLabelControl = std::make_shared<UiControlItemLabel>("", label);
    Add(spLabelControl);
    return spLabelControl;
}

std::shared_ptr<UiControlItemLabel> UiControlContainer::AddLabelControl(const std::string& label, const std::string& value)
{
    auto spLabelControl = std::make_shared<UiControlItemLabel>(label, value);
    Add(spLabelControl);
    return spLabelControl;
}

std::shared_ptr<UiControlItemPicture> UiControlContainer::AddPictureControl()
{
    auto spPictureControl = std::make_shared<UiControlItemPicture>();
    Add(spPictureControl);
    return spPictureControl;
}

std::shared_ptr<UiControlItemButton> UiControlContainer::AddPictureInPictureControl(const std::string& label,
                                                                                    PiPLayerUpdateCB layerUpdateCB,
                                                                                    std::shared_ptr<PiPDetails> spPiPDetails)
{
    auto pipHandler = CommonApplicationBase::Get()->GetDialogFactory()->CreatePictureInPictureHandler(std::move(layerUpdateCB), std::move(spPiPDetails));
    auto openResizerUiCB = [handler = std::move(pipHandler)](uint32_t clientID) { handler->ShowDialog(clientID); };
    return AddButtonControl(label, openResizerUiCB);
}

std::shared_ptr<UiControlItemButton> UiControlContainer::AddColourPickerControl(const std::string& label, ColourPickerUpdateCB colourUpdateCB, AppToolkit::RgbPixel colourVals, bool preview)
{
     auto colourPickHandler = CommonApplicationBase::Get()->GetDialogFactory()->CreateColourPickerHandler(label, std::move(colourUpdateCB),
                                                                                                      colourVals, preview);
     auto openColourPickerCB = [handler = std::move(colourPickHandler)](uint32_t clientID) { handler->ShowDialog(clientID); };
     return AddButtonControl(label, openColourPickerCB);
}

std::shared_ptr<UiControlItemButton> UiControlContainer::AddPictureCaptureControl(const std::string& label,
                                                                                  ModalDialogUiUpdate::DialogType dialogType,
                                                                                  CapturePictureCB capturePictureCB)
{
    auto pictureCaptureHandler = CommonApplicationBase::Get()->GetDialogFactory()->CreatePictureCaptureHandler(dialogType,
                                                                                                               std::move(capturePictureCB));
    auto openDialogCB = [handler = std::move(pictureCaptureHandler)](uint32_t clientID) { handler->ShowDialog(clientID); };
    return AddButtonControl(label, openDialogCB);
}

///////////////////////////////////////////////////////////////////////////////

UiControlItem::UiControlItem(const char* elementType)
:    UiElement(elementType)
{
    RegisterServerCommand();
}

UiControlItem::UiControlItem(const char* elementType,
                             const std::string& label, uint32_t nLines)
:    UiElement(elementType)
,    _label(label)
,    _nLines(nLines)
{
    RegisterServerCommand();
}

UiControlItem::UiControlItem(const char* elementType,
                             const std::string& label,
                             const std::string& valuePrefix,
                             const std::string& valuePostfix,
                             uint32_t nLines)
:    UiElement(elementType)
,    _label(label)
,    _valuePrefix(valuePrefix)
,    _valuePostfix(valuePostfix)
,    _nLines(nLines)
{
    RegisterServerCommand();
}

void UiControlItem::RegisterServerCommand()
{
    ICommandMap* map = CommonApplicationBase::Get()->GetCommandMap();
    _serverCallbackName = AtUtils::FormatString("ControlItemUpdate_%d", _unique_id);
    map->Add(new UiServerCommand(_serverCallbackName.c_str(), [this](ParamListPtr& spParameterList) { UpdateCB(spParameterList); }));
}

void UiControlItem::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiElement::FillJson(spJsonObject);
    spJsonObject->AddValue("label", _label);
    spJsonObject->AddValue("value_prefix", _valuePrefix);
    spJsonObject->AddValue("value_postfix", _valuePostfix);
    spJsonObject->AddValue("server_callback_name", _serverCallbackName);
    spJsonObject->AddValue("client_callback_name", _clientCallbackName);
    spJsonObject->AddValue("client_callback_owner_id", _clientCallbackOwnerID);
    spJsonObject->AddValue("num_lines", _nLines);
    spJsonObject->AddValue("enabled", _enabled);
    spJsonObject->AddValue("tooltip", _tooltip);
    spJsonObject->AddValue("show", _shown);
    spJsonObject->AddValue("control_x", _controlX);
    spJsonObject->AddValue("full_width_value", _fullWidthValue);
    spJsonObject->AddValue("read_only", _readOnly);

    if (!_styleSettings.empty())
    {
        auto spStyleArray = spJsonObject->AddArray("Style");

        for (auto& styleSetting : _styleSettings)
        {
            auto spStyleObject = spStyleArray->AddElement()->AddObject();
            spStyleObject->AddValue("setting", styleSetting._setting);
            spStyleObject->AddValue("value", styleSetting._value);
        }
    }

    for (auto& additionalAttribute : _additionalAttributes)
        spJsonObject->AddValue(additionalAttribute._name.c_str(), additionalAttribute._value);
}

void UiControlItem::UpdateCB(ParamListPtr& spParameterList)
{
    if (_enabled)
        ForwardUpdateCB(spParameterList->GetClientID(), spParameterList->GetCSV());
}

UiControlItem* UiControlItem::Enable(bool state)
{
    _enabled = state;

    if (_existsInClient)
        UiUpdate::ControlItemUpdateEnabled(this);

    return this;
}

UiControlItem* UiControlItem::Show(bool state)
{
    _shown = state;

    if (_existsInClient)
        UiUpdate::ControlItemUpdateShow(this);

    return this;
}

UiControlItem* UiControlItem::ReadOnly(bool state)
{
    _readOnly = state;

    if (_existsInClient)
        UiUpdate::ControlItemUpdateShow(this);

    return this;
}

UiControlItem* UiControlItem::SetClientCallbackName(uint32_t ownerID, const std::string& clientCallbackName)
{
    // The function called clientCallbackName is called on the object
    // identified by pOwner in the client
    _clientCallbackName = clientCallbackName;
    _clientCallbackOwnerID = ownerID;
    return this;
}

void UiControlItem::LoadSettings(bool restoreFromDefaults)
{
    UpdateFromSettings(restoreFromDefaults);
}

UiControlItem* UiControlItem::UpdateLabel(const std::string& newLabel)
{
    _label = newLabel;
    UiUpdate::ControlItemUpdateLabel(this, _label);
    return this;
}

UiControlItem *UiControlItem::SetValuePreAndPostfix(const std::string& prefix,
                                                    const std::string& postfix)
{
    _valuePrefix = prefix;
    _valuePostfix = postfix;
    return this;
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemLabel::UiControlItemLabel(const std::string& label,
                                       const std::string& valueString,
                                       uint32_t nLines)
    : UiControlItem("UiControlItemLabel", label, nLines)
    , _value(valueString)
{
}

void UiControlItemLabel::FillJson(AtUtils::IJsonObjectPtr &spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value);
}

AtUtils::JsonValueVariant UiControlItemLabel::UpdateValue(const std::string& value)
{
    _value = value;

    // Reflect the validated value to all clients
    UiElementValueUpdate update_ui(this, _value);
    return _value;
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemSlider::UiControlItemSlider(const std::string& label,
                                         int32_t minimum,
                                         int32_t maximum,
                                         SliderControlCB callback,
                                         std::shared_ptr<ISettingValue<int32_t>> spSetting)
:    UiControlItem("UiControlItemSlider", label)
,    _value(spSetting->Get())
,    _minimum(minimum)
,    _maximum(maximum)
,    _callback(std::move(callback))
,    _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

UiControlItemSlider::UiControlItemSlider(const std::string& label,
                                         float minimum,
                                         float maximum,
                                         uint32_t showDecimalPlaces,
                                         FloatControlCB callback,
                                         std::shared_ptr<ISettingValue<float>> spSetting)
:    UiControlItem("UiControlItemFloatSlider", label)
,   _floatValue(spSetting->Get())
,   _floatMinimum(minimum)
,   _floatMaximum(maximum)
,   _showDecimalPlaces(showDecimalPlaces)
,   _floatCallback(std::move(callback))
,   _spFloatSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spFloatSetting->IsValid())
        _floatCallback(UiUpdate::invalidClient, _floatValue);
}

void UiControlItemSlider::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    if (_callback)
    {
        int32_t newValue;
        AtUtils::FromString(value, newValue);
        UpdateValue(newValue, true, clientId);
    }
    else if (_floatCallback)
    {
        float newValue;
        AtUtils::FromString(value, newValue);
        UpdateValue(newValue, true, clientId);
    }
}

AtUtils::JsonValueVariant UiControlItemSlider::UpdateValue(int32_t value, bool callCallback, uint32_t fromClientId)
{
    _value = value;

    if (_value < _minimum)
    {
        LogDataTampering("UiControlItemSlider::UpdateValue", fromClientId);
        _value = _minimum;
    }
    if (_value > _maximum)
    {
        LogDataTampering("UiControlItemSlider::UpdateValue", fromClientId);
        _value = _maximum;
    }

    if (callCallback && _callback)
        _callback(fromClientId, _value);

    if (_spSetting)
        _spSetting->Set(_value);

    // Reflect the validated value to all other clients
    return PushValueToUI(fromClientId);
}

AtUtils::JsonValueVariant UiControlItemSlider::UpdateValue(float value, bool callCallback, uint32_t fromClientId)
{
    _floatValue = value;

    if (_floatValue < _floatMinimum)
    {
        LogDataTampering("UiControlItemSlider::UpdateValue", fromClientId);
        _floatValue = _floatMinimum;
    }
    if (_floatValue > _floatMaximum)
    {
        LogDataTampering("UiControlItemSlider::UpdateValue", fromClientId);
        _floatValue = _floatMaximum;
    }

    if (callCallback && _floatCallback)
        _floatCallback(fromClientId, _floatValue);

    if (_spFloatSetting)
        _spFloatSetting->Set(_floatValue);

    // Reflect the validated value to all other clients
    return PushValueToUI(fromClientId);
}

AtUtils::JsonValueVariant UiControlItemSlider::PushValueToUI(uint32_t excludeClientID)
{
    if (_floatCallback)
    {
        UiElementValueUpdate(this, _floatValue, false, excludeClientID);
        return _floatValue;
    }
    else
    {
        UiElementValueUpdate(this, _value, false, excludeClientID);
        return _value;
    }
}

void UiControlItemSlider::FillJson(AtUtils::IJsonObjectPtr &spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    if (_callback)
    {
        spJsonObject->AddValue("value", _value);
        spJsonObject->AddValue("minimum", _minimum);
        spJsonObject->AddValue("maximum", _maximum);
    }
    else if (_floatCallback)
    {
        spJsonObject->AddValue("value", _floatValue);
        spJsonObject->AddValue("minimum", _floatMinimum);
        spJsonObject->AddValue("maximum", _floatMaximum);
    }

    spJsonObject->AddValue("show_decimal_places", _showDecimalPlaces);
    spJsonObject->AddValue("convert_value_to_display_owner_id", _valueToDisplayOwnerID);
    spJsonObject->AddValue("convert_value_to_display_fn_name", _valueToDisplayFn);
    spJsonObject->AddValue("convert_display_to_value_owner_id", _displayToValueOwnerID);
    spJsonObject->AddValue("convert_display_to_value_fn_name", _displayToValueFn);
}

void UiControlItemSlider::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_callback && _spSetting && _spSetting->IsValid())
    {
        _value = _spSetting->Get(restoreFromDefaults);

        if (_value < _minimum)
            _value = _minimum;
        if (_value > _maximum)
            _value = _maximum;

        // Update the app with the value taken from settings
        _callback(UiUpdate::invalidClient, _value);

        // Reflect the validated value to all clients
        UiElementValueUpdate update_ui(this, _value);
    }
    else if (_floatCallback && _spFloatSetting && _spFloatSetting->IsValid())
    {
        _floatValue = _spFloatSetting->Get(restoreFromDefaults);

        if (_floatValue < _floatMinimum)
            _floatValue = _floatMinimum;
        if (_floatValue > _floatMaximum)
            _floatValue = _floatMaximum;

        // Update the app with the value taken from settings
        _floatCallback(UiUpdate::invalidClient, _floatValue);

        // Reflect the validated value to all clients
        UiElementValueUpdate update_ui(this, _floatValue);

    }
}

void UiControlItemSlider::SetValueToDisplayConverters(uint32_t ownerID,
                                                      const std::string& valueToDisplayFn,
                                                      const std::string& displayToValueFn)
{
    _valueToDisplayOwnerID = ownerID;
    _displayToValueOwnerID = ownerID;
    _valueToDisplayFn = valueToDisplayFn;
    _displayToValueFn = displayToValueFn;
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemInteger::UiControlItemInteger(const std::string& label,
                                           const std::string& prefix,
                                           const std::string& postfix,
                                           int32_t minimum,
                                           int32_t maximum,
                                           IntegerControlCB callback,
                                           std::shared_ptr<ISettingValue<int32_t>> spSetting,
                                           bool hex,
                                           uint32_t increment)
:   UiControlItem("UiControlItemInteger", label, prefix, postfix)
,   _value(spSetting->Get())
,   _minimum(minimum)
,   _maximum(maximum)
,   _hex(hex)
,   _increment(increment)
,   _callback(std::move(callback))
,   _spSetting(spSetting)
{
    // Push the value from the setting to the callback
    if (spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

void UiControlItemInteger::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    int32_t newValue;
    AtUtils::FromString(value, newValue);
    UpdateValue(newValue, true, clientId);
}

AtUtils::JsonValueVariant UiControlItemInteger::UpdateValue(int32_t value, bool callCallback, uint32_t fromClientId)
{
    _value = value;

    if (_value < _minimum)
    {
        LogDataTampering("UiControlItemInteger::UpdateValue", fromClientId);
        _value = _minimum;
    }
    if (_value > _maximum)
    {
        LogDataTampering("UiControlItemInteger::UpdateValue", fromClientId);
        _value = _maximum;
    }

    if (callCallback && _callback)
        _callback(fromClientId, _value);

    if (_spSetting)
        _spSetting->Set(_value);

    // Reflect the validated value to all clients
    return PushValueToUI();
}

AtUtils::JsonValueVariant UiControlItemInteger::PushValueToUI(uint32_t excludeClientID)
{
    UiElementValueUpdate updateUI(this, _value, false, excludeClientID);
    return _value;
}

void UiControlItemInteger::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value);
    spJsonObject->AddValue("minimum", _minimum);
    spJsonObject->AddValue("maximum", _maximum);
    spJsonObject->AddValue("hex", _hex);
    spJsonObject->AddValue("increment", _increment);
}

void UiControlItemInteger::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        _value = _spSetting->Get(restoreFromDefaults);

        if (_value < _minimum)
            _value = _minimum;
        if (_value > _maximum)
            _value = _maximum;

        // Update the app with the value taken from settings
        if (_callback)
            _callback(UiUpdate::invalidClient, _value);

        // Reflect the validated value to all clients
        UiElementValueUpdate update_ui(this, _value);
    }
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemUInteger::UiControlItemUInteger(const std::string& label,
                                             const std::string& prefix,
                                             const std::string& postfix,
                                             uint32_t minimum,
                                             uint32_t maximum,
                                             UIntegerControlCB callback,
                                             std::shared_ptr<ISettingValue<uint32_t>> spSetting,
                                             bool hex,
                                             uint32_t increment)
:    UiControlItem("UiControlItemInteger", label, prefix, postfix)
,    _value(spSetting->Get())
,    _minimum(minimum)
,    _maximum(maximum)
,    _increment(increment)
,    _callback(std::move(callback))
,    _spSetting(spSetting)
,    _hex(hex)
{
    // Push the value from the setting to the callback
    if (spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

void UiControlItemUInteger::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    uint32_t newValue;
    AtUtils::FromString(value, newValue);
    UpdateValue(newValue, true, clientId);
}

AtUtils::JsonValueVariant UiControlItemUInteger::UpdateValue(uint32_t value, bool callCallback, uint32_t fromClientId)
{
    _value = value;

    if (_value < _minimum)
    {
        LogDataTampering("UiControlItemInteger::UpdateValue", fromClientId);
        _value = _minimum;
    }
    if (_value > _maximum)
    {
        LogDataTampering("UiControlItemInteger::UpdateValue", fromClientId);
        _value = _maximum;
    }

    if (callCallback && _callback)
        _callback(fromClientId, _value);

    if (_spSetting)
        _spSetting->Set(_value);

    // Reflect the validated value to all clients
    UiElementValueUpdate update_ui(this, _value);
    return _value;    
}

AtUtils::JsonValueVariant UiControlItemUInteger::PushValueToUI(uint32_t excludeClientID)
{
    UiElementValueUpdate updateUI(this, _value, false, excludeClientID);
    return _value;
}

void UiControlItemUInteger::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value);
    spJsonObject->AddValue("minimum", _minimum);
    spJsonObject->AddValue("maximum", _maximum);
    spJsonObject->AddValue("hex", _hex);
    spJsonObject->AddValue("increment", _increment);
}

void UiControlItemUInteger::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        _value = _spSetting->Get(restoreFromDefaults);

        if (_value < _minimum)
            _value = _minimum;
        if (_value > _maximum)
            _value = _maximum;

        // Update the app with the value taken from settings
        if (_callback)
            _callback(UiUpdate::invalidClient, _value);

        // Reflect the validated value to all clients
        UiElementValueUpdate update_ui(this, _value);
    }
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemBoolean::UiControlItemBoolean(const std::string& label,
                                           BooleanControlCB callback,
                                           std::shared_ptr<ISettingValue<bool>> spSetting,
                                           eUILedMode ledMode)
:    UiControlItem("UiControlItemBoolean", label)
,    _value(spSetting->Get())
,    _ledMode(ledMode)
,    _callback(std::move(callback))
,    _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

void UiControlItemBoolean::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    bool newValue;
    AtUtils::FromString(value, newValue);
    UpdateValue(newValue, true, clientId);
}

AtUtils::JsonValueVariant UiControlItemBoolean::UpdateValue(bool value, bool callCallback, uint32_t fromClientId)
{
    _value = value;

    if (callCallback && _callback)
        _callback(fromClientId, _value);

    if (_spSetting)
        _spSetting->Set(_value);

    // Reflect the validated value to all clients
    return PushValueToUI();
}

AtUtils::JsonValueVariant UiControlItemBoolean::PushValueToUI(uint32_t excludeClientID)
{
    UiElementValueUpdate updateUI(this, _value, false, excludeClientID);
    return _value;
}

void UiControlItemBoolean::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value);
    std::string ledModeStr;
    switch( _ledMode )
    {
        case kUILedMode_Green:
            ledModeStr = "green";
            break;
        case kUILedMode_Red:
            ledModeStr = "red";
            break;
        case kUILedMode_Blue:
            ledModeStr = "blue";
            break;
        default:
        case kUiLedMode_None:
            ledModeStr = "none";
            break;
    }
    spJsonObject->AddValue("_ledMode", ledModeStr);
}

void UiControlItemBoolean::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        _value = _spSetting->Get(restoreFromDefaults);

        // Update the app with the value taken from settings
        if (_callback)
            _callback(UiUpdate::invalidClient, _value);

        // Reflect the validated value to all clients
        UiElementValueUpdate update_ui(this, _value);
    }
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemFloat::UiControlItemFloat(const std::string& label,
                                       float minimum, float maximum,
                                       FloatControlCB callback,
                                       std::shared_ptr<ISettingValue<float>> spSetting)
:    UiControlItem("UiControlItemFloat", label)
,    _value(spSetting->Get())
,    _minimum(minimum)
,    _maximum(maximum)
,    _callback(std::move(callback))
,    _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

void UiControlItemFloat::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    float newValue;
    AtUtils::FromString(value, newValue);
    UpdateValue(newValue, true, clientId);
}

AtUtils::JsonValueVariant UiControlItemFloat::UpdateValue(float value, bool callCallback, uint32_t fromClientId)
{
    _value = value;

    if (_value < _minimum)
    {
        LogDataTampering("UiControlItemFloat::UpdateValue", fromClientId);
        _value = _minimum;
    }
    if (_value > _maximum)
    {
        LogDataTampering("UiControlItemFloat::UpdateValue", fromClientId);
        _value = _maximum;
    }

    if (callCallback && _callback)
        _callback(fromClientId, _value);

    if (_spSetting)
        _spSetting->Set(_value);

    // Reflect the validated value to all clients
    return PushValueToUI();
}

AtUtils::JsonValueVariant UiControlItemFloat::PushValueToUI(uint32_t excludeClientID)
{
    UiElementValueUpdate updateUI(this, (double)_value, false, excludeClientID);
    return (double)_value;
}

void UiControlItemFloat::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value);
    spJsonObject->AddValue("minimum", _minimum);
    spJsonObject->AddValue("maximum", _maximum);
    spJsonObject->AddValue("show_decimal_places", _showDecimalPlaces);
}

void UiControlItemFloat::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        _value = _spSetting->Get(restoreFromDefaults);

        if (_value < _minimum)
            _value = _minimum;
        if (_value > _maximum)
            _value = _maximum;

        // Update the app with the value taken from settings
        if (_callback)
            _callback(UiUpdate::invalidClient, _value);

        // Reflect the validated value to all clients
        UiElementValueUpdate update_ui(this, _value);
    }
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemEnum::UiControlItemEnum(const std::string& label,
                                     const std::vector<UiEnumOption>& enumOptions,
                                     EnumControlCB callback,
                                     std::shared_ptr<ISettingValue<uint32_t>> spSetting,
                                     uint32_t controlUserData)
:    UiControlItem("UiControlItemEnum", label)
,    _selectedIndex(0)
,    _enumOptions(enumOptions)
,    _callback(std::move(callback))
,    _spSetting(std::move(spSetting))
,    _controlUserData(controlUserData)
{
    CommonInit(_spSetting->Get());

    if (_spSetting->IsValid() && _callback)
    {
        // Update the app with the value from settings (or default)
        if (_selectedIndex < (uint32_t)_enumOptions.size())
        {
            UiEnumOption enumOption = _enumOptions[_selectedIndex];
            _callback(UiUpdate::invalidClient, enumOption, _controlUserData);
        }
    }
}

void UiControlItemEnum::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    // If value is a not an integer, then select the item by its label
    bool isInteger = true;
    for (char ch: value)
    {
        if (not std::isdigit(ch))
        {
            isInteger = false;
            break;
        }
    }

    if (isInteger)
    {
        uint32_t selectedIndex;
        AtUtils::FromString(value, selectedIndex);
        if (selectedIndex < (uint32_t)_enumOptions.size())
        {
            uint32_t selectedUserItemData = _enumOptions[selectedIndex]._userItemData;
            UpdateValue(selectedUserItemData, true, clientId);
        }
    }
    else
    {
        for (auto& option : _enumOptions)
        {
            if (option._label == value)
            {
                uint32_t selectedUserItemData = option._userItemData;
                UpdateValue(selectedUserItemData, true, clientId);
                break;
            }
        }
    }
}

AtUtils::JsonValueVariant UiControlItemEnum::UpdateValue(uint32_t selectedUserItemData,
                                                         bool callCallback,
                                                         uint32_t fromClientId)
{
    CommonInit(selectedUserItemData);

    if (callCallback && _callback)
    {
        UiEnumOption& enumOption = _enumOptions[_selectedIndex];
        _callback(fromClientId, enumOption, _controlUserData);
    }

    if (_spSetting)
        _spSetting->Set(selectedUserItemData);

    // Reflect to all clients
    return PushValueToUI();
}

AtUtils::JsonValueVariant UiControlItemEnum::PushValueToUI(uint32_t excludeClientID)
{
    UiElementValueUpdate updateUI(this, _selectedIndex, false, excludeClientID);
    return _selectedIndex;
}

void UiControlItemEnum::RemoveEnumItem(int index, const std::string& value)
{
    size_t uIndex = static_cast<size_t>(index);
    if ((uIndex < _enumOptions.size()) && (_enumOptions[uIndex]._label == value))
    {
        _enumOptions.erase(_enumOptions.begin() + uIndex);
        UiUpdate::ControlItemRemoveEnum(this, index);
    }
}

void UiControlItemEnum::AddEnumItem(const std::string& value, uint32_t userItemData)
{
    _enumOptions.push_back({value, userItemData});
    UiUpdate::ControlItemAddEnum(this, value);
}

uint32_t UiControlItemEnum::GetSelectedIndex()
{
    return _selectedIndex;
}

void UiControlItemEnum::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);

    spJsonObject->AddValue("value", _selectedIndex);
    auto spOptionsArray = spJsonObject->AddArray("Options");

    for (auto& enumOption : _enumOptions)
    {
        auto spJsonObject = spOptionsArray->AddElement()->AddObject();
        spJsonObject->AddValue("string", enumOption._label);
        if(!enumOption._imgUrl.empty())
            spJsonObject->AddValue("url", enumOption._imgUrl);
    }
}

void UiControlItemEnum::CommonInit(uint32_t selectedItemUserData)
{
    // Find the selected item index
    bool foundIndex = false;

    // If all the userdata is un-assigned, set them to the index
    bool noUserData = true;
    for (size_t i = 0, ni = _enumOptions.size(); i < ni; i++)
    {
        if (_enumOptions[i]._userItemData != (uint32_t)-1)
        {
            noUserData = false;
            break;
        }
    }

    if (noUserData)
    {
        for (size_t i = 0, ni = _enumOptions.size(); i < ni; i++)
            _enumOptions[i]._userItemData = (uint32_t)i;
    }

    for (size_t i = 0, ni = _enumOptions.size(); i < ni; i++)
    {
        _enumOptions[i]._index = (uint32_t)i;

        if (_enumOptions[i]._userItemData == selectedItemUserData)
        {
            _selectedIndex = (uint32_t)i;
            foundIndex = true;
        }
    }

    if (!foundIndex)
    {
        // The specified user data doesn't match any item,
        // so consider it to be the selected index instead
        if(selectedItemUserData < _enumOptions.size())
            _selectedIndex = selectedItemUserData;
    }
}

void UiControlItemEnum::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        uint32_t userItemData = _spSetting->Get(restoreFromDefaults);

        for(const auto& option : _enumOptions)
        {
            if(option._userItemData == userItemData)
            {
                _selectedIndex = option._index;

                // Update the app with the value taken from settings
                if (_callback)
                    _callback(UiUpdate::invalidClient, option, _controlUserData);

                // Reflect the validated value to all clients
                UiElementValueUpdate update_ui(this, _selectedIndex);
                break;
            }
        }       
    }
}

UiEnumOption UiControlItemEnum::GetOption(uint32_t index) const
{
    if (index < (uint32_t)_enumOptions.size())
        return _enumOptions[index];
    else
        return { "" };
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemText::UiControlItemText(const std::string& label,
                                     TextControlCB callback,
                                     std::shared_ptr<ISettingValue<std::string>> spSetting)
:   UiControlItem("UiControlItemText", label)
,   _value(spSetting->Get())
,   _callback(std::move(callback))
,   _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

UiControlItemText::UiControlItemText(const std::string& label,
                                     const std::string& valuePrefix,
                                     const std::string& valuePostfix,
                                     TextControlCB callback,
                                     std::shared_ptr<ISettingValue<std::string>> spSetting)
:    UiControlItem("UiControlItemText", label, valuePrefix, valuePostfix)
,   _value(spSetting->Get())
,   _callback(std::move(callback))
,   _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);
}

UiControlItemText::UiControlItemText(TextControlCB callback,
                                     std::shared_ptr<ISettingValue<std::string>> spSetting)
:    UiControlItem("UiControlItemText", "")
,    _value(spSetting->Get())
,    _callback(std::move(callback))
,    _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);

    _fullWidthValue = true;
}

UiControlItemText::UiControlItemText(TextControlCB callback,
                                     const std::string& valuePrefix,
                                     const std::string& valuePostfix,
                                     std::shared_ptr<ISettingValue<std::string>> spSetting)
:   UiControlItem("UiControlItemText", "", valuePrefix, valuePostfix)
,   _value(spSetting->Get())
,   _callback(std::move(callback))
,   _spSetting(std::move(spSetting))
{
    // Push the value from the setting to the callback
    if (_spSetting->IsValid() && _callback)
        _callback(UiUpdate::invalidClient, _value);

    _fullWidthValue = true;
}

void UiControlItemText::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    UpdateValue(value, true, clientId);
}

AtUtils::JsonValueVariant UiControlItemText::UpdateValue(const std::string& value, bool callCallback, uint32_t fromClientId)
{
    _value = value;

    if (callCallback && _callback)
        _callback(fromClientId, _value);

    if (_spSetting)
        _spSetting->Set(_value);

    // Reflect the validated value to all clients
    UiElementValueUpdate update_ui(this, _value);
    return _value;
}

AtUtils::JsonValueVariant UiControlItemText::PushValueToUI(uint32_t excludeClientID)
{
    UiElementValueUpdate updateUI(this, _value, false, excludeClientID);
    return _value;
}

void UiControlItemText::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value);
}

void UiControlItemText::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        _value = _spSetting->Get(restoreFromDefaults);

        // Update the app with the value taken from settings
        if (_callback)
            _callback(UiUpdate::invalidClient, _value);

        // Reflect the value to all clients
        UiElementValueUpdate update_ui(this, _value);
    }
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemButton::UiControlItemButton(const std::string& title,
                                         const std::string& buttonLabel,
                                         ButtonControlCB callback)
:    UiControlItem("UiControlItemButton", title)
,    _buttonLabel(buttonLabel)
,    _callback(std::move(callback))
{
}

void UiControlItemButton::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    if (_callback)
        _callback(clientId);
}

void UiControlItemButton::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("buttonLabel", _buttonLabel);
}

void UiControlItemButton::RenameButton(const std::string& newName)
{
    _buttonLabel = newName;
    UiElementValueUpdate update_ui(this, _buttonLabel);
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemPicture::UiControlItemPicture()
:    UiControlItem("UiControlItemPicture", "", 10)
{
}

void UiControlItemPicture::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    //spJsonObject->AddValue("buttonLabel", _buttonLabel);
}

///////////////////////////////////////////////////////////////////////////////

WarpPointsBase::WarpPointsBase(const char* elementType, const std::string& label,
                               const WarpPointsValue& value, WarpPointsCB callback,
                               uint32_t warpControlID)
    : UiControlItem(elementType, label)
    , _value(value)
    , _callback(std::move(callback))
    , _warpControlID(warpControlID)
{
}

WarpPointsBase::~WarpPointsBase()
{
}

void WarpPointsBase::ForwardUpdateCB(uint32_t clientId, const std::string& value)
{
    if(_callback)
    {
        if(!value.empty())
        {
            auto split_string = [](const std::string& str, const char token)
            {
                std::vector<std::string> words;

                if (!str.empty())
                {
                    std::size_t p1 = 0;

                    while (p1 < str.size())
                    {
                        std::size_t p2 = str.find(token, p1);
                        auto count = p2 - p1;
                        if(count > 0)
                            words.push_back(str.substr(p1, p2 - p1));
                        p1 = (p2 != std::string::npos ? p2 + 1 : p2);
                    }
                }

                return words;
            };

            auto params = split_string(value, ' ');

            if(params.size() == 3)
            {
                uint32_t pointId = AtUtils::FromString<uint32_t>(params[0]);
                float x = AtUtils::FromString<float>(params[1]);
                float y = AtUtils::FromString<float>(params[2]);
                UpdateValue(pointId, x, y, true, clientId);
            }
        }
    }

    return;
}

AtUtils::JsonValueVariant WarpPointsBase::UpdateValue(uint32_t pointId,
                                                      float x,
                                                      float y,
                                                      bool callCallback,
                                                      uint32_t fromClientId)
{
    std::string stringValue;

    if (pointId < _value._points.size())
    {
        _value._points[pointId] = {x,y};

        if (callCallback && _callback)
            _callback(fromClientId, (uint32_t)_value._points.size(), pointId, x, y);

        stringValue = _value.ToString(false, (int32_t)pointId);

        if (_spSetting)
        {
            // Save CSV value to settings
            std::string stringValue = _value.ToString(true);
            _spSetting->Set(stringValue);
        }

        // Reflect the validated value to all other clients
        UiElementValueUpdate update_ui(this, stringValue, true, fromClientId);
    }

    return stringValue;
}

AtUtils::JsonValueVariant WarpPointsBase::UpdateValue(const WarpPointsValue& value,
                                                      bool callCallback,
                                                      uint32_t fromClientId)
{
    _value = value;

    if (callCallback && _callback)
    {
        uint32_t i = 0;
        uint32_t numKnots = (uint32_t)_value._points.size();
        for (const auto& [x, y] : _value._points)
        {
            _callback(fromClientId, numKnots, i, x, y);
            ++i;
        }
    }

    if (_spSetting)
    {
        // Save CSV value to settings
        std::string stringValue = _value.ToString(true);
        _spSetting->Set(stringValue);
    }

    return PushValueToUI(fromClientId);
}

void WarpPointsBase::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    spJsonObject->AddValue("value", _value.ToString(false))->SetStringIsObject();
    spJsonObject->AddValue("warp_control_id", _warpControlID);
}

AtUtils::JsonValueVariant WarpPointsBase::PushValueToUI(uint32_t excludeClientID)
{
    std::string strValue = _value.ToString(false);
    UiElementValueUpdate update_ui(this, strValue, true, excludeClientID);
    return strValue;
}

void WarpPointsBase::UpdateFromSettings(bool restoreFromDefaults)
{
    if (_spSetting->IsValid())
    {
        std::string settingString = _spSetting->Get(restoreFromDefaults);
        if (!settingString.empty())
        {
            _value = WarpPointsValue::FromString(true, std::move(settingString));

            if (_callback)
            {
                // Push the value from the setting to the callback
                uint32_t i = 0;
                uint32_t numKnots = (uint32_t)_value._points.size();
                for (const auto& [x, y] : _value._points)
                {
                    _callback(UiUpdate::invalidClient, numKnots, i, x, y);
                    ++i;
                }
            }
        }
    }
}

void WarpPointsBase::SetSetting(std::shared_ptr<SettingValue<std::string>> spSetting)
{
    _spSetting = std::move(spSetting);
    UpdateFromSettings(false);
}

WarpPointsValue::WarpPointsValue(uint32_t nRows, uint32_t nColumns)
:    _nRows(nRows)
,    _nColumns(nColumns)
,    _points(nRows * nColumns)
{
}

WarpPointsValue::WarpPointsValue()
{
}

// Set csvFormat for command separated values, otherwise it's JSON
// If firstIndex < 0, then convert whole array of points, otherwise
// convert single item at index firstIndex
std::string WarpPointsValue::ToString(bool csvFormat, int32_t firstIndex)
{
    size_t nPoints = (firstIndex < 0) ? _points.size() : 1;
    if (firstIndex < 0)
        firstIndex = 0;

    if (csvFormat)
    {
        std::string stringValue;
        for (size_t i = 0; i < nPoints; i++)
        {
            if (i > 0)
                stringValue += ",";
            std::string indexedPointStr =
                AtUtils::FormatString("%zu,%f,%f", i + firstIndex, _points[i + firstIndex].first, _points[i + firstIndex].second);
            stringValue += indexedPointStr;
        }
        return stringValue;
    }
    else
    {
        auto spJson = AtUtils::IJson::Create();
        if (!spJson)
            return {};

        auto spObject = spJson->RootObject();
        if (!spObject)
            return {};

        spObject->AddValue("rows", (int32_t)_nRows);
        spObject->AddValue("columns", (int32_t)_nColumns);
        auto spArray = spObject->AddArray("points");

        for (size_t i = 0; i < nPoints; i++)
        {
            auto pointElement = spArray->AddElement();
            if (pointElement)
            {
                auto pointObject = pointElement->AddObject();
                if (pointObject)
                {
                    pointObject->AddValue("i", (int32_t)i + firstIndex);
                    pointObject->AddValue("x", (double)_points[i + firstIndex].first);
                    pointObject->AddValue("y", (double)_points[i + firstIndex].second);
                }
            }
        }

        return spJson->ToString();
    }
}

// Set csvFormat for command separated values, otherwise it's JSON
WarpPointsValue WarpPointsValue::FromString(bool csvFormat, std::string stringValue)
{
    if (csvFormat)
    {
        WarpPointsValue value;

        while (!stringValue.empty())
        {
            std::string indexString = UiElement::CsvExtract(stringValue);
            std::string xString = UiElement::CsvExtract(stringValue);
            std::string yString = UiElement::CsvExtract(stringValue);
            if (indexString.empty() || xString.empty() || yString.empty())
                break;

            float x = AtUtils::FromString<float>(xString);
            float y = AtUtils::FromString<float>(yString);
            value._points.push_back({ x, y });
        }

        // Assume square
        value._nColumns = (uint32_t)(std::sqrt((double)value._points.size()) + 0.01);
        value._nRows = value._nColumns;
        return value;
    }
    else
    {
        auto spJson = AtUtils::IJson::Create(stringValue);
        if (!spJson)
            return {};

        auto spObject = spJson->Parse();
        if (!spObject)
            return {};

        uint32_t nRows = (uint32_t)spObject->GetValue<int32_t>("rows");
        uint32_t nColumns = (uint32_t)spObject->GetValue<int32_t>("columns");

        auto spArray = spObject->GetArray("points");
        size_t nPoints = spArray->Size();

        if ((nRows * nColumns) == static_cast<uint32_t>(nPoints))
        {
            WarpPointsValue value(nRows, nColumns);

            for (size_t i = 0; i < nPoints; i++)
            {
                auto spElement = spArray->At(i);
                auto spObject = spElement->GetObject();
                if (spObject)
                {
                    double x = spObject->GetValue<double>("x");
                    double y = spObject->GetValue<double>("y");
                    value._points[i] = {(float)x, (float)y};
                }
            }

            return value;
        }
    }

    return WarpPointsValue{0, 0};
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemWarpCorners::UiControlItemWarpCorners(const std::string& label,
                                                   const WarpPointsValue& value,
                                                   WarpPointsCB callback,
                                                   uint32_t warpControlID):
    WarpPointsBase("UiControlItemWarpCorners", label, value, std::move(callback), warpControlID)
{
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemWarpArbitrary::UiControlItemWarpArbitrary(const std::string& label,
                                                       const WarpPointsValue& value,
                                                       WarpPointsCB callback,
                                                       uint32_t warpControlID):
    WarpPointsBase("UiControlItemWarpArbitrary", label, value, std::move(callback), warpControlID)
{
}

///////////////////////////////////////////////////////////////////////////////

UiControlItemEnumRadio::UiControlItemEnumRadio(const std::vector<UiEnumOption>& enumOptions,
                                               EnumControlCB callback,
                                               std::shared_ptr<ISettingValue<uint32_t>> spSetting,
                                               uint32_t controlUserData)
:   UiControlItemEnum("", enumOptions, std::move(callback), std::move(spSetting), controlUserData)
{
    _elementType = "UiControlItemEnumRadio";
}

///////////////////////////////////////////////////////////////////////////////

UiCustomControl::UiCustomControl(const std::string& controlIdName,
                                 CustomFillXmlCB customFillXmlCB)
:    UiControlItem(controlIdName.c_str())
,	_customFillXmlCB(std::move(customFillXmlCB))
{
}

void UiCustomControl::FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);
    if (_customFillXmlCB)
        _customFillXmlCB(spJsonObject);
}

