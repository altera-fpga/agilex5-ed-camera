/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Wbc.h"
#include "intel_vvp_wbc.h"


namespace SwApi 
{

    std::shared_ptr<Wbc> Wbc::Create(Hapi::VvpWbcPtr spWbc, uint32_t initialOutputWidth, uint32_t initialOutputHeight) 
    {
        return std::make_shared<Wbc>(spWbc, initialOutputWidth, initialOutputHeight);
    }


    Wbc::Wbc(Hapi::VvpWbcPtr spWbc, uint32_t initialOutputWidth, uint32_t initialOutputHeight):
        VvpCoreBase("WBC"),
        _spWbc{spWbc},
        _cfaPhase{TCfaPhase::RGGB}
    {
        // Set the output width and height.
        SetResolution(initialOutputWidth, initialOutputHeight);

        for(uint32_t i = 0; i < CFA_SIZE; ++i)
            _scalers[i] = 2048;            
    }        


    bool Wbc::SetBypass(bool bypass, UpdatePolicy policy) 
    {
        auto update_hw = [instance = _spWbc->GetInstance(), bypass]()->bool {
            int rv = intel_vvp_wbc_set_bypass(instance, bypass);
            return (rv == kIntelVvpCoreOk || rv == kIntelVvpWbcCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(!ret)
            std::cerr << "[" << GetName() << "] Failed to change bypass setting\n";

        return ret;
    }


    bool Wbc::GetBypass() 
    {
        return intel_vvp_wbc_get_bypass(_spWbc->GetInstance());
    }


    bool Wbc::IsRunning() 
    {
        return intel_vvp_wbc_is_running(_spWbc->GetInstance());
    }


    bool Wbc::SetCfaPhase(TCfaPhase to, UpdatePolicy policy) 
    {
        auto update_hw = [this, to]()->bool {
            int rv = intel_vvp_wbc_set_cfa_phase(_spWbc->GetInstance(), static_cast<uint8_t>(to));
            return (rv == kIntelVvpCoreOk || rv == kIntelVvpWbcCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
            _cfaPhase = to;
        else
            std::cerr << "[" << GetName() << "] Failed to set CFA phase\n";
            
        return ret;
    }        


    TCfaPhase Wbc::GetCfaPhase() 
    {
        return _cfaPhase;
    }


    bool Wbc::SetColorScalerByIdx(uint8_t index, uint32_t to, UpdatePolicy policy) 
    {
        bool ret = false;

        if(index < CFA_SIZE)
        {
            auto update_hw = [this, index, to]()->bool {

                int rv = kIntelVvpWbcParameterErr;
                
                switch (index) 
                {
                    case 0b00: 
                    {
                        rv = intel_vvp_wbc_set_cfa_00_color_scaler(_spWbc->GetInstance(), to);
                        break;
                    }
                    case 0b01: 
                    {
                        rv = intel_vvp_wbc_set_cfa_01_color_scaler(_spWbc->GetInstance(), to);
                        break;
                    }
                    case 0b10: 
                    {
                        rv = intel_vvp_wbc_set_cfa_10_color_scaler(_spWbc->GetInstance(), to);
                        break;
                    }
                    case 0b11: 
                    {
                        rv = intel_vvp_wbc_set_cfa_11_color_scaler(_spWbc->GetInstance(), to);
                        break;
                    }
                    default:
                        break;
                }

                return (rv == kIntelVvpCoreOk || rv == kIntelVvpWbcCommitPendingErr);
            };

            ret = UpdateHw(update_hw, policy);

            if(ret)
                _scalers[index] = to;
            else
                std::cerr << "[" << GetName() << "] Failed to set color scaler for index " << static_cast<uint32_t>(index) << "\n";
        }

        return ret;
    }


    bool Wbc::SetAllColorScalers(const uint32_t scalers[CFA_SIZE], UpdatePolicy policy)
    {
        auto update_hw = [this, &scalers]()->bool {
            auto instance = _spWbc->GetInstance();

            int rv[] = {
                intel_vvp_wbc_set_cfa_00_color_scaler(instance, scalers[0]),
                intel_vvp_wbc_set_cfa_01_color_scaler(instance, scalers[1]),
                intel_vvp_wbc_set_cfa_10_color_scaler(instance, scalers[2]),
                intel_vvp_wbc_set_cfa_11_color_scaler(instance, scalers[3])
            };

            return (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpWbcCommitPendingErr) &&
                   (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpWbcCommitPendingErr) &&
                   (rv[2] == kIntelVvpCoreOk || rv[2] == kIntelVvpWbcCommitPendingErr) &&
                   (rv[3] == kIntelVvpCoreOk || rv[3] == kIntelVvpWbcCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
        {
            for(uint32_t i = 0; i < CFA_SIZE; ++i)
                _scalers[i] = scalers[i];
        }
        else
            std::cerr << "[" << GetName() << "] Failed to set all color scalers\n";

        return ret;
    }


    uint32_t Wbc::GetColorScalerByIdx(uint8_t index) 
    {
        switch (index) 
        {
            case 0b00: 
            {
                return intel_vvp_wbc_get_cfa_00_color_scaler(_spWbc->GetInstance());
            }
            case 0b01: 
            {
                return  intel_vvp_wbc_get_cfa_01_color_scaler(_spWbc->GetInstance());
            }
            case 0b10: 
            {
                return  intel_vvp_wbc_get_cfa_10_color_scaler(_spWbc->GetInstance());
            }
            case 0b11: 
            {
                return intel_vvp_wbc_get_cfa_11_color_scaler(_spWbc->GetInstance());
            }
            default:
                // We received an invalid index, return an error value.
                return 0x3FFFFu; // a.k.a. 18 bits of 1s
        }
    }


    bool Wbc::SetResolution(uint32_t width, uint32_t height) 
    {
        bool ret = false;

        if(intel_vvp_core_set_img_info_width(_spWbc->GetInstance(), width) == kIntelVvpCoreOk) 
        {
            if(intel_vvp_core_set_img_info_height(_spWbc->GetInstance(), height) == kIntelVvpCoreOk) 
            {
                _width = width;
                _height = height;
                ret = true;
            }
        }

        return ret;
    }


    std::pair<uint32_t, uint32_t> Wbc::GetResolution() const
    {
        return {_width, _height};
    }
    

    bool Wbc::CommitSettings()
    {
        return (intel_vvp_wbc_commit(_spWbc->GetInstance()) == kIntelVvpCoreOk);
    }


    bool Wbc::IsCommitPending()
    {
        return intel_vvp_wbc_commit_is_pending(_spWbc->GetInstance());
    }

} // namespace SwApi
