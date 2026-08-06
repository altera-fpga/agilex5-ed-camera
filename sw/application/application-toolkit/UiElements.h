/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "CommonUiUpdate.h"
#include "Settings.h"
#include <functional>
#include "PictureInPictureHandler.h"
#include "ColourPickerHandler.h"
#include <memory>
#include "AtUtils.h"

#undef GetObject

enum class ANCHOR_TYPE : uint32_t
{
    PARENT_NEAR, // left or top
    PARENT_FAR, // right or bottom
    SIBLING_NEAR, // left or top
    SIBLING_FAR, // right or bottoM
    FIT_CHILDREN,
    FIXED_SIZE
};

using HashAnchorType = std::hash<ANCHOR_TYPE>;

class UiEnumOption;

class UiElement
{
public:
    UiElement(const char* elementType);
    virtual ~UiElement();

    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    uint32_t GetID() { return _unique_id; }
    virtual void LoadSettings(bool restoreFromDefaults) {}
    virtual AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) { return ""; }
    void SetLogStream(std::ostream* pLogStream);  // For testing - uses std::cout by default
    static std::string CsvExtract(std::string& sourceString);

protected:
    virtual void FillJson(AtUtils::IJsonObjectPtr& parent_node);
    void LogDataTampering(const char* functionName, uint32_t clientID);
    std::string     _elementType;
    uint32_t        _unique_id;
    bool            _existsInClient = false;
    std::ostream*   _pLogStream;
    static uint32_t    _next_unique_id;
    static std::unordered_map<ANCHOR_TYPE, std::string, HashAnchorType> _attachment_type_string;
    static std::function<void(const char* functionName, uint32_t clientID)> _logDataTampering;
};

using BooleanControlCB = std::function<void(uint32_t clientId, bool& value)>;
using IntegerControlCB = std::function<void(uint32_t clientId, int32_t& value)>;
using UIntegerControlCB = std::function<void(uint32_t clientId, uint32_t& value)>;
using FloatControlCB = std::function<void(uint32_t clientId, float& value)>;
using SliderControlCB = std::function<void(uint32_t clientId, int32_t&)>;
using EnumControlCB = std::function<void(uint32_t clientId, const UiEnumOption& selectedOption, uint32_t controlUserData)>;
using TextControlCB = std::function<void(uint32_t clientId, std::string& value)>;
using ButtonControlCB = std::function<void(uint32_t clientId)>;
using WarpPointsCB = std::function<void(uint32_t clientId, uint32_t numKnots, uint32_t pointId, float x, float y)>;
using WarpModeCB = std::function<void(uint32_t clientId, uint32_t& value)>;
using PiPCanvasDataCB = std::function<void(uint32_t clientId, uint32_t canvasId, uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)>;
using CapturePictureCB = std::function<void()>;
using TResolution = std::pair<uint32_t, uint32_t>;

//////////////////
using LoggerDialogCB = std::function<void()>;

class UiControlSeparator;
class UiControlItemBoolean;
class UiControlItemSlider;
class UiControlItemEnum;
class UiControlItemEnumRadio;
class UiControlItemInteger;
class UiControlItemUInteger;
class UiControlItemFloat;
class UiControlItemText;
class UiControlItemButton;
class UiControlItemLabel;
class UiControlItemWarpCorners;
class UiControlItemWarpArbitrary;
class UiControlItemPicture;
class PictureCaptureHandler;

class WarpPointsValue
{
public:
    WarpPointsValue(uint32_t nRows, uint32_t nColumns);
    WarpPointsValue();
    uint32_t _nRows = 0;
    uint32_t _nColumns = 0;
    std::vector<std::pair<float, float>> _points;
    std::string ToString(bool csvFormat, int32_t firstIndex = -1);
    static WarpPointsValue FromString(bool csvFormat, std::string stringValue);
};

class ControlContainerHeaderButton
{
public:
    ControlContainerHeaderButton(const std::string& name,
                                 ButtonControlCB callback,
                                 const std::string& tooltip,
                                 const std::string& img,
                                 const std::string& clientCallback = "",
                                 uint32_t clientCallbackControlID = 0)
    :   _name(name)
    ,   _callback(std::move(callback))
    ,   _tooltip(tooltip)
    ,   _img(img)
    ,   _clientCallback(clientCallback)
    ,   _clientCallbackControlID(clientCallbackControlID)
    {
    }

    std::string _name;
    ButtonControlCB _callback;
    std::string _tooltip;
    std::string _img;
    std::string _clientCallback;
    uint32_t _clientCallbackControlID;

    static void Callback(const ButtonControlCB& callback, ParamListPtr& spParameterList)
    {
        if (callback)
            callback(spParameterList->GetClientID());
    }

    void Callback(ParamListPtr& spParameterList) const
    {
        Callback(_callback, spParameterList);
    }

    ButtonControlCB GetCallback() const
    {
        return _callback;
    }
};

class AttachmentDetails
{
public:
    AttachmentDetails() {}

    AttachmentDetails(ANCHOR_TYPE type, int32_t sizeOrOffset)
    :   _type(type)
    ,   _sizeOrOffset(sizeOrOffset)
    {
    }

    AttachmentDetails(ANCHOR_TYPE type, uint32_t siblingID, int32_t sizeOrOffset)
    :   _type(type)
    ,   _sizeOrOffset(sizeOrOffset)
    ,   _siblingID(siblingID)
    {
    }

    AttachmentDetails(ANCHOR_TYPE type, float dynamicPosition, int32_t sizeOrOffset)
    :   _type(type)
    ,   _sizeOrOffset(sizeOrOffset)
    ,   _dynamicPosition(dynamicPosition)
    {
    }

    ANCHOR_TYPE _type = ANCHOR_TYPE::PARENT_NEAR;
    int32_t     _sizeOrOffset = 0;
    float       _dynamicPosition = 9999.0f;
    uint32_t    _siblingID = 0;
};

enum eUILedMode
{
    kUiLedMode_None = 0,
    kUILedMode_Green = 1,
    kUILedMode_Red = 2,
    kUILedMode_Blue = 3,
};

class UiControlContainer : public UiElement
{
public:
    UiControlContainer(const std::string& containerTitle, const std::string& settingSectionName);

    void SetLeftAnchor(AttachmentDetails attachmentDetails);
    void SetTopAnchor(AttachmentDetails attachmentDetails);
    void SetRightAnchor(AttachmentDetails attachmentDetails);
    void SetBottomAnchor(AttachmentDetails attachmentDetails);

    std::shared_ptr<UiElement> Add(UiElement* control_element);
    std::shared_ptr<UiElement> Add(std::shared_ptr<UiElement> spControlElement,
                                   const std::string& settingName = "",
                                   const std::string& defaultValue = "");
    size_t GetNumControls() { return _controls.size(); }
    size_t GetNumControlsForHeight();
    std::shared_ptr<UiControlSeparator> AddSeparator();
    virtual bool IsColumnBreak() { return false; }

    std::shared_ptr<UiControlItemBoolean> AddLedControl(const std::string& label,
                                                         bool value,
                                                         BooleanControlCB callback,
                                                         eUILedMode color=kUILedMode_Green);
    std::shared_ptr<UiControlItemBoolean> AddLedControl(const std::string& label,
                                                         BooleanControlCB callback,
                                                         const char* settingName,
                                                         bool defaultValue,
                                                         eUILedMode color=kUILedMode_Green);

    std::shared_ptr<UiControlItemBoolean> AddBoolControl(const std::string& label,
                                                         bool value,
                                                         BooleanControlCB callback);
    std::shared_ptr<UiControlItemBoolean> AddBoolControl(const std::string& label,
                                                         BooleanControlCB callback,
                                                         const char* settingName,
                                                         bool defaultValue);

    std::shared_ptr<UiControlItemSlider> AddSliderControl(const std::string& label,
                                                          int32_t value,
                                                          int32_t minimumValue,
                                                          int32_t maximumValue,
                                                          SliderControlCB callback);
    std::shared_ptr<UiControlItemSlider> AddSliderControl(const std::string& label,
                                                          int32_t minimumValue,
                                                          int32_t maximumValue,
                                                          SliderControlCB callback,
                                                          const char* settingName,
                                                          int32_t defaultValue);
    std::shared_ptr<UiControlItemSlider> AddSliderControl(const std::string& label,
                                                          float value,
                                                          float minimumValue,
                                                          float maximumValue,
                                                          FloatControlCB callback,
                                                          uint32_t showDecimalPlaces = 2);
    std::shared_ptr<UiControlItemSlider> AddSliderControl(const std::string& label,
                                                          float minimumValue,
                                                          float maximumValue,
                                                          FloatControlCB callback,
                                                          const char* settingName,
                                                          float defaultValue,
                                                          uint32_t showDecimalPlaces = 2);

    std::shared_ptr<UiControlItemEnum> AddEnumControl(const std::string& label,
                                                      uint32_t selectedItemValue,
                                                      const std::vector<UiEnumOption>& options,
                                                      EnumControlCB callback,
                                                      uint32_t controlUserData = 0);
    std::shared_ptr<UiControlItemEnum> AddEnumControl(const std::string& label,
                                                      const std::vector<UiEnumOption>& options,
                                                      EnumControlCB callback,
                                                      const char* settingName,
                                                      uint32_t defaultValue,
                                                      uint32_t controlUserData = 0);
    std::shared_ptr<UiControlItemEnumRadio> AddEnumRadioControl(const std::vector<UiEnumOption>& options,
                                                                EnumControlCB callback,
                                                                const char* settingName,
                                                                uint32_t defaultValue,
                                                                uint32_t controlUserData = 0);

    std::shared_ptr<UiControlItemInteger> AddIntegerControl(const std::string& label,
                                                            int32_t value,
                                                            int32_t minimumValue,
                                                            int32_t maximumValue,
                                                            IntegerControlCB callback,
                                                            bool hex = false,
                                                            uint32_t increment = 1);
    std::shared_ptr<UiControlItemInteger> AddIntegerControl(const std::string& label,
                                                            int32_t minimumValue,
                                                            int32_t maximumValue,
                                                            IntegerControlCB callback,
                                                            const char* settingName,
                                                            int32_t defaultValue,
                                                            bool hex = false,
                                                            uint32_t increment = 1);
    std::shared_ptr<UiControlItemInteger> AddIntegerControl(const std::string& label,
                                                            const std::string& valuePrefix,
                                                            const std::string& valuePostfix,
                                                            int32_t value,
                                                            int32_t minimumValue,
                                                            int32_t maximumValue,
                                                            IntegerControlCB callback,
                                                            bool hex = false,
                                                            uint32_t increment = 1);
    std::shared_ptr<UiControlItemInteger> AddIntegerControl(const std::string& label,
                                                            const std::string& valuePrefix,
                                                            const std::string& valuePostfix,
                                                            int32_t minimumValue,
                                                            int32_t maximumValue,
                                                            IntegerControlCB callback,
                                                            const char* settingName,
                                                            int32_t defaultValue,
                                                            bool hex = false,
                                                            uint32_t increment = 1);

    std::shared_ptr<UiControlItemUInteger> AddUIntegerControl(const std::string& label,
                                                              uint32_t value,
                                                              uint32_t minimumValue,
                                                              uint32_t maximumValue,
                                                              UIntegerControlCB callback,
                                                              bool hex = true,
                                                              uint32_t increment = 1);
    std::shared_ptr<UiControlItemUInteger> AddUIntegerControl(const std::string& label,
                                                              uint32_t minimumValue,
                                                              uint32_t maximumValue,
                                                              UIntegerControlCB callback,
                                                              const char* settingName,
                                                              uint32_t defaultValue,
                                                              bool hex = true,
                                                              uint32_t increment = 1);

    std::shared_ptr<UiControlItemFloat> AddFloatControl(const std::string& label,
                                                        float value,
                                                        float minimumValue,
                                                        float maximumValue,
                                                        FloatControlCB callback);
    std::shared_ptr<UiControlItemFloat> AddFloatControl(const std::string& label,
                                                        float minimumValue,
                                                        float maximumValue,
                                                        FloatControlCB callback,
                                                        const char* settingName,
                                                        float defaultValue);

    std::shared_ptr<UiControlItemText> AddTextControl(const std::string& label,
                                                      const std::string& value,
                                                      TextControlCB callback);
    std::shared_ptr<UiControlItemText> AddTextControl(const std::string& label,
                                                      const std::string& valuePrefix,
                                                      const std::string& valuePostfix,
                                                      const std::string& value,
                                                      TextControlCB callback);
    std::shared_ptr<UiControlItemText> AddTextControl(const std::string& value,
                                                      TextControlCB callback);
    std::shared_ptr<UiControlItemText> AddTextControl(const std::string& label,
                                                      TextControlCB callback,
                                                      const char* settingName,
                                                      const std::string& defaultValue);
    std::shared_ptr<UiControlItemText> AddTextControl(const std::string& label,
                                                      const std::string& valuePrefix,
                                                      const std::string& valuePostfix,
                                                      TextControlCB callback,
                                                      const char* settingName,
                                                      const std::string& defaultValue);

    std::shared_ptr<UiControlItemButton> AddButtonControl(const std::string& title,
                                                          const std::string& buttonLabel,
                                                          ButtonControlCB callback);
    std::shared_ptr<UiControlItemButton> AddButtonControl(const std::string& buttonLabel,
                                                          ButtonControlCB callback);

    std::shared_ptr<UiControlItemLabel> AddLabelControl(const std::string& label);
    std::shared_ptr<UiControlItemLabel> AddLabelControl(const std::string& label, const std::string& value);

    std::shared_ptr<UiControlItemButton> AddPictureCaptureControl(const std::string& label,
                                                                  ModalDialogUiUpdate::DialogType dialogType,
                                                                  CapturePictureCB capturePictureCB);
    std::shared_ptr<UiControlItemButton> AddPictureInPictureControl(const std::string& label,
                                                                    PiPLayerUpdateCB layerUpdateCB,
                                                                    std::shared_ptr<PiPDetails> spPiPDetails);
    std::shared_ptr<UiControlItemButton> AddColourPickerControl(const std::string& label, ColourPickerUpdateCB colourUpdateCB,
                                                                AppToolkit::RgbPixel colourVals, bool preview = true);
    std::shared_ptr<UiControlItemPicture> AddPictureControl();

    void AddHeaderButtons(const std::vector<ControlContainerHeaderButton>& header_buttons);
    std::shared_ptr<ISettingsSection> GetSettings();
    void LoadSettings(bool restoreDefaults) override;
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    bool IsManuallyPositioned() { return _manualPositioning; }
    void Show(bool state, bool updateNow = true);
    bool GetShown() { return _show; }
    void EnableBorder(bool state);
    void SetTransparent(bool state);
    void SetSpacing(uint32_t spacing) { _spacing = spacing; }
    void GetUiControlsFlat(AtUtils::IJsonArrayPtr& spJsonArray);
    std::shared_ptr<UiElement> GetControl(const std::string& controlLabel);
    std::shared_ptr<UiElement> GetButton(const std::string& buttonLabel);
    std::shared_ptr<UiElement> GetControl(uint32_t controlID);
    std::string GetPanelName() { return _containerTitle; }

    void SetEnabled(bool state);

protected:
    void FillJson(AtUtils::IJsonObjectPtr& parent_node) override;
    bool                       _manualPositioning = false;
    std::vector<std::shared_ptr<UiElement>> _controls;

private:
    std::string             _containerTitle;
    AttachmentDetails       _leftAttachment;
    AttachmentDetails       _rightAttachment;
    AttachmentDetails       _topAttachment;
    AttachmentDetails       _bottomAttachment;

    size_t                  _numSeparators = 0;
    size_t                  _numControls = 0;
    ButtonControlCB         _resetPanelCallback;
    std::string             _resetPanelCallbackName;
    std::vector<ControlContainerHeaderButton> _header_buttons;
    std::shared_ptr<PictureInPictureHandler> _pipHandler;
    std::shared_ptr<ColourPickerHandler> _colourPickHandler;
    std::shared_ptr<PictureCaptureHandler> _pictureCaptureHandler;
    std::string             _settingSectionName;
    bool                    _show = true;
    bool                    _enableBorder = true;
    bool                    _transparentPanel = false;
    uint32_t                _spacing = 4;
};

class ColumnBreak : public UiControlContainer
{
public:
    ColumnBreak()
    :   UiControlContainer("", "")
    {
    }
    bool IsColumnBreak() override { return true; }
};

class StyleSetting
{
public:
    std::string _setting;
    std::string _value;
};

class AdditionalAttribute
{
public:
    std::string _name;
    std::string _value;
};

class UiControlItem : public UiElement
{
public:
    UiControlItem* Enable(bool state);
    bool GetEnabled() { return _enabled; }
    UiControlItem* Show(bool state);
    bool GetShown() { return _shown; }
    UiControlItem* SetToolTip(const std::string& stringID) { _tooltip = stringID; return this; }
    UiControlItem* SetClientCallbackName(uint32_t ownerID, const std::string& clientCallbackName);
    void LoadSettings(bool restoreFromDefaults) override;
    UiControlItem* OverrideStyle(const std::vector<StyleSetting>& styleSettings) { _styleSettings = styleSettings; return this; }
    UiControlItem* AddAttributes(const std::vector<AdditionalAttribute>& additionalAttributes) { _additionalAttributes = additionalAttributes; return this; }
    UiControlItem* UpdateLabel(const std::string& newLabel);
    UiControlItem* SetValuePreAndPostfix(const std::string& prefix, const std::string& postfix);
    UiControlItem* SetControlX(uint32_t controlX) { _controlX = controlX; return this; }
    UiControlItem* ReadOnly(bool state = true);
    std::string GetLabel() { return _label; }

protected:
    UiControlItem(const char* elementType);
    UiControlItem(const char* elementType, const std::string& label, uint32_t nLines = 1);
    UiControlItem(const char* elementType, const std::string& label,
                  const std::string& value_prefix, const std::string& value_postfix,
                  uint32_t nLines = 1);

    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;
    void UpdateCB(ParamListPtr& spParemeterList);
    void RegisterServerCommand();
    virtual void ForwardUpdateCB(uint32_t clientId, const std::string& value) {}
    virtual void UpdateFromSettings(bool restoreFromDefaults) {}

    std::string _label;
    std::string _valuePrefix;
    std::string _valuePostfix;
    std::string _serverCallbackName;
    std::string _clientCallbackName;
    uint32_t _clientCallbackOwnerID = 0;
    uint32_t _nLines = 1;
    bool    _enabled = true;
    bool    _shown = true;
    bool    _readOnly = false;
    bool    _fullWidthValue = false;
    uint32_t _controlX = 0; // If not overridden, OJControlContainer sets the default

    std::string _tooltip;
    std::vector<StyleSetting> _styleSettings;
    std::vector<AdditionalAttribute> _additionalAttributes;
};

class UiControlItemLabel : public UiControlItem
{
public:
    UiControlItemLabel(const std::string& label,
                       const std::string& valueString,
                       uint32_t nLines = 1);
    AtUtils::JsonValueVariant UpdateValue(const std::string& value);

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    std::string _value;
};

class UiControlItemSlider : public UiControlItem
{
public:
    UiControlItemSlider(const std::string& label,
                        int32_t minimum,
                        int32_t maximum,
                        SliderControlCB callback,
                        std::shared_ptr<ISettingValue<int32_t>> spSetting);
    UiControlItemSlider(const std::string& label,
                        float minimum,
                        float maximum,
                        uint32_t showDecimalPlaces,
                        FloatControlCB callback,
                        std::shared_ptr<ISettingValue<float>> spSetting);
    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(int32_t value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant UpdateValue(float value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    void UpdateFromSettings(bool restoreFromDefaults) override;
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void SetValueToDisplayConverters(uint32_t ownerID,
                                     const std::string& valueToDisplayFn,
                                     const std::string& displayToValueFn);

    template<typename T>
    T GetValue()
    {
        if constexpr (std::is_integral_v<T>)
            return _value;
        else if (std::is_floating_point_v<T>)
            return _floatValue;
    }   

    template<typename T>
    void UpdateRange(T min, T max)
    {
        static_assert(std::is_arithmetic_v<T> == true);

        if constexpr (std::is_integral_v<T>)
        {
            _minimum = min;
            _maximum = max;
        }
        else if (std::is_floating_point_v<T>)
        {
            _floatMinimum = min;
            _floatMaximum = max;
        }

        UiUpdate::ControlItemUpdateRange(this, std::to_string(min), std::to_string(max));        
    }

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    int32_t            _value = 0;
    int32_t            _minimum = 0;
    int32_t            _maximum = 0;
    SliderControlCB    _callback;

    float           _floatValue = 0.0;
    float           _floatMinimum = 0.0;
    float           _floatMaximum = 0.0;
    uint32_t        _showDecimalPlaces = 0;
    FloatControlCB    _floatCallback;
    std::shared_ptr<ISettingValue<int32_t>>  _spSetting;
    std::shared_ptr<ISettingValue<float>>    _spFloatSetting;
    uint32_t _valueToDisplayOwnerID = 0;
    std::string _valueToDisplayFn;
    uint32_t _displayToValueOwnerID = 0;
    std::string _displayToValueFn;
};

class UiControlItemInteger : public UiControlItem
{
public:
    UiControlItemInteger(const std::string& label, const std::string& prefix, const std::string& postfix,
                         int32_t minimum, int32_t maximum,
                         IntegerControlCB callback,
                         std::shared_ptr<ISettingValue<int32_t>> spSetting,
                         bool hex = false,
                         uint32_t increment = 1);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(int32_t value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void UpdateFromSettings(bool restoreFromDefaults) override;

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    int32_t                _value;
    int32_t                _minimum;
    int32_t                _maximum;
    bool                   _hex;
    uint32_t               _increment;
    IntegerControlCB    _callback;
    std::shared_ptr<ISettingValue<int32_t>> _spSetting;
};

class UiControlItemUInteger : public UiControlItem
{
public:
    UiControlItemUInteger(const std::string& label, const std::string& prefix, const std::string& postfix,
                          uint32_t minimum, uint32_t maximum,
                          UIntegerControlCB callback,
                          std::shared_ptr<ISettingValue<uint32_t>> spSetting,
                          bool hex = true,
                          uint32_t increment = 1);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(uint32_t value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void UpdateFromSettings(bool restoreFromDefaults) override;
    uint32_t GetValue() { return _value; }

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    uint32_t            _value;
    uint32_t            _minimum;
    uint32_t            _maximum;
    uint32_t            _increment;
    UIntegerControlCB    _callback;
    std::shared_ptr<ISettingValue<uint32_t>> _spSetting;
    bool _hex;
};

class UiControlItemBoolean : public UiControlItem
{
public:
    UiControlItemBoolean(const std::string& label,
                         BooleanControlCB callback,
                         std::shared_ptr<ISettingValue<bool>> spSetting,
                         eUILedMode ledMode=kUiLedMode_None);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(bool value,
                            bool callCallback = false,
                            uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void UpdateFromSettings(bool restoreFromDefaults) override;

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    bool                _value;
    eUILedMode          _ledMode;
    BooleanControlCB    _callback;
    std::shared_ptr<ISettingValue<bool>> _spSetting;
};

class UiControlItemFloat : public UiControlItem
{
public:
    UiControlItemFloat(const std::string& label,
                       float minimum, float maximum,
                       FloatControlCB callback,
                       std::shared_ptr<ISettingValue<float>> spSetting);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(float value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void UpdateFromSettings(bool restoreFromDefaults) override;
    void ShowDecimalPlaces(uint32_t decimalPlaces) { _showDecimalPlaces = decimalPlaces; }

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    float           _value;
    float           _minimum;
    float           _maximum;
    uint32_t        _showDecimalPlaces = 3;
    FloatControlCB  _callback;
    std::shared_ptr<ISettingValue<float>> _spSetting;
};

class UiEnumOption
{
public:
    UiEnumOption(const std::string& label, uint32_t userItemData = (uint32_t)-1, const std::string& imgUrl = "")
        : _label{label}
        , _index{0}
        , _userItemData{userItemData}
        , _imgUrl{imgUrl}
    {
    }

    std::string _label;
    uint32_t _index;
    uint32_t _userItemData;
    std::string _imgUrl;
};

class UiControlItemEnum : public UiControlItem
{
public:
    UiControlItemEnum(const std::string& label,
                      const std::vector<UiEnumOption>& enumOptions,
                      EnumControlCB callback,
                      std::shared_ptr<ISettingValue<uint32_t>> spSetting,
                      uint32_t controlUserData = 0);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(uint32_t selectedUserItemData, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    uint32_t GetSelectedIndex();
    void RemoveEnumItem(int index, const std::string& value);
    void AddEnumItem(const std::string& value, uint32_t userItemData = (uint32_t)-1);
    size_t GetLength() { return _enumOptions.size(); }

    void UpdateFromSettings(bool restoreFromDefaults) override;
    UiEnumOption GetOption(uint32_t index) const;

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    void CommonInit(uint32_t selectedItemUserData);

    uint32_t _selectedIndex = 0;
    std::vector<UiEnumOption> _enumOptions;
    EnumControlCB _callback;
    std::shared_ptr<ISettingValue<uint32_t>> _spSetting;
    uint32_t _controlUserData;
};

class UiControlItemText : public UiControlItem
{
public:
    UiControlItemText(const std::string& label,
                      TextControlCB callback,
                      std::shared_ptr<ISettingValue<std::string>> spSetting);
    UiControlItemText(const std::string& label,
                      const std::string& valuePrefix,
                      const std::string& valuePostfix,
                      TextControlCB callback,
                      std::shared_ptr<ISettingValue<std::string>> spSetting);
    UiControlItemText(TextControlCB callback,
                      std::shared_ptr<ISettingValue<std::string>> spSetting);
    UiControlItemText(TextControlCB callback,
                      const std::string& valuePrefix,
                      const std::string& valuePostfix,
                      std::shared_ptr<ISettingValue<std::string>> spSetting);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(const std::string& value,
                            bool callCallback = false,
                            uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void UpdateFromSettings(bool restoreFromDefaults) override;

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    std::string     _value;
    TextControlCB   _callback;
    std::shared_ptr<ISettingValue<std::string>> _spSetting;
};

class UiControlItemButton : public UiControlItem
{
public:
    UiControlItemButton(const std::string& title,
                        const std::string& buttonLabel,
                        ButtonControlCB callback);

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    void RenameButton(const std::string& newName);
    std::string GetButtonLabel() { return _buttonLabel; }

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    std::string        _buttonLabel;
    ButtonControlCB    _callback;
};


class UiControlItemPicture : public UiControlItem
{
public:
    UiControlItemPicture();

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;
};

class UiControlSeparator : public UiControlItem
{
public:
    UiControlSeparator()
    :    UiControlItem("UiControlSeparator")
    {
    }
};

class WarpPointsBase : public UiControlItem
{
public:
    virtual ~WarpPointsBase();
    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override;
    AtUtils::JsonValueVariant UpdateValue(uint32_t pointId, float x, float y, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    AtUtils::JsonValueVariant UpdateValue(const WarpPointsValue& value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient);
    void UpdateFromSettings(bool restoreFromDefaults) override;
    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID = UiUpdate::invalidClient) override;
    void SetSetting(std::shared_ptr<SettingValue<std::string>> spSetting);

protected:
    WarpPointsBase(const char* elementType, const std::string& label,
                   const WarpPointsValue& value, WarpPointsCB callback,
                   uint32_t warpControlID);
    WarpPointsBase() = delete;
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;

private:
    WarpPointsValue _value;
    WarpPointsCB    _callback;
    uint32_t        _warpControlID;
    std::shared_ptr<SettingValue<std::string>> _spSetting;
};

class UiControlItemWarpCorners : public WarpPointsBase
{
public:
    UiControlItemWarpCorners(const std::string& label,
                             const WarpPointsValue& value,
                             WarpPointsCB callback,
                             uint32_t warpControlID);
};

class UiControlItemWarpArbitrary : public WarpPointsBase
{
public:
    UiControlItemWarpArbitrary(const std::string& label,
                               const WarpPointsValue& value,
                               WarpPointsCB callback,
                               uint32_t warpControlID);
};

class UiControlItemEnumRadio : public UiControlItemEnum
{
public:
    UiControlItemEnumRadio(const std::vector<UiEnumOption>& enumOptions, EnumControlCB callback,
                           std::shared_ptr<ISettingValue<uint32_t>> spSetting,
                           uint32_t controlUserData = 0);
};

template<typename T>
class UiControlItemHiddenValue : public UiControlItem
{
public:
    UiControlItemHiddenValue(const T& value,
                             uint32_t callbackObjectID,
                             const char* jsClientFunction)
    :   UiControlItem("UiControlItemHiddenValue")
    ,    _value(value)
    {
        _clientCallbackName = jsClientFunction;
        _clientCallbackOwnerID = callbackObjectID;
    }

    AtUtils::JsonValueVariant UpdateValue(const T& value, bool callCallback = false, uint32_t fromClientId = UiUpdate::invalidClient)
    {
        _value = value;

        // Reflect the validated value to all clients
        auto str_val = GetJsonStringValue();
        UiElementValueUpdate update_ui(this, str_val, true);
        return str_val;
    }

    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID) override
    {
        auto str_val = GetJsonStringValue();
        UiElementValueUpdate update_ui(this, str_val, true, excludeClientID);
        return str_val;
    }

    void UpdateFromSettings(bool restoreFromDefaults) override {}

protected:
    virtual std::string GetJsonStringValue() = 0;

    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override
    {
        UiControlItem::FillJson(spJsonObject);
    }

protected:
    T _value;
};

class UiControlItemHiddenResolution : public UiControlItemHiddenValue<TResolution>
{
public:
    using UiControlItemHiddenValue::UiControlItemHiddenValue;

protected:
    std::string GetJsonStringValue() override
    {
        auto spJson = AtUtils::IJson::Create();
        if (!spJson)
            return {};

        auto spObject = spJson->RootObject();
        if (!spObject)
            return {};

        spObject->AddValue("_x", (int32_t)_value.first);
        spObject->AddValue("_y", (int32_t)_value.second);
        return spJson->ToString();
    }

    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override
    {
        UiControlItemHiddenValue::FillJson(spJsonObject);
        spJsonObject->AddValue("json_value", GetJsonStringValue())->SetStringIsObject();
    }
};

class UiControlItemHiddenUInt32 : public UiControlItemHiddenValue<uint32_t>
{
public:
    using UiControlItemHiddenValue::UiControlItemHiddenValue;

protected:
    std::string GetJsonStringValue() override
    {
        auto spJson = AtUtils::IJson::Create();
        if (!spJson)
            return {};

        auto spObject = spJson->RootObject();
        if (!spObject)
            return {};

        spObject->AddValue("_value", (int32_t)_value);
        return spJson->ToString();
    }

    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override
    {
        UiControlItemHiddenValue::FillJson(spJsonObject);
        spJsonObject->AddValue("json_value", GetJsonStringValue())->SetStringIsObject();
    }
};

using CustomFillXmlCB = std::function<void(AtUtils::IJsonObjectPtr& spJsonObject)>;

class UiCustomControl : public UiControlItem
{
public:
    UiCustomControl(const std::string& controlIdName,
                    CustomFillXmlCB customFillXmlCB = nullptr);

protected:
    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject) override;
    CustomFillXmlCB _customFillXmlCB;
};

template <typename VALUE_TYPE>
class UiCustomControlWithValue : public UiCustomControl
{
public:
    using CustomControlWithValueCB = std::function<void(uint32_t clientId, VALUE_TYPE& value)>;

    UiCustomControlWithValue(const std::string& controlIdName,
                             const VALUE_TYPE& value,
                             CustomControlWithValueCB callback = nullptr,
                             CustomFillXmlCB customFillXmlCB = nullptr,
                             std::shared_ptr<ISettingValue<std::string>> spSetting = nullptr)
    :   UiCustomControl(controlIdName, std::move(customFillXmlCB))
    ,   _value(value)
    ,   _spSetting(std::move(spSetting))
    ,   _callback(std::move(callback))
    {
        // Push the value from the setting to the callback
        if (_spSetting && _spSetting->IsValid() && _callback)
        {
             _value = VALUE_TYPE::FromString(true, _spSetting->Get());
             _callback(UiUpdate::invalidClient, _value);
        }
    }

    void ForwardUpdateCB(uint32_t clientId, const std::string& value) override
    {
        VALUE_TYPE newValue = VALUE_TYPE::FromString(true, value);
        UpdateValue(newValue, true, clientId);
    }

    AtUtils::JsonValueVariant UpdateValue(const VALUE_TYPE& value, bool callCallback = false,
                                          uint32_t fromClientId = UiUpdate::invalidClient)
    {
        _value = value;

        if (callCallback && _callback)
            _callback(fromClientId, _value);

        if (_spSetting)
        {
            // Save CSV value to settings
            std::string stringValue = _value.ToString(true);
            _spSetting->Set(stringValue);
        }

        return PushValueToUI(fromClientId);
    }

    AtUtils::JsonValueVariant PushValueToUI(uint32_t excludeClientID)
    {
        // Convert to JSON
        std::string strValue = _value.ToString(false);

        bool stringIsObject = true;
        UiElementValueUpdate updateUI(this, strValue, stringIsObject, excludeClientID);
        return strValue;
    }

    void FillJson(AtUtils::IJsonObjectPtr& spJsonObject)
    {
        UiCustomControl::FillJson(spJsonObject);
        auto spValueObject = spJsonObject->AddObject("value");
        _value.GetJSON(spValueObject);
    }

    void UpdateFromSettings(bool restoreFromDefaults)
    {
        if (_spSetting && _spSetting->IsValid())
        {
            _value = VALUE_TYPE::FromString(true, _spSetting->Get(restoreFromDefaults));

            // Update the app with the value taken from settings
            if (_callback)
                _callback(UiUpdate::invalidClient, _value);

            PushValueToUI(UiUpdate::invalidClient);
        }
    }    

protected:
    VALUE_TYPE _value;
    std::shared_ptr<ISettingValue<std::string>> _spSetting;
    CustomControlWithValueCB _callback;
};


