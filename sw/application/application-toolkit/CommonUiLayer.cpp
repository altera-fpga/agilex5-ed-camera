/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#include "CommonUiLayer.h"
#include "CommonUiUpdate.h"
#include <ctime>
#include "UiElements.h"
#include "WebServer.h"
#include "CommonApplicationBase.h"
#include "UiConfig.h"

#ifdef WIN32
#include "WinSock2.h"
#endif

#ifdef __linux__
#include <sys/utsname.h>
#include <unistd.h>
#endif

static uint32_t _nextUniqueID = 1000;
uint32_t GetNextUniqueId()
{
    return _nextUniqueID++;
}

CommonUiLayer* CommonUiLayer::_pInstance = nullptr;

CommonUiLayer::CommonUiLayer(ConnectionCB newConnectionCB)
:   _newConnectionCB(std::move(newConnectionCB))
{
    assert(_pInstance == nullptr);
    _pInstance = this;
}

CommonUiLayer::~CommonUiLayer()
{
    _pInstance = nullptr;
}

bool CommonUiLayer::Startup(StartupPhase phase)
{
    if (phase == StartupPhase::ONE)
    {
        AddCommands();
        return true;
    }
    else if (phase == StartupPhase::TWO)
    {
        AddInfoLabels();
        CreatetUiElements();
        return true;
    }

    return false;
}

bool CommonUiLayer::Shutdown(ShutdownPhase phase)
{
    return true;
}

// Add our personal commands
void CommonUiLayer::AddCommands()
{
}

std::shared_ptr<std::string> CommonUiLayer::GetMainUserInterfaceXml(uint32_t clientId)
{
    JsonDOM jsonDOM("MainUserInterface");
    AtUtils::IJsonObjectPtr spUiJsonObject = jsonDOM.GetRoot();

    std::string productName = CommonApplicationBase::Get()->GetProductName();
    std::string softwaveVersion = CommonApplicationBase::Get()->GetSoftwareVersion();
    std::string fontFamily = "'GT Walsheim Regular'";

    std::string os_name;
#ifdef WIN32
    os_name = "Windows 10";
#endif

#ifdef __linux__
    utsname system_name_info;
    uname(&system_name_info);
    os_name = AtUtils::FormatString("%s %s", system_name_info.sysname, system_name_info.release);
#endif
    spUiJsonObject->AddValue("os_name", os_name);

    char hostName[0x100];
    gethostname(hostName, 0x100);
    spUiJsonObject->AddValue("product_name", productName);
    spUiJsonObject->AddValue("software_version", softwaveVersion);
    spUiJsonObject->AddValue("font_family", fontFamily);
    spUiJsonObject->AddValue("hostname", hostName);
    spUiJsonObject->AddValue("build_date", __DATE__);
    spUiJsonObject->AddValue("build_time", __TIME__);
    spUiJsonObject->AddValue("debugging", CommonApplicationBase::Get()->IsDebugging());
    spUiJsonObject->AddValue("window_width", CommonApplicationBase::Get()->GetWindowWidth());
    spUiJsonObject->AddValue("window_height", CommonApplicationBase::Get()->GetWindowHeight());

    AddUiSettings(spUiJsonObject);
    GetUiControls(spUiJsonObject);
    GetInfoLabels(spUiJsonObject);

    std::shared_ptr<std::string> spJsonString(jsonDOM.ToString());

    if (_newConnectionCB)
        _newConnectionCB(clientId);

    return spJsonString;
}

void CommonUiLayer::AddInfoLabel(const char* label)
{
    _infoLabels.push_back(std::make_shared<InfoLabel>(label));
}

void CommonUiLayer::AddInfoLabels()
{
    std::string productName = CommonApplicationBase::Get()->GetProductName();
    std::string productLabel = AtUtils::FormatString("<span style='color:black;'><b>%s</b></span>",
                                             productName.c_str());
    AddInfoLabel(productLabel.c_str());

    time_t now = time(0);
    tm* theLocalTime = localtime(&now);
    if (theLocalTime)
        AddInfoLabel(asctime(theLocalTime));
    AddInfoLabel("");

    std::vector<std::string> logMessages = CommonApplicationBase::Get()->GetLogMessages();
    for (auto& label : logMessages)
        AddInfoLabel(label.c_str());
}

void CommonUiLayer::AddUiSettings(AtUtils::IJsonObjectPtr& omnitek_ui)
{
    WebServer::Get()->AddUiSettings(omnitek_ui);
}

// Called from RefreshUiControlsCmd on the command queue
void CommonUiLayer::RefreshUiControls()
{
    CreatetUiElements();
    UiControlsUpdate uiControlsUpdate;
    uiControlsUpdate.Notify();
}


void CommonUiLayer::CreatetUiElements()
{
    UiConfigUtils uiConfig(CommonApplicationBase::Get()->GetUiConfig());

    int32_t panelWidth = uiConfig.Get("PanelWidth", 465);
    int32_t verticalSpacingValue = uiConfig.Get("PanelVerticalSpacing", 12);
    int32_t horizontalSpacingValue = uiConfig.Get("PanelHorizontalSpacing", 12);

    int32_t verticalSpacing = verticalSpacingValue;
    int32_t horizontalSpacing = horizontalSpacingValue;

    // Clear any existing panels
    _tabs.clear();

    ICommonApplicationBase* pAppBase = CommonApplicationBase::Get();

    std::vector<std::weak_ptr<TopLevelUiTab>> allTabs = pAppBase->GetTopLevelTabs();
    uint32_t tabIndex = 0;

    for (auto& wpTopLevelTab : allTabs)
    {
        std::shared_ptr<TopLevelUiTab> spTopLevelTab = wpTopLevelTab.lock();
        std::shared_ptr<UiTab> tab = std::make_shared<UiTab>(tabIndex++, spTopLevelTab->_title, spTopLevelTab->_tooltip);
        _tabs.push_back(tab);

        std::vector<std::shared_ptr<UiControlContainer>>& controlContainers = tab->_controlContainers;
        std::vector<std::weak_ptr<IpUiControls>> allControls = spTopLevelTab->GetAllControls();

        AttachmentDetails leftAttachment{ANCHOR_TYPE::PARENT_NEAR, 0};
        AttachmentDetails topAttachment{ANCHOR_TYPE::PARENT_NEAR, 0};

        uint32_t currentVerticalUnits = 0;
        uint32_t numInStack = 0;
        uint32_t numColumns = 1;
        uint32_t heightLimit = uiConfig.Get("PanelHeightLimit", 23);

        // Collect all the control containers from all of the IpUiControls
        for (auto& wpIpUiControls : allControls)
        {
            std::shared_ptr<IpUiControls> spIpUiControls = wpIpUiControls.lock();

            if (spIpUiControls)
            {
                std::vector<std::shared_ptr<UiControlContainer>> uiControlContainers = spIpUiControls->AddUiElements();
                spIpUiControls->SetControlsLoaded();

                // If the application supplied a callback to manually position
                // the controls, it's done in DoManualPosition
                spIpUiControls->DoManualPosition(uiControlContainers);

                // Count the columns first. If only one, reduce the height
                for (auto& spUiControlContainer : uiControlContainers)
                {
                    if (spUiControlContainer)
                        controlContainers.push_back(spUiControlContainer);
                }

                // Collect dialog controls (if any) which are not on screen at all
                // times but need to be loaded from settings
                std::vector<std::shared_ptr<UiControlContainer>> dialogControlContainers = spIpUiControls->GetDialogControls();
                for (auto& spDialogControlContainer : dialogControlContainers)
                {
                    if (spDialogControlContainer)
                        _dialogControlContainers.push_back(spDialogControlContainer);
                }
            }
        }

        // Count the columns
        for (auto& spUiControlContainer : controlContainers)
        {
            if (spUiControlContainer and !spUiControlContainer->IsManuallyPositioned())
            {
                // Take the header into account
                uint32_t verticalUnits = (uint32_t)spUiControlContainer->GetNumControlsForHeight() + 1;

                // heightLimit maximum in a stack
                bool startNewColumn = ((currentVerticalUnits + verticalUnits) > heightLimit) and (numInStack > 0);
                if (spUiControlContainer->IsColumnBreak())
                    startNewColumn = true;   

                if (startNewColumn)
                {
                    // Start another column
                    currentVerticalUnits = 0;
                    numInStack = 0;
                    numColumns++;

                    if (spUiControlContainer->IsColumnBreak())
                        continue;
                }

                currentVerticalUnits += verticalUnits;
                numInStack++;
            }
        }

        if (numColumns <= 2)
            heightLimit = 15;
        currentVerticalUnits = 0;
        numInStack = 0;

        for (auto& spUiControlContainer : controlContainers)
        {
            if (!spUiControlContainer or spUiControlContainer->IsManuallyPositioned())
                continue;

            // Take the header into account
            uint32_t verticalUnits = (uint32_t)spUiControlContainer->GetNumControlsForHeight() + 1;

            // heightLimit maximum in a stack
            bool startNewColumn = ((currentVerticalUnits + verticalUnits) > heightLimit) and (numInStack > 0);
            if (spUiControlContainer->IsColumnBreak())
                startNewColumn = true;   

            if (startNewColumn)
            {
                // Start another column
                currentVerticalUnits = 0;
                topAttachment._type = ANCHOR_TYPE::PARENT_NEAR;
                topAttachment._siblingID = 0;
                leftAttachment._type = ANCHOR_TYPE::SIBLING_FAR;
                horizontalSpacing = horizontalSpacingValue;
                numInStack = 0;

                if (spUiControlContainer->IsColumnBreak())
                    continue;
            }

            leftAttachment._sizeOrOffset = (leftAttachment._siblingID == 0) ? 0 : horizontalSpacing;
            topAttachment._sizeOrOffset = (topAttachment._siblingID == 0) ? 0 : verticalSpacing;
            spUiControlContainer->SetLeftAnchor(leftAttachment);
            spUiControlContainer->SetTopAnchor(topAttachment);
            spUiControlContainer->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
            spUiControlContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

            currentVerticalUnits += verticalUnits;
            numInStack++;

            leftAttachment._type = ANCHOR_TYPE::SIBLING_NEAR;
            topAttachment._type = ANCHOR_TYPE::SIBLING_FAR;

            verticalSpacing = verticalSpacingValue;
            horizontalSpacing = 0;

            leftAttachment._siblingID = spUiControlContainer->GetID();
            topAttachment._siblingID = spUiControlContainer->GetID();
        }

    } // Next tab

    if (!_spSettingsContainer)
    {
        _spSettingsContainer = std::make_shared<UiControlContainer>("Presets", "");

        _spSettingsContainer->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 12 });
        _spSettingsContainer->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 12 });
        _spSettingsContainer->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 350 });
        _spSettingsContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

        auto exportPresetCB = [this](uint32_t clientId)
        {
            CommonApplicationBase::Get()->GetSettings()->Export(clientId);

            ModalDialogUiUpdate closeDialogs(UiUpdate::allClients, ModalDialogUiUpdate::DialogType::CLOSE);
            closeDialogs.Notify();
        };
        _spSettingsContainer->AddButtonControl("Export", ">", exportPresetCB);

        auto importPresetCB = [this](uint32_t clientId)
        {
            CommonApplicationBase::Get()->GetSettings()->StartImport(clientId);

            ModalDialogUiUpdate closeDialogs(UiUpdate::allClients, ModalDialogUiUpdate::DialogType::CLOSE);
            closeDialogs.Notify();
        };
        _spSettingsContainer->AddButtonControl("Import", ">", importPresetCB);

        auto restorePresetCB = [this](uint32_t)
        {
            CommonApplicationBase::Get()->RestoreFactorySettings();

            ModalDialogUiUpdate closeDialogs(UiUpdate::allClients, ModalDialogUiUpdate::DialogType::CLOSE);
            closeDialogs.Notify();
        };
        _spSettingsContainer->AddButtonControl("Restore factory settings", ">", restorePresetCB);

        auto restoreBackupPresetCB = [this](uint32_t)
        {
            CommonApplicationBase::Get()->RestoreFromBackup();

            ModalDialogUiUpdate closeDialogs(UiUpdate::allClients, ModalDialogUiUpdate::DialogType::CLOSE);
            closeDialogs.Notify();
        };
        _spSettingsContainer->AddButtonControl("Reset from backup", ">", restoreBackupPresetCB);
    }
}

void CommonUiLayer::GetUiControls(AtUtils::IJsonObjectPtr& spUiJsonObject)
{
    auto spTabsArray = spUiJsonObject->AddArray("Tabs");
    for (auto& spTab : _tabs)
    {
        auto spTabObject = spTabsArray->AddElement()->AddObject();
        spTabObject->AddValue("index", spTab->_index);
        spTabObject->AddValue("title", spTab->_title);
        spTabObject->AddValue("tooltip", spTab->_tooltip);

        auto spControlsArray = spTabObject->AddArray("Controls");
        for (auto& spControlContainer : spTab->_controlContainers)
        {
            if (spControlContainer->IsColumnBreak())
                continue;
                
            auto spControlObject = spControlsArray->AddElement()->AddObject();
            spControlContainer->GetJSON(spControlObject);
        }        
    }

    auto spSettingsArray = spUiJsonObject->AddArray("Settings");
    {
        auto spJsonObject = spSettingsArray->AddElement()->AddObject();
        _spSettingsContainer->GetJSON(spJsonObject);
    }
}

void CommonUiLayer::GetUiControlsFlat(AtUtils::IJsonObjectPtr& spUiJsonObject)
{
    auto spControlsArray = spUiJsonObject->AddArray("Controls");
    for (auto& spTab : _tabs)
    {
        for (auto& spControlContainer : spTab->_controlContainers)
        {
            if (spControlContainer->IsColumnBreak())
                continue;

            spControlContainer->GetUiControlsFlat(spControlsArray);
        }
    }
}

std::shared_ptr<UiElement> CommonUiLayer::GetControl(const std::string& panelName, const std::string& controlLabel)
{
    for (auto& spTab : _tabs)
    {
        for (auto& spControlContainer : spTab->_controlContainers)
        {
            if (spControlContainer->IsColumnBreak())
                continue;

            if (spControlContainer->GetPanelName() == panelName)
                return spControlContainer->GetControl(controlLabel);
        }
    }

    return nullptr;
}

std::shared_ptr<UiElement> CommonUiLayer::GetButton(const std::string& panelName, const std::string& buttonLabel)
{
    for (auto& spTab : _tabs)
    {
        for (auto& spControlContainer : spTab->_controlContainers)
        {
            if (spControlContainer->IsColumnBreak())
                continue;

            if (spControlContainer->GetPanelName() == panelName)
                return spControlContainer->GetButton(buttonLabel);
        }
    }    

    return nullptr;
}

std::shared_ptr<UiElement> CommonUiLayer::GetControl(uint32_t controlID)
{
    for (auto& spTab : _tabs)
    {
        for (auto& spControlContainer : spTab->_controlContainers)
        {
            if (spControlContainer->IsColumnBreak())
                continue;

            std::shared_ptr<UiElement> spControl = spControlContainer->GetControl(controlID);
            if (spControl)
                return spControl;
        }
    }      

    return nullptr;
}

void CommonUiLayer::GetInfoLabels(AtUtils::IJsonObjectPtr& spUiJsonObject)
{
    auto spLabelsArray = spUiJsonObject->AddArray("InfoLabels");
    for (auto& spLabel : _infoLabels)
        spLabel->GetJSON(spLabelsArray);
}

// Update all controls with settings from new preset
void CommonUiLayer::LoadSettings(bool restoreDefaults)
{
    // Load settings for main screen controls
    for (auto& spTab : _tabs)
    {
        for (auto& spControlContainer : spTab->_controlContainers)
        {
            if (spControlContainer->IsColumnBreak())
                continue;

            spControlContainer->LoadSettings(restoreDefaults);
        }
    }    

    // Load settings for any dialog controls (see Warp, TMO)
    for (auto& spControlContainer : _dialogControlContainers)
        spControlContainer->LoadSettings(restoreDefaults);
}
