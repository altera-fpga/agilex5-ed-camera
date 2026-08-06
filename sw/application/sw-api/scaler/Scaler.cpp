/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <cstdio>
#include "Scaler.h"
#include "scaler_coeffs.h"


namespace SwApi
{
    std::shared_ptr<Scaler> Scaler::Create( Hapi::VvpScalerPtr spScaler)
    {
        std::shared_ptr<Scaler> scaler{nullptr};

        try{
            scaler = std::make_shared<Scaler>(spScaler);
        }
        catch(const std::exception& e){
            std::cerr << e.what() << '\n';
        }
        
        return scaler;
    }


    Scaler::Scaler(Hapi::VvpScalerPtr spVvpScaler):
        _spVvpScaler(spVvpScaler)
    {
        if(!_spVvpScaler)
            throw std::runtime_error("Scaler: Hapi instance is null");

        //PrintCoreConfig();
    }


    Scaler::~Scaler()
    {
    }


    bool Scaler::GetLiteMode()
    {
        return intel_vvp_scaler_get_lite_mode(_spVvpScaler->GetInstance());
    }


    // Required only if the core is configured in "Lite" mode
    void Scaler::SetInputResolution(const uint32_t width, const uint32_t height)
    {
        intel_vvp_core_set_img_info_width(_spVvpScaler->GetInstance(), width);
        intel_vvp_core_set_img_info_height(_spVvpScaler->GetInstance(), height);
    }


    void Scaler::SetOutputResolution(const uint32_t width, const uint32_t height)
    {
        intel_vvp_scaler_set_output_width(_spVvpScaler->GetInstance(), width);
        intel_vvp_scaler_set_output_height(_spVvpScaler->GetInstance(), height);
    }


    bool Scaler::SetCoefficientBanks(const uint32_t v_bank, const uint32_t h_bank)
    {
        const auto instance = _spVvpScaler->GetInstance();

        bool ret_v = intel_vvp_scaler_set_v_bank(instance, v_bank ) == kIntelVvpCoreOk;
        bool ret_h = (intel_vvp_scaler_set_h_bank(instance, h_bank ) == kIntelVvpCoreOk);

        return ret_v && ret_h;
    }


    void Scaler::Commit()
    {
        intel_vvp_scaler_commit_writes(_spVvpScaler->GetInstance());
    }


    bool Scaler::GenerateAndLoadCoefficients(const pair_t& input, const pair_t& output, const pair_t& banks)
    {
        bool ret = false;

        const auto instance = _spVvpScaler->GetInstance();
        const bool coeffLoadingEnabled = intel_vvp_scaler_is_coeffs_loading_enabled(instance);

        if(coeffLoadingEnabled)
        {
            const bool vScalingEnabled = intel_vvp_scaler_is_vertical_scaling_enabled(instance);
            bool vScalingOk =  true;

            if(vScalingEnabled)
            {
                const auto num_banks = intel_vvp_scaler_get_num_vertical_banks(instance);

                if((input.second > 0) && (output.second > 0))
                {
                    const auto height_in = static_cast<int>(input.second);
                    const auto height_out = static_cast<int>(output.second);
                    scaler_create_and_load_vertical_coefficents(instance, height_in, height_out, banks.second % num_banks);

                    std::cout << "Vertical scaler coefficients applied\n";
                }
                else
                    vScalingOk = false;
            }

            if(vScalingOk)
            {
                const bool hScalingEnabled = intel_vvp_scaler_is_horizontal_scaling_enabled(instance);
                bool hScalingOk =  true;

                if(hScalingEnabled)
                {
                    const auto num_banks = intel_vvp_scaler_get_num_horizontal_banks(instance);

                    if((input.first > 0) && (output.first > 0))
                    {
                        const auto width_in = static_cast<int>(input.first);
                        const auto width_out = static_cast<int>(output.first);
                        scaler_create_and_load_horizontal_coefficents(instance, width_in, width_out, banks.first % num_banks);

                        std::cout << "Horizontal scaler coefficients applied\n";
                    }
                    else
                        hScalingOk = false;
                }

                ret = vScalingOk && hScalingOk;
            }
        }
        else
            std::cerr << "Coefficient loading not enabled in the Scaler core!\n";        

        return ret;
    }


    void Scaler::PrintCoreConfig()
    {
        const auto instance = _spVvpScaler->GetInstance();

        printf("#Scaler\n");
        printf("\tMode: %s\n", GetLiteMode() ? "Lite" : "Full");
        printf("\tDebug: %s\n", intel_vvp_scaler_get_debug_enabled(instance) ? "Enabled" : "Disabled");
        printf("\tPixels in Parallel: %d\n", intel_vvp_scaler_get_pixels_in_parallel(instance));
        printf("\tMax Input Width: %d\n", intel_vvp_scaler_get_max_input_width(instance));
        printf("\tMax Output Width: %d\n", intel_vvp_scaler_get_max_output_width(instance));
        printf("\tScaling Algorithm: %s\n", intel_vvp_scaler_get_scaling_algorithm(instance) == kIntelVvpScalerNearestNeighborScaling ? "NearestNeighbour" : (intel_vvp_scaler_get_scaling_algorithm(instance) == kIntelVvpScalerBilinearScaling ? "Bilinear" : "Polyphase"));
        printf("\tCoeffs Loading: %s\n", intel_vvp_scaler_is_coeffs_loading_enabled(instance) ? "Enabled" : "Disabled");
        printf("\tCoeffs Preinitialized: %s\n", intel_vvp_scaler_are_coeffs_preinitialized(instance) ? "Yes" : "No");

        const bool vScalingEnabled = intel_vvp_scaler_is_vertical_scaling_enabled(instance);
        
        printf("\tVertical Scaling: %s\n", vScalingEnabled ? "Enabled" : "Disabled");

        if (vScalingEnabled) {
            printf("\t\tPartial Vertical Scaling: %s\n", intel_vvp_scaler_is_partial_vertical_scaling_enabled(instance) ? "Enabled" : "Disabled");
            printf("\t\tNumber of Vertical Taps: %d\n", intel_vvp_scaler_get_num_vertical_taps(instance));
            printf("\t\tNumber of Vertical Phases: %d\n", intel_vvp_scaler_get_num_vertical_phases(instance));
            printf("\t\tNumber of Vertical Banks: %d\n", intel_vvp_scaler_get_num_vertical_banks(instance));
            printf("\t\tVertical Coeffs Signed: %s\n", intel_vvp_scaler_are_vertical_coeffs_signed(instance) ? "Yes" : "No");
            printf("\t\tVertical Coeffs Int Bits: %d\n", intel_vvp_scaler_get_vertical_coeffs_int_bits(instance));
            printf("\t\tVertical Coeffs Frac Bits: %d\n", intel_vvp_scaler_get_vertical_coeffs_frac_bits(instance));
        }

        const bool hScalingEnabled = intel_vvp_scaler_is_horizontal_scaling_enabled(instance);

        printf("\tHorizontal Scaling: %s\n", hScalingEnabled ? "Enabled" : "Disabled");

        if (hScalingEnabled) {
            printf("\t\tPartial Horizontal Scaling: %s\n", intel_vvp_scaler_is_partial_horizontal_scaling_enabled(instance) ? "Enabled" : "Disabled");
            printf("\t\tNumber of Horizontal Taps: %d\n", intel_vvp_scaler_get_num_horizontal_taps(instance));
            printf("\t\tNumber of Horizontal Phases: %d\n", intel_vvp_scaler_get_num_horizontal_phases(instance));
            printf("\t\tNumber of Horizontal Banks: %d\n", intel_vvp_scaler_get_num_horizontal_banks(instance));
            printf("\t\tHorizontal Coeffs Signed: %s\n", intel_vvp_scaler_are_horizontal_coeffs_signed(instance) ? "Yes" : "No");
            printf("\t\tHorizontal Coeffs Int Bits: %d\n", intel_vvp_scaler_get_horizontal_coeffs_int_bits(instance));
            printf("\t\tHorizontal Coeffs Frac Bits: %d\n", intel_vvp_scaler_get_horizontal_coeffs_frac_bits(instance));
        }
    }


}; // namespace SwApi
