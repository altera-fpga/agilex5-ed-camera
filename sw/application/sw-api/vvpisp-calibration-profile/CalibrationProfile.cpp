/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CalibrationProfile.h"
#include <fstream>
#include <iostream>
#include <numeric>
#include <cmath>
#include <memory>


void to_json(AtUtils::IJsonObjectPtr j, const AutoWhiteBalanceGainTableEntry& p)
{
    auto blcPedestal = j->AddArray("blc_pedestal");
    blcPedestal->AddElement()->AddValue(p._BlcPedestals[0]);
    blcPedestal->AddElement()->AddValue(p._BlcPedestals[1]);
    blcPedestal->AddElement()->AddValue(p._BlcPedestals[2]);
    blcPedestal->AddElement()->AddValue(p._BlcPedestals[3]);

    auto blcScalar = j->AddArray("blc_scalar");
    blcScalar->AddElement()->AddValue(p._BlcScalars[0]);
    blcScalar->AddElement()->AddValue(p._BlcScalars[1]);
    blcScalar->AddElement()->AddValue(p._BlcScalars[2]);
    blcScalar->AddElement()->AddValue(p._BlcScalars[3]);

    j->AddValue("dark_noise", p._DarkNoise);
    j->AddValue("combined_noise", p._CombinedNoise);
};


void to_json(AtUtils::IJsonObjectPtr j, const AutoWhiteBalanceColourTempTableEntry& p)
{
    auto normalisedWbsRegion = j->AddArray("norm_wbs_ratio");
    normalisedWbsRegion->AddElement()->AddValue(p._WbsRatio.red_strength);
    normalisedWbsRegion->AddElement()->AddValue(p._WbsRatio.green_strength);
    normalisedWbsRegion->AddElement()->AddValue(p._WbsRatio.blue_strength);

    auto rgbScalars = j->AddArray("rgb_scalars");
    rgbScalars->AddElement()->AddValue(p._ChanScalars[0]);
    rgbScalars->AddElement()->AddValue(p._ChanScalars[1]);
    rgbScalars->AddElement()->AddValue(p._ChanScalars[2]);

    auto ccmCoeffArray = j->AddArray("ccm_coeffs");

    if (p._HasCcm)
    {
        for(std::size_t i = 0; i < 3; ++i)
        {
            const auto json_array = ccmCoeffArray->AddElement()->AddArray();
            json_array->AddElement()->AddValue(p._CcmCoeffs[i][0]);
            json_array->AddElement()->AddValue(p._CcmCoeffs[i][1]);
            json_array->AddElement()->AddValue(p._CcmCoeffs[i][2]);
            json_array->AddElement()->AddValue(p._CcmCoeffs[i][3]);
        }
    }
    else
    {
        for(std::size_t i = 0; i < 3; ++i)
        {
            const auto json_array = ccmCoeffArray->AddElement()->AddArray();
            json_array->AddElement()->AddValue(i == 0 ? 1.0f : 0.0f);
            json_array->AddElement()->AddValue(i == 1 ? 1.0f : 0.0f);
            json_array->AddElement()->AddValue(i == 2 ? 1.0f : 0.0f);
            json_array->AddElement()->AddValue(0.0f);
        }
    }

    if (p._HasRaw)
    {
        j->AddValue("has_raw", true);
        j->AddValue("frac_size", p._RawDecBits);
        // Note: check the actual size in bits required for x0/x1_integer
        j->AddValue("x0_int", static_cast<uint32_t>(p._Raw.x0_integer));
        j->AddValue("x0_frac", p._Raw.x0_fraction);
        j->AddValue("x1_int", static_cast<uint32_t>(p._Raw.x1_integer));
        j->AddValue("x1_frac", p._Raw.x1_fraction);
        j->AddValue("ratio_count", p._Raw.num_pixels_accumulated);
        j->AddValue("ratio_format", SwApi::ToString(p._RawFormat));
    }
    else
    {
        j->AddValue("has_raw", false);
    }

};


void to_json(AtUtils::IJsonObjectPtr j, SensorCalibrationProfile& p)
{
    //j->AddValue("version", std::to_string(MAX_VERSION_SUPPORTED)); // We always output the newest format
    j->AddValue("version", MAX_VERSION_SUPPORTED); // We always output the newest format
    j->AddValue("sensor", p.GetSensor());
    j->AddValue("bits_per_second", p.GetBps());  // FIXME - needed?
    j->AddValue("pip", p.GetPip());
    j->AddValue("baseline_temp", p.GetBaselineColourTemp());
    j->AddValue("cfa_phase", ToString(p.GetCfaPhase()));

    auto gain_levels = j->AddObject("gain_levels");
    gain_levels->AddValue("change_on_resolution", p.GetChangeOnResolution());

    auto listOfResolutions = p.GetListOfResolutions();
    auto resolutions = gain_levels->AddObject("resolutions");

    for (const auto& r: listOfResolutions)
        resolutions->AddValue(std::to_string(r.first).c_str(), r.second);

    auto res_gain_data = gain_levels->AddObject("res_gain_data");
    std::string currentRes = p.GetSelectedResolution();

    for (const auto& r: listOfResolutions)
    {
        p.SelectResolution(r.second);
        const auto listOfGains = p.GetListOfGains();

        auto rg = res_gain_data->AddObject(r.second.c_str());

        for(const auto& g: listOfGains)
        {
            // This is meant to be automatic, but this works for now
            auto resData = rg->AddObject(std::to_string(g).c_str());
            auto blackLevelEntry = resData->AddObject("black_level");

            to_json(std::move(blackLevelEntry), p.GetInterpolatedTableEntryForGain(g));
        }
    }

    p.SelectResolution(std::move(currentRes));

    auto temps = j->AddObject("colour_temps");

    for(const auto& t: p.GetListOfTemps())
    {
        auto entry = temps->AddObject(std::to_string(t).c_str());
        to_json(std::move(entry), p.GetInterpolatedTableEntryForTemperature(t));
    }


    if (p.HasVCMesh())
    {
        auto vignette_correction = j->AddObject("vignette_correction");

        uint8_t numColPlanes = p.GetVCNumColorPlanes();

        vignette_correction->AddValue("horiz_mesh_size", p.GetHorizVCMeshPoints());
        vignette_correction->AddValue("verti_mesh_size", p.GetVertiVCMeshPoints());
        vignette_correction->AddValue("num_col_planes", numColPlanes);
        vignette_correction->AddValue("step_mesh_x_res", p.GetVCStepMeshHorizRes());
        vignette_correction->AddValue("step_mesh_y_res", p.GetVCStepMeshVertiRes());

        auto add_vcc_mesh_json = [vignette_correction = std::move(vignette_correction)](const char* name, const std::vector<uint32_t>& v){
            auto json_array = vignette_correction->AddArray(name);
            for(const auto _v: v)
                json_array->AddElement()->AddValue(_v);
        };

        add_vcc_mesh_json("cp_mesh_0", p.GetVCCpMesh(0));

        if (numColPlanes > 1)
            add_vcc_mesh_json("cp_mesh_1", p.GetVCCpMesh(1));

        if (numColPlanes > 2)
            add_vcc_mesh_json("cp_mesh_2", p.GetVCCpMesh(2));

        if (numColPlanes > 3)
            add_vcc_mesh_json("cp_mesh_3", p.GetVCCpMesh(3));

        add_vcc_mesh_json("step_mesh", p.GetVCStepMesh());
    }
};


int64_t json_get_int_value(const AtUtils::IJsonValuePtr json_value, const int64_t default_value = 0){
    int64_t rv{default_value};

    if(json_value)
    {
        const AtUtils::JsonValueVariant jv = json_value->GetValue();
  
        if(std::holds_alternative<int32_t>(jv))
            rv = static_cast<int64_t>(std::get<int32_t>(jv));
        else if(std::holds_alternative<uint32_t>(jv))
            rv = static_cast<int64_t>(std::get<uint32_t>(jv));
        else
            std::cerr << "JSON value is not an integer\n"; 
    }

    return rv;
};


std::string json_get_string_value(const AtUtils::IJsonValuePtr json_value, const std::string& default_value = ""){
    std::string rv{default_value};

    if(json_value)
    {
        const AtUtils::JsonValueVariant jv = json_value->GetValue();        

        if(std::holds_alternative<const char*>(jv))
            rv = std::get<const char*>(jv);
        else if(std::holds_alternative<std::string>(jv))
            rv = std::get<std::string>(jv);
        else
            std::cerr << "JSON value is not a string\n";
    }

    return rv;
};


double json_get_fp_value(const AtUtils::IJsonValuePtr json_value, const double default_value = 0.0){
    double rv{default_value};

    if(json_value)
    {
        const AtUtils::JsonValueVariant jv = json_value->GetValue();
  
        // Note: this is a bit of a hack, but it works for now
        // We assume that the JSON parser will always return a double for floating point numbers
        // This is not guaranteed by the JSON standard, but it is a common practice
        if(std::holds_alternative<double>(jv))
            rv = std::get<double>(jv);
        else
            std::cerr << "JSON value is not a floating point number\n";
    }

    return rv;
};


bool json_get_bool_value(const AtUtils::IJsonValuePtr json_value, const bool default_value = false){
    bool rv{default_value};

    if(json_value)
    {
        const AtUtils::JsonValueVariant jv = json_value->GetValue();

        if(std::holds_alternative<bool>(jv))
            rv = std::get<bool>(jv);
        else
            std::cerr << "JSON value is not a boolean\n";
    }

    return rv;
};


std::size_t ver_str_to_num(const std::string& verstr) {
    return std::accumulate(verstr.begin(), verstr.end(), 0, 
        [](std::size_t acc, char c) {
            return std::isdigit(c) ? (acc * 10) + (c - '0') : acc;
        });
}


SensorCalibrationProfile::SensorCalibrationProfile():
    _cfaPhase{TCfaPhase::RGGB},
    _vcHorizMeshPoints{0},
    _vcVertiMeshPoints{0},
    _vcNumColorPlanes{0},
    _vcStepMeshResX{0},
    _vcStepMeshResY{0}
{
    _valid = false;
    _version = MAX_VERSION_SUPPORTED; // Current version
    _sensor = "N/A";

    _changeOnResolution = false;
    _selectedResolution = "default";
    std::shared_ptr<std::map<float, std::shared_ptr<AutoWhiteBalanceGainTableEntry>>> defaultResTableEntry = std::make_shared<std::map<float, std::shared_ptr<AutoWhiteBalanceGainTableEntry>>>();
    _perResolutionBlackLevelTable.emplace_back(defaultResTableEntry);
}

ProfileErrors SensorCalibrationProfile::LoadFromFile(
    const std::string& fileName,
    const std::filesystem::path& fileDirectory)
{
    _fileName = fileName;

    _valid = false;

    std::ifstream fileStream( fileDirectory );

    if(!fileStream.good())
    {
        return ProfileErrors::ProfileIOLoadError;
    }

    auto json = AtUtils::IJson::Create(fileStream);
    auto data = json->Parse();

    if(!data)
    {
        std::cout << "JSON ERROR - could not parse file" << std::endl;
        return ProfileErrors::ProfileIOLoadError;
    }

    _version = json_get_string_value(data->GetValue("version"));

    if(_version.empty())
    {
        std::cout << "JSON ERROR - no version in json" << std::endl;
        return ProfileErrors::ProfileVersInvalid;
    }

    std::cout << "Version: " << _version << "\n";

    const auto min_ver_supported = ver_str_to_num(MIN_VERSION_SUPPORTED);
    const auto max_ver_supported = ver_str_to_num(MAX_VERSION_SUPPORTED);
    const auto current_version = ver_str_to_num(_version);

    if (current_version < min_ver_supported || current_version > max_ver_supported) 
    {
        std::cout << "JSON ERROR - unsupported version" << std::endl;
        return ProfileErrors::ProfileVersInvalid;
    }

    const auto cfaPhaseStr = json_get_string_value(data->GetValue("cfa_phase"));

    if (cfaPhaseStr == "RGGB")
    {
        _cfaPhase = TCfaPhase::RGGB;
    }
    else if (cfaPhaseStr == "GRBG")
    {
        _cfaPhase = TCfaPhase::GRBG;
    }
    else if (cfaPhaseStr == "GBRG")
    {
        _cfaPhase = TCfaPhase::GBRG;
    }
    else if (cfaPhaseStr == "BGGR")
    {
        _cfaPhase = TCfaPhase::BGGR;
    }
    else
    {
        std::cout << "JSON Warning - no CFA phase value in json - defaulting to RGGB" << std::endl;
        _cfaPhase = TCfaPhase::RGGB;
    }
    std::cout << "CFA Phase: " << _cfaPhase << "\n";


    if(!data->GetValue("gain_levels"))
    {
        std::cout << "JSON ERROR - no gain values in json" << std::endl;
        return ProfileErrors::ProfileIncomplete;
    }

    _baselineColourTemp = json_get_int_value(data->GetValue("baseline_temp"), 6500);

    const auto gain_data_value = data->GetValue("gain_levels");
    const auto gain_data = gain_data_value ? gain_data_value->GetObject() : nullptr;

    if(!gain_data)
    {
        std::cout << "JSON ERROR - no gain levels in json" << std::endl;
        return ProfileErrors::ProfileIncomplete;
    }

    {
        _changeOnResolution = json_get_bool_value(gain_data->GetValue("change_on_resolution"), false);

        if (_changeOnResolution)
        {
            _availableResolutions.clear();

            const auto json_res = gain_data->GetValue("resolutions");

            if(json_res)
            {
                const auto json_obj = json_res->GetObject();

                if(json_obj)
                {
                    for(std::size_t i = 0; i < json_obj->GetNumMembers(); ++i)
                    {
                        const auto jr = json_obj->GetMember(i);
                        const int res_key = std::stoi(jr->GetName());

                        const auto res_val = json_get_string_value(jr->GetValue());
                        _availableResolutions.insert(std::make_pair(res_key, res_val));

                        if (res_key == 0)
                        {
                            _selectedResolution = std::move(res_val);
                            _selectedResolutionIndex = res_key;
                        }
                    }
                }
            }
        }

        {
            _perResolutionBlackLevelTable.clear();

            const auto res_gain_data = gain_data->GetValue("res_gain_data");

            if(res_gain_data)
            {
                const auto json_obj = res_gain_data->GetObject();

                if(!json_obj)
                {
                    std::cerr << "JSON ERROR - no resolution gain data in json" << std::endl;
                    return ProfileErrors::ProfileIncomplete;
                }

                for(const auto& ar: _availableResolutions)
                {
                    const auto& res_str = ar.second;
                    const auto jv = json_obj->GetValue(res_str.c_str());
                    const auto resdata_json_obj =  jv ? jv->GetObject() : nullptr;

                    if(resdata_json_obj)
                    {
                        auto resGainTableEntry = std::make_shared<std::map<float, std::shared_ptr<AutoWhiteBalanceGainTableEntry>>>();

                        for(std::size_t i = 0; i < resdata_json_obj->GetNumMembers(); ++i)
                        {
                            const auto json_member = resdata_json_obj->GetMember(i);
                            
                            if(!json_member)
                            {
                                std::cerr << "JSON ERROR - no member at index " << i << " in resolution data for " << res_str << std::endl;
                                continue;
                            }

                            const auto& gain_str = json_member->GetName();
                            const float gain = std::stof(gain_str);

                            AtUtils::IJsonArrayPtr blc_pedestal_json_arr{nullptr};
                            AtUtils::IJsonArrayPtr blc_scalar_json_arr{nullptr};

                            const auto gain_json_obj = json_member->GetValue() ? json_member->GetValue()->GetObject() : nullptr;
                            const auto jv = gain_json_obj ? gain_json_obj->GetValue("black_level") : nullptr;
                            const auto bl_json_obj = jv ? jv->GetObject() : nullptr;

                            if(bl_json_obj)
                            {
                                {
                                    const auto jv = bl_json_obj->GetValue("blc_pedestal");

                                    if(jv)
                                        blc_pedestal_json_arr = jv->GetArray();
                                }

                                {
                                    const auto jv = bl_json_obj->GetValue("blc_scalar");

                                    if(jv)
                                        blc_scalar_json_arr = jv->GetArray();
                                }
                            }

                            std::shared_ptr<AutoWhiteBalanceGainTableEntry> newEntry = std::make_shared<AutoWhiteBalanceGainTableEntry>();

                            for(int j = 0; j < 4; j++)
                            {
                                newEntry->_BlcPedestals[j] = blc_pedestal_json_arr ? json_get_int_value(blc_pedestal_json_arr->At(j)) : 0;
                                newEntry->_BlcScalars[j] = blc_scalar_json_arr ? json_get_int_value(blc_scalar_json_arr->At(j)) : 0;
                            }

                            if(bl_json_obj)
                            {
                                newEntry->_DarkNoise = json_get_fp_value(bl_json_obj->GetValue("dark_noise"));
                                newEntry->_CombinedNoise = json_get_fp_value(bl_json_obj->GetValue("combined_noise"));                            
                            }

                            resGainTableEntry->insert(std::pair(gain, newEntry));

                            /*
                            TODO - This is ignored, so I'm not bothering with it
                            auto colour_data = el.value().at("colour_data");
                            {
                                std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> newEntry = std::make_shared<AutoWhiteBalanceColourTempTableEntry>();

                                auto has_raw = colour_data["has_raw"];
                                if (has_raw)
                                {
                                    newEntry->_HasRaw = true;
                                    newEntry->_RawDecBits = colour_data["frac_size"];
                                    if (colour_data["ratio_format"] == "GG_RB")
                                    {
                                        newEntry->_RawFormat = SwApi::WbsResultFormat::GG_RB;
                                    }
                                    // TODO - Add other formats here

                                    newEntry->_Raw.x0_integer = (unsigned long)(colour_data["x0_ratio"]) >> newEntry->_RawDecBits;
                                    newEntry->_Raw.x0_fraction = (unsigned long)(colour_data["x0_ratio"]) & ((1 << newEntry->_RawDecBits) - 1);
                                    newEntry->_Raw.x1_integer = (unsigned long)(colour_data["x1_ratio"]) >> newEntry->_RawDecBits;
                                    newEntry->_Raw.x1_fraction = (unsigned long)(colour_data["x1_ratio"]) & ((1 << newEntry->_RawDecBits) - 1);
                                    newEntry->_Raw.num_pixels_accumulated = colour_data["ratio_count"];
                                }
                                else
                                {
                                    newEntry->_HasRaw = false;
                                }

                                newEntry->_WbsRatio.red_strength = colour_data["norm_wbs_ratio"][0];
                                newEntry->_WbsRatio.green_strength = colour_data["norm_wbs_ratio"][1];
                                newEntry->_WbsRatio.blue_strength = colour_data["norm_wbs_ratio"][2];


                                newEntry->_ChanScalars[0] = colour_data["rgb_scalars"][0];
                                newEntry->_ChanScalars[1] = colour_data["rgb_scalars"][1];
                                newEntry->_ChanScalars[2] = colour_data["rgb_scalars"][2];

                                _gainScaleBaselineTable.insert(std::pair(gain, newEntry));
                            }
                            */
                        }

                        _perResolutionBlackLevelTable.emplace_back(resGainTableEntry);
                    }
                }
            }
        }
    }
    /*
    for (auto& el : data.at("gain_levels").items())
    {
        float gain = std::stof(std::string(el.key()));
        auto black_level_data = el.value().at("black_level");
        {

            auto blc_pedestal = black_level_data["blc_pedestal"];
            auto blc_scalar = black_level_data["blc_scalar"];

            std::shared_ptr<AutoWhiteBalanceGainTableEntry> newEntry = std::make_shared<AutoWhiteBalanceGainTableEntry>();

            for (int i = 0; i < 4; i++)
            {
                newEntry->_BlcPedestals[i] = blc_pedestal[i];
                newEntry->_BlcScalars[i] = blc_scalar[i];
            }

            _perResolutionBlackLevelTable[_selectedResolutionIndex]->insert(std::pair(gain, newEntry));
        }
        auto colour_data = el.value().at("colour_data");
        {
            std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> newEntry = std::make_shared<AutoWhiteBalanceColourTempTableEntry>();

            auto has_raw = colour_data["has_raw"];
            if (has_raw)
            {
                newEntry->_HasRaw = true;
                newEntry->_RawDecBits = colour_data["frac_size"];
                if (colour_data["ratio_format"] == "GG_RB")
                {
                    newEntry->_RawFormat = SwApi::WbsResultFormat::GG_RB;
                }
                // TODO - Add other formats here

                newEntry->_Raw.x0_integer = (unsigned long)(colour_data["x0_ratio"]) >> newEntry->_RawDecBits;
                newEntry->_Raw.x0_fraction = (unsigned long)(colour_data["x0_ratio"]) & ((1 << newEntry->_RawDecBits) - 1);
                newEntry->_Raw.x1_integer = (unsigned long)(colour_data["x1_ratio"]) >> newEntry->_RawDecBits;
                newEntry->_Raw.x1_fraction = (unsigned long)(colour_data["x1_ratio"]) & ((1 << newEntry->_RawDecBits) - 1);
                newEntry->_Raw.num_pixels_accumulated = colour_data["ratio_count"];
            }
            else
            {
                newEntry->_HasRaw = false;
            }

            newEntry->_WbsRatio.red_strength = colour_data["norm_wbs_ratio"][0];
            newEntry->_WbsRatio.green_strength = colour_data["norm_wbs_ratio"][1];
            newEntry->_WbsRatio.blue_strength = colour_data["norm_wbs_ratio"][2];


            newEntry->_ChanScalars[0] = colour_data["rgb_scalars"][0];
            newEntry->_ChanScalars[1] = colour_data["rgb_scalars"][1];
            newEntry->_ChanScalars[2] = colour_data["rgb_scalars"][2];

            _gainScaleBaselineTable.insert(std::pair(gain, newEntry));
        }
    }
    */
    {
        _colourTempTable.clear();

        const auto colour_temps = data->GetValue("colour_temps");

        if(colour_temps)
        {
            auto colour_temps_json_obj = colour_temps->GetObject();

            if(colour_temps_json_obj)
            {
                for(std::size_t i = 0; i < colour_temps_json_obj->GetNumMembers(); ++i)
                {
                    const auto ct = colour_temps_json_obj->GetMember(i);

                    if(ct)
                    {
                        uint16_t temp = static_cast<uint16_t>(std::stoi(ct->GetName()));

                        std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> newEntry = std::make_shared<AutoWhiteBalanceColourTempTableEntry>();

                        const auto ct_json_obj = ct->GetValue() ? ct->GetValue()->GetObject() : nullptr;
                        const auto has_raw = ct_json_obj ? ct_json_obj->GetValue("has_raw") : nullptr;

                        newEntry->_HasRaw = false;

                        if(has_raw)
                        {
                            newEntry->_HasRaw = true;
                            newEntry->_RawDecBits = static_cast<uint16_t>(json_get_int_value(ct_json_obj->GetValue("frac_size")));

                            const std::string ratio_format_str = json_get_string_value(ct_json_obj->GetValue("ratio_format"));

                            if(ratio_format_str == "GG_RB")
                                newEntry->_RawFormat = SwApi::WbsResultFormat::GG_RB;

                            // TODO - Add other formats here

                            // Note this might require 64bit signed integer for underlying type in JSON parser
                            newEntry->_Raw.x0_integer = static_cast<uint64_t>(json_get_int_value(ct_json_obj->GetValue("x0_int")));
                            newEntry->_Raw.x0_fraction = static_cast<uint16_t>(json_get_int_value(ct_json_obj->GetValue("x0_frac")));

                            newEntry->_Raw.x1_integer = static_cast<uint64_t>(json_get_int_value(ct_json_obj->GetValue("x1_int")));
                            newEntry->_Raw.x1_fraction = static_cast<uint16_t>(json_get_int_value(ct_json_obj->GetValue("x1_frac")));

                            newEntry->_Raw.num_pixels_accumulated = static_cast<uint16_t>(json_get_int_value(ct_json_obj->GetValue("ratio_count")));
                        }

                        {
                            const auto jv = ct_json_obj ? ct_json_obj->GetValue("norm_wbs_ratio") : nullptr;
                            const auto norm_wbs_ratio_json_arr = jv ? jv->GetArray() : nullptr;

                            if(norm_wbs_ratio_json_arr)
                            {
                                newEntry->_WbsRatio.red_strength = json_get_fp_value(norm_wbs_ratio_json_arr->At(0));
                                newEntry->_WbsRatio.green_strength = json_get_fp_value(norm_wbs_ratio_json_arr->At(1));
                                newEntry->_WbsRatio.blue_strength = json_get_fp_value(norm_wbs_ratio_json_arr->At(2));
                            }
                        }

                        {
                            const auto jv = ct_json_obj->GetValue("rgb_scalars");
                            const auto rgb_scalars_json_arr = jv ? jv->GetArray() : nullptr;

                            if(rgb_scalars_json_arr)
                            {
                                newEntry->_ChanScalars[0] = json_get_fp_value(rgb_scalars_json_arr->At(0));
                                newEntry->_ChanScalars[1] = json_get_fp_value(rgb_scalars_json_arr->At(1));
                                newEntry->_ChanScalars[2] = json_get_fp_value(rgb_scalars_json_arr->At(2));
                            }
                        }

                        const auto ccm_coeffs = ct_json_obj->GetValue("ccm_coeffs");

                        if (ccm_coeffs)
                        {
                            const auto ccm_coeffs_json_arr = ccm_coeffs->GetArray();

                            if(ccm_coeffs_json_arr)
                            {
                                newEntry->_HasCcm = true;

                                for(std::size_t i = 0; i < 3; ++i)
                                {
                                    const auto json_arr = ccm_coeffs_json_arr->At(i)->GetArray();

                                    if(json_arr)
                                    {
                                        newEntry->_CcmCoeffs[i][0] = json_get_fp_value(json_arr->At(0));
                                        newEntry->_CcmCoeffs[i][1] = json_get_fp_value(json_arr->At(1));
                                        newEntry->_CcmCoeffs[i][2] = json_get_fp_value(json_arr->At(2));
                                        newEntry->_CcmCoeffs[i][3] = json_get_fp_value(json_arr->At(3));
                                    }
                                }
                            }
                        }

                        _colourTempTable.insert(std::pair(temp, newEntry));
                    }
                }
            }
        }

        const auto vignette_correction = data->GetValue("vignette_correction");

        if(vignette_correction)
        {
            const auto vc_json_obj = vignette_correction->GetObject();

            if(vc_json_obj)
            {
                _vcHorizMeshPoints = json_get_int_value(vc_json_obj->GetValue("horiz_mesh_size"));
                _vcVertiMeshPoints = json_get_int_value(vc_json_obj->GetValue("verti_mesh_size"));
                _vcNumColorPlanes = json_get_int_value(vc_json_obj->GetValue("num_col_planes"));

                _vcStepMeshResX = json_get_int_value(vc_json_obj->GetValue("step_mesh_x_res"));
                _vcStepMeshResY = json_get_int_value(vc_json_obj->GetValue("step_mesh_y_res"));

                _vcCpMesh[0].clear();
                _vcCpMesh[1].clear();
                _vcCpMesh[2].clear();
                _vcCpMesh[3].clear();
                _vcStepMesh.clear();

                const auto cp_mesh_arr = vc_json_obj->GetArray("cp_mesh_0");

                if(cp_mesh_arr)
                {
                    for(std::size_t i = 0; i < cp_mesh_arr->Size(); ++i)
                        _vcCpMesh[0].push_back(json_get_int_value(cp_mesh_arr->At(i)));
                }

                if (_vcNumColorPlanes > 1)
                {
                    const auto cp_mesh_arr = vc_json_obj->GetArray("cp_mesh_1");

                    if(cp_mesh_arr)
                    {
                        for(std::size_t i = 0; i < cp_mesh_arr->Size(); ++i)
                            _vcCpMesh[1].push_back(json_get_int_value(cp_mesh_arr->At(i)));
                    }
                }

                if (_vcNumColorPlanes > 2)
                {
                    const auto cp_mesh_arr = vc_json_obj->GetArray("cp_mesh_2");

                    if(cp_mesh_arr)
                    {
                        for(std::size_t i = 0; i < cp_mesh_arr->Size(); ++i)
                            _vcCpMesh[2].push_back(json_get_int_value(cp_mesh_arr->At(i)));
                    }
                }

                if (_vcNumColorPlanes > 3)
                {
                    const auto cp_mesh_arr = vc_json_obj->GetArray("cp_mesh_3");

                    if(cp_mesh_arr)
                    {
                        for(std::size_t i = 0; i < cp_mesh_arr->Size(); ++i)
                            _vcCpMesh[3].push_back(json_get_int_value(cp_mesh_arr->At(i)));
                    }
                }

                {
                    const auto cp_mesh_arr = vc_json_obj->GetArray("step_mesh");

                    if(cp_mesh_arr)
                    {
                        for(std::size_t i = 0; i < cp_mesh_arr->Size(); ++i)
                            _vcStepMesh.push_back(json_get_int_value(cp_mesh_arr->At(i)));
                    }
                }

                _hasVC = true;                
            }
        }
    }

    _sensor = json_get_string_value(data->GetValue("sensor"));
    _valid = true;

    return ProfileErrors::OK;
}

void SensorCalibrationProfile::SaveToFile(const std::string& fileName)
{
    auto json = AtUtils::IJson::Create();
    auto data = json->RootObject();

    to_json(std::move(data), *this);

    bool ret = json->Save(fileName);

    if(!ret)
        std::cerr << "Error writing to " << fileName << std::endl;
}

int SensorCalibrationProfile::GetProfileIndexForResolution(std::string resolution)
{
    for (auto itr = _availableResolutions.begin(); itr != _availableResolutions.end(); itr++)
    {
        if (itr->second == resolution)
        {
            return itr->first;
        }
    }
    return 0; // Return default if error
}

bool SensorCalibrationProfile::SelectResolution(std::string resolution)
{
    bool resolutionFound = false;
    for (auto itr = _availableResolutions.begin(); itr != _availableResolutions.end(); itr++)
    {
        if (itr->second == resolution)
        {
            _selectedResolutionIndex = itr->first;
            _selectedResolution = resolution;
        }
    }
    return resolutionFound;
}

EntryRepresentationStatus SensorCalibrationProfile::IsGainRepresentableInTable(float gain)
{
    if (_perResolutionBlackLevelTable[_selectedResolutionIndex]->find(gain) != _perResolutionBlackLevelTable[_selectedResolutionIndex]->end())
    {
        // If the table contains the value, then it's a direct value we can obtain
        return EntryRepresentationStatus::Direct;
    }

    if(_perResolutionBlackLevelTable[_selectedResolutionIndex]->empty())
        return EntryRepresentationStatus::Invalid;


    auto upperBound = _perResolutionBlackLevelTable[_selectedResolutionIndex]->upper_bound(gain);
    auto lowerBound = std::prev(upperBound);

    if (lowerBound == _perResolutionBlackLevelTable[_selectedResolutionIndex]->end() ||
        upperBound == _perResolutionBlackLevelTable[_selectedResolutionIndex]->end())
    {
        if (_perResolutionBlackLevelTable[_selectedResolutionIndex]->size() > 1)
        {
            return EntryRepresentationStatus::Extrapolate;
        }
        else
        {
            return EntryRepresentationStatus::Invalid;
        }
    }

    return EntryRepresentationStatus::Interpolate;
}

std::vector<float> SensorCalibrationProfile::GetResListOfGains(std::string resolution)
{
    std::string currentRes = _selectedResolution;
    SelectResolution(std::move(resolution));
    auto list = GetListOfGains();
    SelectResolution(std::move(currentRes));
    return list;
}

std::vector<float> SensorCalibrationProfile::GetListOfGains()
{
    std::vector<float> gainList;

    for (auto itr = _perResolutionBlackLevelTable[_selectedResolutionIndex]->begin(); itr != _perResolutionBlackLevelTable[_selectedResolutionIndex]->end(); itr++)
    {
        gainList.push_back((float)itr->first);
    }

    return gainList;
}

AutoWhiteBalanceGainTableEntry SensorCalibrationProfile::GetInterpolatedTableEntryForGain(
    float gain,
    EntryInterpolationMode interpolationMode)
{
    auto foundEntry = _perResolutionBlackLevelTable[_selectedResolutionIndex]->find(gain);

    if (foundEntry != _perResolutionBlackLevelTable[_selectedResolutionIndex]->end())
    {
        return *foundEntry->second;
    }

    // The user should have checked with IsGainRepresentableInTable before
    // calling this function for invalid entries.
    EntryRepresentationStatus entryRepresentationStatus = IsGainRepresentableInTable(gain);

    if (entryRepresentationStatus == EntryRepresentationStatus::Interpolate)
    {
        // Interpolate
        auto upperBound = _perResolutionBlackLevelTable[_selectedResolutionIndex]->upper_bound(gain);
        auto lowerBound = std::prev(upperBound);

        float gainGap = upperBound->first - lowerBound->first;

        float interpolationScalar = (gain - lowerBound->first) / gainGap;

        if (interpolationMode == EntryInterpolationMode::Linear)
        {
            auto firstHalf = (*lowerBound->second * (1.0 - interpolationScalar));
            auto latterHalf = (*upperBound->second * interpolationScalar);

            return (firstHalf + latterHalf);
        }
        else
        {
            float exponentialScalar = std::pow(interpolationScalar, 2); // FIXME - Which scale to use? Should it be user/profile specified?
            return ((*lowerBound->second * (1.0 - exponentialScalar)) + (*upperBound->second * exponentialScalar));
        }
    }
    else
    {
        // Extrapolate if we can (linear only)
        auto upperBound = gain > std::prev(_perResolutionBlackLevelTable[_selectedResolutionIndex]->end())->first ?
            std::prev(_perResolutionBlackLevelTable[_selectedResolutionIndex]->end()) : std::next(_perResolutionBlackLevelTable[_selectedResolutionIndex]->begin());
        auto lowerBound = std::prev(upperBound);

        float boundGap = upperBound->first - lowerBound->first;
        float extraGap = gain > upperBound->first ? gain - upperBound->first : lowerBound->first - gain;
        auto tableGapScaled = (*upperBound->second - *lowerBound->second) * (extraGap / boundGap);

        return (gain > upperBound->first ? *upperBound->second + tableGapScaled : *lowerBound->second - tableGapScaled);
    }
}

void SensorCalibrationProfile::DeleteEntryByGain(float gain)
{
    /*
    std::cout << "Deleting: " << gain << "\n";
    for (auto entry : _perResolutionBlackLevelTable)
    {
        std::cout << entry.first << "\n";
    }
    */

    auto foundEntry = _perResolutionBlackLevelTable[_selectedResolutionIndex]->find(gain);

    //std::cout << foundEntry->first << "\n";

    if (foundEntry == _perResolutionBlackLevelTable[_selectedResolutionIndex]->end())
    {
        return;
    }

    _perResolutionBlackLevelTable[_selectedResolutionIndex]->erase(foundEntry);
}

std::shared_ptr<AutoWhiteBalanceGainTableEntry> SensorCalibrationProfile::FindOrCreateEntryByGain(float gain)
{
    auto foundEntry = _perResolutionBlackLevelTable[_selectedResolutionIndex]->find(gain);

    if (foundEntry != _perResolutionBlackLevelTable[_selectedResolutionIndex]->end())
    {
        return foundEntry->second;
    }

    // Otherwise, we need to add a new, blank entry

    std::shared_ptr<AutoWhiteBalanceGainTableEntry> newEntry = std::make_shared<AutoWhiteBalanceGainTableEntry>();

    _perResolutionBlackLevelTable[_selectedResolutionIndex]->insert(std::pair(gain, newEntry));

    foundEntry = _perResolutionBlackLevelTable[_selectedResolutionIndex]->find(gain);

    if (foundEntry != _perResolutionBlackLevelTable[_selectedResolutionIndex]->end())
    {
        return foundEntry->second;
    }
    // This shouldn't happen, but will stop the app crashing if the entry was not added to the table successfully.
    return newEntry;
}

bool SensorCalibrationProfile::UpdateBlcParametersForGain(float gain, uint32_t blcPedestals[4], uint32_t blcScalars[4])
{
    auto entry = FindOrCreateEntryByGain(gain);

    for (int i = 0; i < 4; i++)
    {
        entry->_BlcPedestals[i] = blcPedestals[i];
        entry->_BlcScalars[i] = blcScalars[i];
    }


    return true;
}

bool SensorCalibrationProfile::UpdateAnrParametersForGain(float gain, float darkNoise, float combinedNoise)
{
    auto entry = FindOrCreateEntryByGain(gain);

    entry->_DarkNoise = darkNoise;
    entry->_CombinedNoise = combinedNoise;

    return true;
}

EntryRepresentationStatus SensorCalibrationProfile::IsColourTempRepresentableInTable(uint16_t colourTemp)
{
    if (_colourTempTable.find(colourTemp) != _colourTempTable.end())
    {
        // If the table contains the value, then it's a direct value we can obtain
        return EntryRepresentationStatus::Direct;
    }

    auto upperBound = _colourTempTable.upper_bound(colourTemp);
    auto lowerBound = std::prev(upperBound);

    if (lowerBound == _colourTempTable.end() ||
        upperBound == _colourTempTable.end())
    {
        if (_colourTempTable.size() > 1)
        {
            return EntryRepresentationStatus::Extrapolate;
        }
        else
        {
            return EntryRepresentationStatus::Invalid;
        }
    }

    return EntryRepresentationStatus::Interpolate;
}

std::vector<uint16_t> SensorCalibrationProfile::GetListOfTemps()
{
    std::vector<uint16_t> tempList;

    for (auto itr = _colourTempTable.begin(); itr != _colourTempTable.end(); itr++)
    {
        tempList.push_back((uint16_t)itr->first);
    }

    return tempList;
}

AutoWhiteBalanceColourTempTableEntry SensorCalibrationProfile::GetInterpolatedTableEntryForTemperature(
    uint16_t colourTemp,
    EntryInterpolationMode interpolationMode)
{
    auto foundEntry = _colourTempTable.find(colourTemp);

    if (foundEntry != _colourTempTable.end())
    {
        return *foundEntry->second;
    }

    // The user should have checked with IsColourTempRepresentableInTable before
    // calling this function for invalid entries.
    EntryRepresentationStatus entryRepresentationStatus = IsColourTempRepresentableInTable(colourTemp);

    if (entryRepresentationStatus == EntryRepresentationStatus::Interpolate)
    {
        // Interpolate
        auto upperBound = _colourTempTable.upper_bound(colourTemp);
        auto lowerBound = std::prev(upperBound);

        float colourTempGap = upperBound->first - lowerBound->first;

        float interpolationScalar = (colourTemp - lowerBound->first) / colourTempGap;

        // FIXME - does it even make sense to interpolate colours non-linearly?
        if (interpolationMode == EntryInterpolationMode::Linear)
        {
            auto firstHalf = (*lowerBound->second * (1.0 - interpolationScalar));
            auto latterHalf = (*upperBound->second * interpolationScalar);

            return (firstHalf + latterHalf);
        }
        else
        {
            float exponentialScalar = std::pow(interpolationScalar, 2); // FIXME - Which scale to use? Should it be user/profile specified?
            return ((*lowerBound->second * (1.0 - exponentialScalar)) + (*upperBound->second * exponentialScalar));
        }
    }
    else
    {
        // Extrapolate if we can (linear only)
        auto upperBound = colourTemp > std::prev(_colourTempTable.end())->first ? std::prev(_colourTempTable.end()) : std::next(_colourTempTable.begin());
        auto lowerBound = std::prev(upperBound);

        float boundGap = upperBound->first - lowerBound->first;
        float extraGap = colourTemp > upperBound->first ? colourTemp - upperBound->first : lowerBound->first - colourTemp;
        auto tableGapScaled = (*upperBound->second - *lowerBound->second) * (extraGap / boundGap);

        return (colourTemp > upperBound->first ? *upperBound->second + tableGapScaled : *lowerBound->second - tableGapScaled);
    }
}

void SensorCalibrationProfile::DeleteEntryByTemp(uint16_t temp)
{
    auto foundEntry = _colourTempTable.find(temp);

    if (foundEntry == _colourTempTable.end())
    {
        return;
    }

    _colourTempTable.erase(foundEntry);
}

std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> SensorCalibrationProfile::FindOrCreateEntryByColourTemp(uint16_t temp)
{
    auto foundEntry = _colourTempTable.find(temp);

    if (foundEntry != _colourTempTable.end())
    {
        return foundEntry->second;
    }

    // Otherwise, we need to add a new, blank entry

    std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> newEntry = std::make_shared<AutoWhiteBalanceColourTempTableEntry>();

    _colourTempTable.insert(std::pair(temp, newEntry));

    foundEntry = _colourTempTable.find(temp);

    if (foundEntry != _colourTempTable.end())
    {
        return foundEntry->second;
    }
    // This shouldn't happen, but will stop the app crashing if the entry was not added to the table successfully.
    return newEntry;

}

bool SensorCalibrationProfile::UpdateCcmCoeffsForTemp(uint16_t temp, float ccmCoeffs[3][4])
{
    auto entry = FindOrCreateEntryByColourTemp(temp);

    entry->_HasCcm = true;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            entry->_CcmCoeffs[i][j] = ccmCoeffs[i][j];
        }
    }

    return true;
}

void SensorCalibrationProfile::ChangeBaselineColourTemp(uint16_t newTemp)
{
    // Changing the baseline temperature means that the recordings need to be redone
    _baselineColourTemp = newTemp;
}

uint16_t SensorCalibrationProfile::GetBaselineColourTemp()
{
    return _baselineColourTemp;
}

EntryRepresentationStatus SensorCalibrationProfile::IsGainScaleTempRepresentableInTable(float gain)
{
    if (_gainScaleBaselineTable.find(gain) != _gainScaleBaselineTable.end())
    {
        // If the table contains the value, then it's a direct value we can obtain
        return EntryRepresentationStatus::Direct;
    }

    auto upperBound = _gainScaleBaselineTable.upper_bound(gain);
    auto lowerBound = std::prev(upperBound);

    if (lowerBound == _gainScaleBaselineTable.end() ||
        upperBound == _gainScaleBaselineTable.end())
    {
        if (_gainScaleBaselineTable.size() > 1)
        {
            return EntryRepresentationStatus::Extrapolate;
        }
        else
        {
            return EntryRepresentationStatus::Invalid;
        }
    }

    return EntryRepresentationStatus::Interpolate;
}

std::vector<float> SensorCalibrationProfile::GetListOfGainScaleReadings()
{
    std::vector<float> gainList;

    for (auto itr = _gainScaleBaselineTable.begin(); itr != _gainScaleBaselineTable.end(); itr++)
    {
        gainList.push_back((float)itr->first);
    }

    return gainList;
}

AutoWhiteBalanceColourTempTableEntry SensorCalibrationProfile::GetInterpolatedTableEntryForGainScale(float gain, EntryInterpolationMode interpolationMode)
{
    auto foundEntry = _gainScaleBaselineTable.find(gain);

    if (foundEntry != _gainScaleBaselineTable.end())
    {
        return *foundEntry->second;
    }

    // The user should have checked with IsColourTempRepresentableInTable before
    // calling this function for invalid entries.
    EntryRepresentationStatus entryRepresentationStatus = IsGainScaleTempRepresentableInTable(gain);

    if (entryRepresentationStatus == EntryRepresentationStatus::Interpolate)
    {
        // Interpolate
        auto upperBound = _gainScaleBaselineTable.upper_bound(gain);
        auto lowerBound = std::prev(upperBound);

        float gainGap = upperBound->first - lowerBound->first;

        float interpolationScalar = (gain - lowerBound->first) / gainGap;

        // FIXME - does it even make sense to interpolate colours non-linearly?
        if (interpolationMode == EntryInterpolationMode::Linear)
        {
            auto firstHalf = (*lowerBound->second * (1.0 - interpolationScalar));
            auto latterHalf = (*upperBound->second * interpolationScalar);

            return (firstHalf + latterHalf);
        }
        else
        {
            float exponentialScalar = std::pow(interpolationScalar, 2); // FIXME - Which scale to use? Should it be user/profile specified?
            return ((*lowerBound->second * (1.0 - exponentialScalar)) + (*upperBound->second * exponentialScalar));
        }
    }
    else
    {
        // Extrapolate if we can (linear only)
        auto upperBound = gain > std::prev(_gainScaleBaselineTable.end())->first ? std::prev(_gainScaleBaselineTable.end()) : std::next(_gainScaleBaselineTable.begin());
        auto lowerBound = std::prev(upperBound);

        float boundGap = upperBound->first - lowerBound->first;
        float extraGap = gain > upperBound->first ? gain - upperBound->first : lowerBound->first - gain;
        auto tableGapScaled = (*upperBound->second - *lowerBound->second) * (extraGap / boundGap);

        return (gain > upperBound->first ? *upperBound->second + tableGapScaled : *lowerBound->second - tableGapScaled);
    }
}

void SensorCalibrationProfile::DeleteGainScaleEntryByGain(float gain)
{
    auto foundEntry = _gainScaleBaselineTable.find(gain);

    if (foundEntry == _gainScaleBaselineTable.end())
    {
        return;
    }

    _gainScaleBaselineTable.erase(foundEntry);
}

std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> SensorCalibrationProfile::FindOrCreateGainScaleTempEntry(float gain)
{
    auto foundEntry = _gainScaleBaselineTable.find(gain);

    if (foundEntry != _gainScaleBaselineTable.end())
    {
        return foundEntry->second;
    }

    // Otherwise, we need to add a new, blank entry

    std::shared_ptr<AutoWhiteBalanceColourTempTableEntry> newEntry = std::make_shared<AutoWhiteBalanceColourTempTableEntry>();

    _gainScaleBaselineTable.insert(std::pair(gain, newEntry));

    foundEntry = _gainScaleBaselineTable.find(gain);

    if (foundEntry != _gainScaleBaselineTable.end())
    {
        return foundEntry->second;
    }
    // This shouldn't happen, but will stop the app crashing if the entry was not added to the table successfully.
    return newEntry;
}

// Vignette correction

std::vector<uint32_t> SensorCalibrationProfile::GetVCCpMesh(uint8_t cp)
{
    if (cp > _vcNumColorPlanes)
    {
        std::cout << "WARNING: Requesting mesh outside of given number of planes\n";
        return _vcCpMesh[0];
    }
    return _vcCpMesh[cp];
}

void SensorCalibrationProfile::LoadVCConfigIntoProfile(uint8_t horizMeshSize, uint8_t vertiMeshSize,
                                                       uint16_t curResX, uint16_t curResY, uint8_t numColorPlanes,
                                                       std::vector<uint32_t> *cp0Mesh,
                                                       std::vector<uint32_t> *cp1Mesh,
                                                       std::vector<uint32_t> *cp2Mesh,
                                                       std::vector<uint32_t> *cp3Mesh,
                                                       std::vector<uint32_t> stepMesh)
{
    _vcHorizMeshPoints = horizMeshSize;
    _vcVertiMeshPoints = vertiMeshSize;
    _vcNumColorPlanes = numColorPlanes;
    _vcStepMeshResX = curResX;
    _vcStepMeshResY = curResY;

    if (!cp0Mesh)
    {
        std::cout << "ERROR: VC CP 0 mesh not provided!\n";
        return;
    }

    _vcCpMesh[0] = *cp0Mesh;
    if (numColorPlanes > 1)
    {
        if (!cp1Mesh)
        {
            std::cout << "ERROR: VC CP 1 mesh not provided!\n";
            return;
        }
        _vcCpMesh[1] = *cp1Mesh;
    }
    if (numColorPlanes > 2)
    {
        if (!cp2Mesh)
        {
            std::cout << "ERROR: VC CP 2 mesh not provided!\n";
            return;
        }
        _vcCpMesh[2] = *cp2Mesh;
    }
    if (numColorPlanes > 3)
    {
        if (!cp3Mesh)
        {
            std::cout << "ERROR: VC CP 3 mesh not provided!\n";
            return;
        }
        _vcCpMesh[3] = *cp3Mesh;
    }
    _vcStepMesh = std::move(stepMesh);

    _hasVC = true;
}

std::vector<float> SensorCalibrationProfile::ResampleVCMeshRegardingStepMesh(std::vector<float> intensityMesh)
{
    if (!_hasVC)
    {
        return {0};
    }
    // TODO - implement this
    return {0};
}

// Helper stuff

std::ostream& operator<<(std::ostream& o, const AutoWhiteBalanceGainTableEntry& tableEntry)
{
    o << "Current entry: " << "\n";

    o << "Blc Pedestal: ";
    for (uint32_t i = 0; i < 4; i++)
    {
        o << tableEntry._BlcPedestals[i] << " ";
    }
    o << "\n";

    o << "Blc Scalar: ";
    for (uint32_t i = 0; i < 4; i++)
    {
        o << tableEntry._BlcScalars[i] << " ";
    }
    o << "\n";
    return o;
}

std::ostream& operator<<(std::ostream& o, const AutoWhiteBalanceColourTempTableEntry& tableEntry)
{
    o << "Current Color Table entry: " << "\n";
    o << "Recorded gain: " << tableEntry._Gain << "\n";
    o << "Has Raw: " << tableEntry._HasRaw << "\n";
    if (tableEntry._HasRaw)
    {
        o << "Raw values:\n";
        o << tableEntry._Raw << "\n";
        o << "Format: " << tableEntry._RawFormat << "\n";
    }

    o << "_WbsRatio: " << tableEntry._WbsRatio << "\n";
    o << "_GainScaledWbsRatio: " << tableEntry._GainScaledWbsRatio << "\n";

    //o << "_WbcCoeffs: {" << tableEntry._WbcCoeffs[0] << ", " << tableEntry._WbcCoeffs[1] << ", " << tableEntry._WbcCoeffs[2] << ", " << tableEntry._WbcCoeffs[3] << "}\n";
    o << "_ChanScalars: {" << tableEntry._ChanScalars[0] << ", " << tableEntry._ChanScalars[1] << ", " << tableEntry._ChanScalars[2] << "}\n";
    o << "Ccm: [\n";
    for (uint32_t level = 0; level < 3; level++)
    {
        o << "[";
        for (uint32_t i = 0; i < 4; i++)
        {
            o << tableEntry._CcmCoeffs[level][i] << " ";
        }
        o << "]\n";
    }
    o << "]\n";
    return o;
}

const char* ToString(const ProfileErrors& error)
{
    switch (error)
    {
        case ProfileErrors::OK:
        {
            return "OK";
        }
        case ProfileErrors::AnalogueGainOutOfRecordedRange:
        {
            return "Sensor gain OOR";
        }
        case ProfileErrors::SceneTempOutOfRecordedRange:
        {
            return "Target temp OOR";
        }
        case ProfileErrors::CoresNotResponding:
        {
            return "Cores not responding";
        }
        case ProfileErrors::ProfileIOLoadError:
        {
            return "File IO failed";
        }
        case ProfileErrors::ProfileVersInvalid:
        {
            return "Profile incompatible";
        }
        case ProfileErrors::ProfileIncomplete:
        {
            return "Profile incomplete";
        }
        case ProfileErrors::ProfileErrUnknown:
        default:
        {
            return "Unknown error";
        }
    }
}