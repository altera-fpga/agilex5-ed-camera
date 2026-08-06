/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "Blc.h"
#include "intel_vvp_blc.h"
#include "intel_vvp_core.h"

namespace SwApi {

    std::shared_ptr<Blc> Blc::Create(Hapi::VvpBlcPtr spBlc,
                                    uint32_t initialOutputWidth,
                                    uint32_t initialOutputHeight)
    {
        return std::make_shared<Blc>(spBlc, initialOutputWidth, initialOutputHeight);
    }


    Blc::Blc(Hapi::VvpBlcPtr spBlc, uint32_t initialOutputWidth, uint32_t initialOutputHeight):
        VvpCoreBase("BLC"),
        _spBlc(spBlc),
        _bypass{false},
        _cfaPhase{TCfaPhase::RGGB},
        _clipZero{false}
    {
        // Set the output width and height.
        SetResolution(initialOutputWidth, initialOutputHeight);

        for(uint32_t i = 0; i < CFA_SIZE; ++i)
        {
            _pedestals[i] = 0;
            _scalers[i] = 0;
        }
    }


    bool Blc::SetBypass(bool bypass, UpdatePolicy policy)
    {
        auto update_hw = [this, bypass]()->bool {
            int rv = intel_vvp_blc_set_bypass(_spBlc->GetInstance(), bypass);
            return (rv == kIntelVvpCoreOk || rv == kIntelVvpBlcCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
            _bypass = bypass;
        else
            std::cerr << "[" << GetName() << "] Failed to change bypass setting\n";

        return ret;
    }


    bool Blc::GetBypass()
    {
        return intel_vvp_blc_get_bypass(_spBlc->GetInstance());
    }


    bool Blc::SetCfaPhase(TCfaPhase to, UpdatePolicy policy)
    {
        auto update_hw = [this, to]()->bool {
            int rv = intel_vvp_blc_set_cfa_phase(_spBlc->GetInstance(), static_cast<uint8_t>(to));
            return (rv == kIntelVvpCoreOk || rv == kIntelVvpBlcCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
            _cfaPhase = to;
        else
            std::cerr << "[" << GetName() << "] Failed to set CFA phase\n";

        return ret;
    }


    TCfaPhase Blc::GetCfaPhase()
    {
        return _cfaPhase;
    }


    bool Blc::SetClipZero(bool to, UpdatePolicy policy)
    {
        auto update_hw = [this, to]()->bool {
            int rv = intel_vvp_blc_set_clip_zero_en(_spBlc->GetInstance(), to);
            return (rv == kIntelVvpCoreOk || rv == kIntelVvpBlcCommitPendingErr);
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
            _clipZero = to;
        else
            std::cerr << "[" << GetName() << "] Failed to set clip zero\n";

        return ret;
    }


    bool Blc::GetClipZero()
    {
        return intel_vvp_blc_get_clip_zero_en(_spBlc->GetInstance());
    }


    bool Blc::SetBlackPedestalByIdx(uint8_t index, uint32_t to, UpdatePolicy policy)
    {
        bool ret = false;

        if(index < CFA_SIZE)
        {
            auto update_hw = [this, index, to]()->bool {

                int rv = kIntelVvpBlcParameterErr;

                switch (index)
                {
                    case 0b00:
                    {
                        rv = intel_vvp_blc_set_cfa_00_black_pedestal(_spBlc->GetInstance(), to);
                        break;
                    }
                    case 0b01:
                    {
                        rv = intel_vvp_blc_set_cfa_01_black_pedestal(_spBlc->GetInstance(), to);
                        break;
                    }
                    case 0b10:
                    {
                        rv = intel_vvp_blc_set_cfa_10_black_pedestal(_spBlc->GetInstance(), to);
                        break;
                    }
                    case 0b11:
                    {
                        rv = intel_vvp_blc_set_cfa_11_black_pedestal(_spBlc->GetInstance(), to);
                        break;
                    }
                    default:
                        break;
                }

                return (rv == kIntelVvpCoreOk || rv == kIntelVvpBlcCommitPendingErr);
            };

            ret = UpdateHw(update_hw, policy);

            if(ret)
                _pedestals[index] = to;
            else
                std::cerr << "[" << GetName() << "] Failed to set black pedestal for index " << static_cast<uint32_t>(index) << "\n";
        }

        return ret;
    }


    uint32_t Blc::GetBlackPedestalByIdx(uint8_t index)
    {
        switch (index)
        {
            case 0b00:
            {
                return intel_vvp_blc_get_cfa_00_black_pedestal(_spBlc->GetInstance());
            }
            case 0b01:
            {
                return intel_vvp_blc_get_cfa_01_black_pedestal(_spBlc->GetInstance());
            }
            case 0b10:
            {
                return intel_vvp_blc_get_cfa_10_black_pedestal(_spBlc->GetInstance());
            }
            case 0b11:
            {
                return intel_vvp_blc_get_cfa_11_black_pedestal(_spBlc->GetInstance());
            }
            default:
                // We received an invalid index.
                return 0xFFFF;
        }
    }

    std::vector<uint32_t> Blc::GetBlackPedestals()
    {
        auto instance = _spBlc->GetInstance();
        return {intel_vvp_blc_get_cfa_00_black_pedestal(instance),
                intel_vvp_blc_get_cfa_01_black_pedestal(instance),
                intel_vvp_blc_get_cfa_10_black_pedestal(instance),
                intel_vvp_blc_get_cfa_11_black_pedestal(instance)};
    }

    bool Blc::SetColorScalerByIdx(uint8_t index, uint32_t to, UpdatePolicy policy)
    {
        bool ret = false;

        if(index < CFA_SIZE)
        {
            auto update_hw = [this, index, to]()->bool {
                bool ret = false;
                int rv = kIntelVvpBlcParameterErr;

                switch (index)
                {
                    case 0b00:
                    {
                        rv = intel_vvp_blc_set_cfa_00_color_scaler(_spBlc->GetInstance(), to);
                        break;
                    }
                    case 0b01:
                    {
                        rv = intel_vvp_blc_set_cfa_01_color_scaler(_spBlc->GetInstance(), to);
                        break;
                    }
                    case 0b10:
                    {
                        rv = intel_vvp_blc_set_cfa_10_color_scaler(_spBlc->GetInstance(), to);
                        break;
                    }
                    case 0b11:
                    {
                        rv = intel_vvp_blc_set_cfa_11_color_scaler(_spBlc->GetInstance(), to);
                        break;
                    }
                    default:
                        break;
                }

                return (rv == kIntelVvpCoreOk || rv == kIntelVvpBlcCommitPendingErr);
            };

            ret = UpdateHw(update_hw, policy);

            if(ret)
                _scalers[index] = to;
            else
                std::cerr << "[" << GetName() << "] Failed to set color scaler for index " << static_cast<uint32_t>(index) << "\n";
        }

        return ret;
    }

    uint32_t Blc::GetColorScalerByIdx(uint8_t index)
    {
        switch (index)
        {
            case 0b00:
            {
                return intel_vvp_blc_get_cfa_00_color_scaler(_spBlc->GetInstance());
            }
            case 0b01:
            {
                return  intel_vvp_blc_get_cfa_01_color_scaler(_spBlc->GetInstance());
            }
            case 0b10:
            {
                return  intel_vvp_blc_get_cfa_10_color_scaler(_spBlc->GetInstance());
            }
            case 0b11:
            {
                return intel_vvp_blc_get_cfa_11_color_scaler(_spBlc->GetInstance());
            }
            default:
                // We received an invalid index, return an error value.
                return 0x3FFFFu; // a.k.a. 18 bits of 1s
        }
    }


    bool Blc::SetPedestalsAndScalers(uint32_t pedestals[CFA_SIZE], uint32_t scalers[CFA_SIZE], UpdatePolicy policy)
    {
        auto update_hw = [this, &pedestals, &scalers]()->bool {
            bool success = true;
            auto instance = _spBlc->GetInstance();

            int rv[CFA_SIZE] = {
                intel_vvp_blc_set_cfa_00_black_pedestal(instance, pedestals[0]),
                intel_vvp_blc_set_cfa_01_black_pedestal(instance, pedestals[1]),
                intel_vvp_blc_set_cfa_10_black_pedestal(instance, pedestals[2]),
                intel_vvp_blc_set_cfa_11_black_pedestal(instance, pedestals[3])
            };

            success = success &&
                ((rv[0] == kIntelVvpCoreOk) || (rv[0] == kIntelVvpBlcCommitPendingErr)) &&
                ((rv[1] == kIntelVvpCoreOk) || (rv[1] == kIntelVvpBlcCommitPendingErr)) &&
                ((rv[2] == kIntelVvpCoreOk) || (rv[2] == kIntelVvpBlcCommitPendingErr)) &&
                ((rv[3] == kIntelVvpCoreOk) || (rv[3] == kIntelVvpBlcCommitPendingErr));

            if(success)
            {
                int rv[CFA_SIZE] = {
                    intel_vvp_blc_set_cfa_00_color_scaler(instance, scalers[0]),
                    intel_vvp_blc_set_cfa_01_color_scaler(instance, scalers[1]),
                    intel_vvp_blc_set_cfa_10_color_scaler(instance, scalers[2]),
                    intel_vvp_blc_set_cfa_11_color_scaler(instance, scalers[3])
                };

                success = success &&
                    ((rv[0] == kIntelVvpCoreOk) || (rv[0] == kIntelVvpBlcCommitPendingErr)) &&
                    ((rv[1] == kIntelVvpCoreOk) || (rv[1] == kIntelVvpBlcCommitPendingErr)) &&
                    ((rv[2] == kIntelVvpCoreOk) || (rv[2] == kIntelVvpBlcCommitPendingErr)) &&
                    ((rv[3] == kIntelVvpCoreOk) || (rv[3] == kIntelVvpBlcCommitPendingErr));
            }

            return success;
        };

        bool ret = UpdateHw(update_hw, policy);

        if(ret)
        {
            for(uint32_t i = 0; i < CFA_SIZE; ++i)
            {
                _pedestals[i] = pedestals[i];
                _scalers[i] = scalers[i];
            }
        }
        else
            std::cerr << "[" << GetName() << "] Failed to set pedestals and scalers\n";

        return ret;
    }

    bool Blc::SetResolution(uint32_t width, uint32_t height)
    {
        bool ret = true;

        ret = ret && (intel_vvp_core_set_img_info_width(_spBlc->GetInstance(), width) == kIntelVvpCoreOk);
        ret = ret && (intel_vvp_core_set_img_info_height(_spBlc->GetInstance(), height) == kIntelVvpCoreOk);

        return ret;
    }

    bool Blc::CommitSettings()
    {
        return (intel_vvp_blc_commit(_spBlc->GetInstance()) == kIntelVvpCoreOk);
    }

    bool Blc::IsCommitPending()
    {
        return intel_vvp_blc_commit_is_pending(_spBlc->GetInstance());
    }

} // namespace SwApi
