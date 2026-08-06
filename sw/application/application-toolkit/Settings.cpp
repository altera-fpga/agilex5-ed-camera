/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Settings.h"
#include "CommonUiUpdate.h"
#include "AtUtils.h"
#include "CommonApplicationBase.h"
#include <iostream>
#ifdef __linux__
#include <unistd.h>
#endif

SettingsSection::SettingsSection(Settings* pSettingsRoot, AtUtils::IJsonObjectPtr spJsonObject)
:    _pSettingsRoot(pSettingsRoot)
,    _spJsonObject(std::move(spJsonObject))
{
}

std::shared_ptr<ISettingsSection> SettingsSection::GetSection(std::string sectionName)
{
    if (sectionName.empty())
    {
        AtUtils::IJsonObjectPtr spChildObject = _spJsonObject->AddObject("dummy");
        return std::make_shared<SettingsSection>(_pSettingsRoot, spChildObject);
    }

    AtUtils::IJsonObjectPtr spChildObject = _spJsonObject->GetObject(sectionName.c_str());
    if (!spChildObject)
    {
        spChildObject = _spJsonObject->AddObject(sectionName.c_str());
        SetDirty();
    }

    auto spSettingsSection = std::make_shared<SettingsSection>(_pSettingsRoot, spChildObject);
    return spSettingsSection;
}

void SettingsSection::SetDirty()
{
    if (_pSettingsRoot)
        _pSettingsRoot->SetDirty();
}

bool SettingsSection::GetValue(std::string valueName, bool defaultValue)
{
    auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
    if (!spValueObject)
    {
        _spJsonObject->AddValue(valueName.c_str(), defaultValue);
        SetDirty();
        return defaultValue;
    }
    else
    {
        return spValueObject->GetValue<bool>();
    }
}

float SettingsSection::GetValue(std::string valueName, float defaultValue)
{
    auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
    if (!spValueObject)
    {
        _spJsonObject->AddValue(valueName.c_str(), static_cast<double>(defaultValue));
        SetDirty();
        return defaultValue;
    }
    else
    {
        return static_cast<float>(spValueObject->GetValue<double>());
    }
}

uint32_t SettingsSection::GetValue(std::string valueName, uint32_t defaultValue)
{
    auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
    if (!spValueObject)
    {
        _spJsonObject->AddValue(valueName.c_str(), defaultValue);
        SetDirty();
        return defaultValue;
    }
    else
    {
        AtUtils::JsonValueVariant valueVariant = spValueObject->GetValue();
        if (std::holds_alternative<int32_t>(valueVariant))
            return static_cast<uint32_t>(spValueObject->GetValue<int32_t>());
        else if (std::holds_alternative<uint32_t>(valueVariant))
            return spValueObject->GetValue<uint32_t>();
        else
            return 0;
    }
}

int32_t SettingsSection::GetValue(std::string valueName, int32_t defaultValue)
{
    auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
    if (!spValueObject)
    {
        _spJsonObject->AddValue(valueName.c_str(), defaultValue);
        SetDirty();
        return defaultValue;
    }
    else
    {
        AtUtils::JsonValueVariant valueVariant = spValueObject->GetValue();
        if (std::holds_alternative<int32_t>(valueVariant))
            return static_cast<uint32_t>(spValueObject->GetValue<int32_t>());
        else if (std::holds_alternative<uint32_t>(valueVariant))
            return spValueObject->GetValue<uint32_t>();
        else
            return 0;
    }
}

std::string SettingsSection::GetValue(std::string valueName, std::string defaultValue)
{
    auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
    if (!spValueObject)
    {
        _spJsonObject->AddValue(valueName.c_str(), defaultValue);
        SetDirty();
        return defaultValue;
    }
    else
    {
        return spValueObject->GetValue<std::string>();
    }
}

void SettingsSection::SetValue(std::string valueName, bool value)
{
    if (!LoadSettingsInProgress())
    {
        auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
        if (!spValueObject)
            _spJsonObject->AddValue(valueName.c_str(), value);
        else
            spValueObject->AddValue(value);

        SetDirty();
    }
}

void SettingsSection::SetValue(std::string valueName, float value)
{
    if (!LoadSettingsInProgress())
    {
        auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
        if (!spValueObject)
            _spJsonObject->AddValue(valueName.c_str(), (double)value);
        else
            spValueObject->AddValue((double)value);

        SetDirty();
    }
}

void SettingsSection::SetValue(std::string valueName, uint32_t value)
{
    if (!LoadSettingsInProgress())
    {
        auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
        if (!spValueObject)
            _spJsonObject->AddValue(valueName.c_str(), value);
        else
            spValueObject->AddValue(value);

        SetDirty();
    }
}

void SettingsSection::SetValue(std::string valueName, int32_t value)
{
    if (!LoadSettingsInProgress())
    {
        auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
        if (!spValueObject)
            _spJsonObject->AddValue(valueName.c_str(), value);
        else
            spValueObject->AddValue(value);

        SetDirty();
    }
}

void SettingsSection::SetValue(std::string valueName, std::string value)
{
    if (!LoadSettingsInProgress())
    {
        auto spValueObject = _spJsonObject->GetValue(valueName.c_str());
        if (!spValueObject)
            _spJsonObject->AddValue(valueName.c_str(), value);
        else
            spValueObject->AddValue(value);

        SetDirty();
    }
}

bool SettingsSection::LoadSettingsInProgress()
{
    return _pSettingsRoot ? _pSettingsRoot->LoadSettingsInProgress() : false;
}

///////////////////////////////////////////////////////////////////////////////

std::string Settings::_settingsFileName = "settings.json";

Settings::Settings(std::filesystem::path settingsFilename)
:    _settingsFilename(settingsFilename)
{
    _spJsonDOM = AtUtils::IJson::Create(std::move(settingsFilename));
    auto spRootObject = _spJsonDOM->Parse();
    if (!spRootObject)
    {
        _spJsonDOM = AtUtils::IJson::Create();
        spRootObject = _spJsonDOM->RootObject();
    }

    if (spRootObject)
    {
        auto spValue = spRootObject->GetValue("name");
        std::string nameValue;

        if (spValue)
        {
            nameValue = spValue->GetValue<std::string>();
        }

        if (nameValue != "Settings")
        {
            spRootObject->AddValue("name", "Settings");
            SetDirty();
        }

        _rootSettingsSection = std::make_shared<SettingsSection>(this, spRootObject);
    }
}

std::shared_ptr<ISettingsSection> Settings::GetSection(std::string sectionName)
{
    return _rootSettingsSection->GetSection(std::move(sectionName));
}

bool Settings::Save(std::filesystem::path settingsFilename)
{
    std::lock_guard lock(_dirty_cs);
    _settingsFilename = settingsFilename;
    return Save();
}

bool Settings::Save(bool always)
{
    std::lock_guard lock(_dirty_cs);
    if (!always and _pause_save)
    {
        // The pause_save stops saving file while SetDirty
        // is constantly called
        _pause_save = false;
        return true;
    }

    _dirty = false;
    bool result = false;

    if (!_settingsFilename.empty())
        result = _spJsonDOM->Save(_settingsFilename);

    if (result)
    {
#ifdef __linux__
        ::sync();
#endif
    }

    return result;
}

bool Settings::IsDirty()
{
    std::lock_guard lock(_dirty_cs);
    return _dirty;
}

void Settings::SetDirty()
{
    std::lock_guard lock(_dirty_cs);
    _dirty = true;
    _pause_save = true;
}

void Settings::Export(uint32_t clientId)
{
    std::lock_guard lock(_dirty_cs);

    // Force a save if dirty
    _pause_save = false;
    if (IsDirty())
        Save();

    std::filesystem::path settingsFilePath = std::filesystem::current_path();
    settingsFilePath /= _settingsFileName;
    ExportFileUiUpdate exportFile(clientId, std::move(settingsFilePath));
}

void Settings::StartImport(uint32_t clientId)
{
    std::filesystem::path settingsFilename = std::filesystem::current_path();
    settingsFilename /= _settingsFileName;

    auto importCompleteAction = [this](std::filesystem::path importPath,
                                       std::filesystem::path importDestinationPath)
    {
        // Load the new settings preset file
        auto spNewSettings = FinishImport(importPath, importDestinationPath);
        CommonApplicationBase::Get()->LoadNewSettings(std::move(spNewSettings));
    };
    ImportFileUiUpdate importFile(clientId, std::move(settingsFilename), "Application Settings", importCompleteAction);
}

std::shared_ptr<ISettings> Settings::FinishImport(std::filesystem::path& importFilename,
                                                  std::filesystem::path& importDestinationPath)
{
    std::lock_guard lock(_dirty_cs);

    if (importFilename != importDestinationPath)
    {
        std::filesystem::path backupFilename = importDestinationPath;
        backupFilename.replace_extension("bak");
        std::error_code ec;
        std::filesystem::remove(backupFilename, ec);
        std::filesystem::rename(importDestinationPath, backupFilename, ec);
        std::filesystem::copy(importFilename, importDestinationPath, std::filesystem::copy_options::overwrite_existing, ec);
        std::filesystem::remove(importFilename, ec);
#ifdef __linux__
        ::sync();
#endif
    }

    auto spNewSettings = std::make_shared<Settings>(importDestinationPath.c_str());

    // Avoid any writes from the old settings before it is replaced
    _settingsFilename = "";

    return spNewSettings;
}

std::shared_ptr<ISettings> Settings::RestoreFromBackup()
{
    std::lock_guard lock(_dirty_cs);
    std::error_code ec;
    if (std::filesystem::exists("settings.bak"))
    {
        std::filesystem::remove(_settingsFileName, ec);
        std::filesystem::copy("settings.bak", _settingsFileName, ec);
    }
    auto spNewSettings = std::make_shared<Settings>(_settingsFilename);
    return spNewSettings;
}

std::shared_ptr<ISettings> Settings::RestoreFactorySettings()
{
    std::lock_guard lock(_dirty_cs);
    std::error_code ec;
    std::filesystem::remove("settings.bak", ec);
    std::filesystem::rename(_settingsFileName, "settings.bak", ec);
    auto spNewSettings = std::make_shared<Settings>(_settingsFileName);
    return spNewSettings;
}

void Settings::SetLoading(bool state)
{
    std::lock_guard lock(_dirty_cs);
    _loadInProgress = state;
}

bool Settings::LoadSettingsInProgress()
{
    std::lock_guard lock(_dirty_cs);
    return _loadInProgress;
}

///////////////////////////////////////////////////////////////////////////////

BaseSettingValue::BaseSettingValue()
{
}

BaseSettingValue::BaseSettingValue(const char* sectionName, const char* name)
:    _sectionName(sectionName)
,    _name(name)
{
}

std::shared_ptr<ISettingsSection> BaseSettingValue::GetSection()
{
    auto spSettingSection = CommonApplicationBase::Get()->GetSettings()->GetSection(_sectionName);
    return spSettingSection;
}

bool BaseSettingValue::IsValid()
{
	return (!_sectionName.empty() && !_name.empty());
}

std::string BaseSettingValue::GetName()
{
    return _name;
}
