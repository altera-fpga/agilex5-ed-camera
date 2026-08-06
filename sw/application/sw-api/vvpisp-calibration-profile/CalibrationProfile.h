/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once


#define MIN_VERSION_SUPPORTED   "0.4"
#define MAX_VERSION_SUPPORTED   "0.6"


#include <map>
#include <cstdint>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>

#include "IspCommon.h"
#include "WbsUtils.h"


enum class ProfileErrors
{
    OK,
    // Operating Errors
    AnalogueGainOutOfRecordedRange,
    SceneTempOutOfRecordedRange,
    CoresNotResponding,
    // Profile load errors
    ProfileIOLoadError,
    ProfileVersInvalid,
    ProfileIncomplete,

    ProfileErrUnknown, // Aka - "dunno"
};

const char* ToString(const ProfileErrors& error);

struct AutoWhiteBalanceGainTableEntry
{
    uint32_t _BlcPedestals[4] = {0, 0, 0, 0};
    uint32_t _BlcScalars[4] = {0, 0, 0, 0};

    float _DarkNoise = 0;
    float _CombinedNoise = 0;

    // The following aren't stored in the json, and are calculated on load

    // *crickets*

    // Operator overload

    AutoWhiteBalanceGainTableEntry operator*(const float& scalar)
    {
        AutoWhiteBalanceGainTableEntry newEntry;

        for (int i = 0; i < 4; i++)
        {
            newEntry._BlcPedestals[i] = static_cast<uint32_t>(static_cast<float>(_BlcPedestals[i]) * scalar + 0.5f);
            newEntry._BlcScalars[i] = static_cast<uint32_t>(static_cast<float>(_BlcScalars[i]) * scalar + 0.5f);
        }

        newEntry._DarkNoise = _DarkNoise * scalar;
        newEntry._CombinedNoise = _CombinedNoise * scalar;

        return newEntry;
    }

    AutoWhiteBalanceGainTableEntry operator+(const AutoWhiteBalanceGainTableEntry& other)
    {
        AutoWhiteBalanceGainTableEntry newEntry;

        for (int i = 0; i < 4; i++)
        {
            newEntry._BlcPedestals[i] = _BlcPedestals[i] + other._BlcPedestals[i];
            newEntry._BlcScalars[i] = _BlcScalars[i] + other._BlcScalars[i];
        }

        newEntry._DarkNoise = _DarkNoise + other._DarkNoise;
        newEntry._CombinedNoise = _CombinedNoise + other._CombinedNoise;

        return newEntry;
    }

    AutoWhiteBalanceGainTableEntry operator-(const AutoWhiteBalanceGainTableEntry& other)
    {
        AutoWhiteBalanceGainTableEntry newEntry;

        for (int i = 0; i < 4; i++)
        {
            // Force these differences to 0 to prevent BLC singularities during extrapolation
            // This corresponds to extending the end value instead of linear extrapolation
            newEntry._BlcPedestals[i] = 0;
            newEntry._BlcScalars[i] = 0;
        }

        newEntry._DarkNoise = _DarkNoise - other._DarkNoise;
        newEntry._CombinedNoise = _CombinedNoise - other._CombinedNoise;

        return newEntry;
    }
};

std::ostream& operator<<(std::ostream& o, const AutoWhiteBalanceGainTableEntry& tableEntry);

struct AutoWhiteBalanceColourTempTableEntry
{
    SwApi::NormalisedWbsRegion _WbsRatio = {};
    //uint32_t _WbcCoeffs[4] = {2048, 2048, 2048, 2048};
    float _ChanScalars[3] = {1.0f, 1.0f, 1.0f};

    bool _HasCcm = false;
    float _CcmCoeffs[3][4] = {{1, 0, 0, 0},
                              {0, 1, 0, 0},
                              {0, 0, 1, 0}};

    // These do not get scaled, and are vestigial beyond being a reference point for future
    // format updates
    bool _HasRaw = false;
    intel_vvp_wbs_zone_result _Raw = {};
    uint16_t _RawDecBits = 0;
    SwApi::WbsResultFormat _RawFormat = SwApi::WbsResultFormat::RB_GG;

    float _Gain = 0.0f;
    // The following aren't stored in the json, and is calculated on profile load
    SwApi::NormalisedWbsRegion _GainScaledWbsRatio = {};

    // Operator overload

    AutoWhiteBalanceColourTempTableEntry operator*(const float& scalar)
    {
        AutoWhiteBalanceColourTempTableEntry newEntry;

        newEntry._WbsRatio.red_strength   = _WbsRatio.red_strength * scalar;
        newEntry._WbsRatio.green_strength = _WbsRatio.green_strength * scalar;
        newEntry._WbsRatio.blue_strength  = _WbsRatio.blue_strength * scalar;

        newEntry._ChanScalars[0] = _ChanScalars[0] * scalar;
        newEntry._ChanScalars[1] = _ChanScalars[1] * scalar;
        newEntry._ChanScalars[2] = _ChanScalars[2] * scalar;

        newEntry._HasRaw = false;

        newEntry._Gain = _Gain * scalar; // This only makes sense if we're interpolating!

        newEntry._HasCcm = _HasCcm;
        if (_HasCcm)
        {
            for (int level = 0; level < 3; level++)
            {
                for (int i = 0; i < 4; i++)
                {
                    newEntry._CcmCoeffs[level][i] = _CcmCoeffs[level][i] * scalar;
                }
            }
        }

        return newEntry;
    }

    AutoWhiteBalanceColourTempTableEntry operator+(const AutoWhiteBalanceColourTempTableEntry& other)
    {
        AutoWhiteBalanceColourTempTableEntry newEntry;

        newEntry._WbsRatio.red_strength   = _WbsRatio.red_strength + other._WbsRatio.red_strength;
        newEntry._WbsRatio.green_strength = _WbsRatio.green_strength + other._WbsRatio.green_strength;
        newEntry._WbsRatio.blue_strength  = _WbsRatio.blue_strength + other._WbsRatio.blue_strength;

        newEntry._ChanScalars[0]  = _ChanScalars[0] + other._ChanScalars[0];
        newEntry._ChanScalars[1]  = _ChanScalars[1] + other._ChanScalars[1];
        newEntry._ChanScalars[2]  = _ChanScalars[2] + other._ChanScalars[2];

        newEntry._HasRaw = false;

        newEntry._Gain = _Gain + other._Gain; // This only makes sense if we're interpolating!

        newEntry._HasCcm = _HasCcm && other._HasCcm;

        if (newEntry._HasCcm)
        {
            for (int level = 0; level < 3; level++)
            {
                for (int i = 0; i < 4; i++)
                {
                    newEntry._CcmCoeffs[level][i] = _CcmCoeffs[level][i] + other._CcmCoeffs[level][i];
                }
            }
        }

        return newEntry;
    }

    AutoWhiteBalanceColourTempTableEntry operator-(const AutoWhiteBalanceColourTempTableEntry& other)
    {
        AutoWhiteBalanceColourTempTableEntry newEntry;

        newEntry._WbsRatio.red_strength   = _WbsRatio.red_strength - other._WbsRatio.red_strength;
        newEntry._WbsRatio.green_strength = _WbsRatio.green_strength - other._WbsRatio.green_strength;
        newEntry._WbsRatio.blue_strength  = _WbsRatio.blue_strength - other._WbsRatio.blue_strength;

        newEntry._ChanScalars[0]  = _ChanScalars[0] - other._ChanScalars[0];
        newEntry._ChanScalars[1]  = _ChanScalars[1] - other._ChanScalars[1];
        newEntry._ChanScalars[2]  = _ChanScalars[2] - other._ChanScalars[2];

        newEntry._HasRaw = false;

        newEntry._Gain = _Gain - other._Gain; // This only makes sense if we're interpolating!

        newEntry._HasCcm = _HasCcm && other._HasCcm;

        if (newEntry._HasCcm)
        {
            for (int level = 0; level < 3; level++)
            {
                for (int i = 0; i < 4; i++)
                {
                    newEntry._CcmCoeffs[level][i] = _CcmCoeffs[level][i] - other._CcmCoeffs[level][i];
                }
            }
        }

        return newEntry;
    }
};

std::ostream& operator<<(std::ostream& o, const AutoWhiteBalanceColourTempTableEntry& tableEntry);

enum class EntryRepresentationStatus
{
    Direct = 0,
    Interpolate = 1,
    Extrapolate = 2,
    Invalid = 3
};

enum class EntryInterpolationMode
{
    Linear = 0,
    Exponential = 1
};

class SensorCalibrationProfile
{
    public:
        SensorCalibrationProfile();
        ProfileErrors LoadFromFile(const std::string& fileName,
                                   const std::filesystem::path& fileDirectory);

        void SaveToFile(const std::string& fileName);

        bool IsValid() { return (_perResolutionBlackLevelTable[0]->size() > 0) && (_colourTempTable.size() > 0); };
        
        void SetSensor(std::string sensor) { _sensor = sensor; };
        void SetCfaPhase(TCfaPhase cfaPhase) { _cfaPhase = cfaPhase; };
        void SetPip(uint8_t pip) { _pip = pip; };
        void SetBps(uint8_t bps) { _bps = bps; };

        std::string GetFilename() { return _fileName; };

        std::string GetVersion() { return _version; };

        std::string GetSensor() { return _sensor; };
        TCfaPhase GetCfaPhase() { return _cfaPhase; };
        uint8_t GetPip() { return _pip; };
        uint8_t GetBps() { return _bps; };
        std::string GetSelectedResolution() { return _selectedResolution; };
        bool GetChangeOnResolution() { return _changeOnResolution; };
        bool HasVCMesh() { return _hasVC; };
        uint8_t GetHorizVCMeshPoints() { return _vcHorizMeshPoints; };
        uint8_t GetVertiVCMeshPoints() { return _vcVertiMeshPoints; };
        uint8_t GetVCNumColorPlanes() { return _vcNumColorPlanes; };
        std::vector<uint32_t> GetVCCpMesh(uint8_t cp);
        uint16_t GetVCStepMeshHorizRes() { return _vcStepMeshResX; };
        uint16_t GetVCStepMeshVertiRes() { return _vcStepMeshResY; };
        std::vector<uint32_t> GetVCStepMesh() { return _vcStepMesh; };

        void LoadVCConfigIntoProfile(uint8_t horizMeshSize, uint8_t vertiMeshSize,
                                     uint16_t curResX, uint16_t curResY, uint8_t numColorPlanes,
                                     std::vector<uint32_t> *cp0Mesh,
                                     std::vector<uint32_t> *cp1Mesh,
                                     std::vector<uint32_t> *cp2Mesh,
                                     std::vector<uint32_t> *cp3Mesh,
                                     std::vector<uint32_t> stepMesh);

        std::vector<float> ResampleVCMeshRegardingStepMesh(std::vector<float> intensityMesh);

        void ApplyVCMeshes(uint16_t currentResX, uint16_t currentResY);

        int GetProfileIndexForResolution(std::string resolution);
        std::map<int, std::string> GetListOfResolutions() { return _availableResolutions; };
        bool SelectResolution(std::string resolution);

        EntryRepresentationStatus IsGainRepresentableInTable(float gain);
        std::vector<float> GetResListOfGains(std::string resolution);
        std::vector<float> GetListOfGains();
        AutoWhiteBalanceGainTableEntry GetInterpolatedTableEntryForGain(
            float gain,
            EntryInterpolationMode interpolationMode = EntryInterpolationMode::Linear);
        void DeleteEntryByGain(float gain);

        std::shared_ptr<AutoWhiteBalanceGainTableEntry> FindOrCreateEntryByGain(float gain);
        bool UpdateBlcParametersForGain(float gain, uint32_t blcPedestals[4], uint32_t blcScalars[4]);
        bool UpdateAnrParametersForGain(float gain, float darkNoise, float combinedNoise);

        EntryRepresentationStatus IsColourTempRepresentableInTable(uint16_t colourTemp);
        std::vector<uint16_t> GetListOfTemps();
        AutoWhiteBalanceColourTempTableEntry GetInterpolatedTableEntryForTemperature(
            uint16_t colourTemp,
            EntryInterpolationMode interpolationMode = EntryInterpolationMode::Linear);
        void DeleteEntryByTemp(uint16_t temp);
        std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> FindOrCreateEntryByColourTemp(uint16_t temp);
        bool UpdateCcmCoeffsForTemp(uint16_t temp, float ccmCoeffs[3][4]);

        void ChangeBaselineColourTemp(uint16_t newTemp);
        uint16_t GetBaselineColourTemp();
        EntryRepresentationStatus IsGainScaleTempRepresentableInTable(float gain);
        std::vector<float> GetListOfGainScaleReadings();
        AutoWhiteBalanceColourTempTableEntry GetInterpolatedTableEntryForGainScale(
            float gain,
            EntryInterpolationMode interpolationMode = EntryInterpolationMode::Linear);
        void DeleteGainScaleEntryByGain(float gain);
        std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> FindOrCreateGainScaleTempEntry(float gain);

    private:

        bool _valid;
        std::string _fileName;
        std::string _version;
        std::string _sensor;

        TCfaPhase _cfaPhase;

        uint8_t _bps = 10;
        uint8_t _pip = 2;

        // BLS/ANR Gains
        bool _changeOnResolution = false;
        std::string _selectedResolution = "default";
        uint16_t _selectedResolutionIndex = 0;
        std::map<int, std::string> _availableResolutions = {{0, "default"}};
        // A vector to a shared pointer of a map that points to shared pointers of the gain table entry.
        // Phew!
        std::vector<std::shared_ptr<std::map<float, std::shared_ptr<AutoWhiteBalanceGainTableEntry>>>> _perResolutionBlackLevelTable;
        // Main colour temperature curve
        std::map<uint16_t, std::shared_ptr<AutoWhiteBalanceColourTempTableEntry>> _colourTempTable;

        // Gain scaled baseline colour temperature data
        uint16_t _baselineColourTemp = 6500;
        std::map<float, std::shared_ptr<AutoWhiteBalanceColourTempTableEntry>> _gainScaleBaselineTable;

        // Vignette correction
        uint8_t _vcHorizMeshPoints;
        uint8_t _vcVertiMeshPoints;
        uint8_t _vcNumColorPlanes;
        uint16_t _vcStepMeshResX;
        uint16_t _vcStepMeshResY;
        bool _hasVC = false;
        std::vector<uint32_t> _vcCpMesh[4];
        std::vector<uint32_t> _vcStepMesh;
};