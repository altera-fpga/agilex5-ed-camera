/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <mutex>
#include "CommonTypes.h"
#include "AppToolkitIJson.h"

class Settings;

class ISettingsSection
{
public:
    virtual ~ISettingsSection() {};

    virtual std::shared_ptr<ISettingsSection> GetSection(std::string sectionName) = 0;
    virtual void SetDirty() = 0;
    virtual bool     GetValue(std::string valueName, bool defaultValue) = 0;
    virtual float    GetValue(std::string valueName, float defaultValue) = 0;
    virtual uint32_t GetValue(std::string valueName, uint32_t defaultValue) = 0;
    virtual int32_t  GetValue(std::string valueName, int32_t defaultValue) = 0;
    virtual std::string GetValue(std::string valueName, std::string defaultValue) = 0;
    virtual void SetValue(std::string valueName, bool value) = 0;
    virtual void SetValue(std::string valueName, float value) = 0;
    virtual void SetValue(std::string valueName, uint32_t value) = 0;
    virtual void SetValue(std::string valueName, int32_t value) = 0;
    virtual void SetValue(std::string valueName, std::string value) = 0;
    virtual bool LoadSettingsInProgress() = 0;
};

class SettingsSection : public ISettingsSection
{
public:
    SettingsSection(Settings* pSettingsRoot, AtUtils::IJsonObjectPtr spJsonObject);

    std::shared_ptr<ISettingsSection> GetSection(std::string sectionName) override;
    void SetDirty() override;

    bool     GetValue(std::string valueName, bool defaultValue) override;
    float    GetValue(std::string valueName, float defaultValue) override;
    uint32_t GetValue(std::string valueName, uint32_t defaultValue) override;
    int32_t  GetValue(std::string valueName, int32_t defaultValue) override;
    std::string GetValue(std::string valueName, std::string defaultValue) override;

    void SetValue(std::string valueName, bool value) override;
    void SetValue(std::string valueName, float value) override;
    void SetValue(std::string valueName, uint32_t value) override;
    void SetValue(std::string valueName, int32_t value) override;
    void SetValue(std::string valueName, std::string value) override;
    bool LoadSettingsInProgress() override;

private:
    Settings* _pSettingsRoot;
    AtUtils::IJsonObjectPtr _spJsonObject;
};

class ISettings
{
public:
    virtual ~ISettings() {};
    virtual bool Save(std::filesystem::path settingsFilename) = 0;
    virtual bool Save(bool always = false) = 0;
    virtual std::shared_ptr<ISettingsSection> GetSection(std::string sectionName) = 0;
    virtual bool IsDirty() = 0;
    virtual void SetDirty() = 0;
    virtual void Export(uint32_t clientId) = 0;
    virtual void StartImport(uint32_t clientId) = 0;
    virtual std::shared_ptr<ISettings> FinishImport(std::filesystem::path& importFilename,
                                                    std::filesystem::path& importDestinationPath) = 0;
    virtual std::shared_ptr<ISettings> RestoreFromBackup() = 0;
    virtual std::shared_ptr<ISettings> RestoreFactorySettings() = 0;
    virtual void SetLoading(bool state) = 0;
    virtual bool LoadSettingsInProgress() = 0;
};

class Settings : public ISettings
{
public:
    static std::string _settingsFileName;

    Settings(std::filesystem::path settingsFilename);
    bool Save(std::filesystem::path settingsFilename) override;
    bool Save(bool always = false) override;
    std::shared_ptr<ISettingsSection> GetSection(std::string sectionName) override;
    bool IsDirty() override;
    void SetDirty() override;
    void Export(uint32_t clientId) override;
    void StartImport(uint32_t clientId) override;
    std::shared_ptr<ISettings> FinishImport(std::filesystem::path& importFilename,
                                            std::filesystem::path& importDestinationPath) override;
    std::shared_ptr<ISettings> RestoreFromBackup() override;
    std::shared_ptr<ISettings> RestoreFactorySettings() override;
    void SetLoading(bool state) override;
    bool LoadSettingsInProgress() override;

private:
    std::filesystem::path _settingsFilename;
    AtUtils::IJsonPtr _spJsonDOM;
    std::shared_ptr<SettingsSection> _rootSettingsSection;
    std::recursive_mutex _dirty_cs;
    bool _dirty = false;
    bool _pause_save = false;
    bool _loadInProgress = false;
};

class BaseSettingValue
{
public:
    BaseSettingValue();
    BaseSettingValue(const char* sectionName, const char* name);
    std::shared_ptr<ISettingsSection> GetSection();
    bool IsValid();
    std::string GetName();
protected:
    std::string _sectionName;
    std::string _name;
};

template <class T>
class ISettingValue
{
public:
    virtual ~ISettingValue() {}

    virtual T Get(bool getDefaultValue = false) = 0;
    virtual void Set(const T& value) = 0;
    virtual T GetDefault() = 0;
    virtual std::shared_ptr<ISettingsSection> GetSection() = 0;
    virtual bool IsValid() = 0;
};

template <class T>
class SettingValue : public ISettingValue<T>
{
public:
    SettingValue(const char* sectionName, const char* name, const T& defaultValue)
    :    _baseSettingValue(sectionName, name)
    ,    _defaultValue(defaultValue)
    {
    }

    SettingValue(const T& defaultValue)
    :    _baseSettingValue()
    ,    _defaultValue(defaultValue)
    {
    }

    T Get(bool getDefaultValue = false) override
    {
        if (getDefaultValue || !IsValid())
            return _defaultValue;
        else
        {
            auto spSettingSection = GetSection();
            return spSettingSection->GetValue(_baseSettingValue.GetName(), _defaultValue);
        }
    }

    void Set(const T& value) override
    {
        if (IsValid())
        {
            auto spSettingSection = GetSection();
            spSettingSection->SetValue(_baseSettingValue.GetName(), value);
        }
    }

    T GetDefault() override
    {
        return _defaultValue;
    }

    virtual std::shared_ptr<ISettingsSection> GetSection() { return _baseSettingValue.GetSection(); }
    virtual bool IsValid() { return _baseSettingValue.IsValid(); }

private:
    BaseSettingValue _baseSettingValue;
    T _defaultValue;
};
