/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpBlc.h"
#include "VvpCoreBase.h"
#include "IspCommon.h"
#include <cstdint>
#include <vector>

namespace SwApi {

/**
 * High-level interface definition for the VVP Black Level Correction IP.
 *
 * The BLC IP performs a subtraction followed by a multiplication on input color values.
 * For a given input, it subtracts the appropriate 'black pedestal', then scales
 * by the appropriate 'color scaler'.
 *
 * These values can be set using SetBlackPedestalByIdx() and SetColorScalerByIdx().
 *
 * The indices used by these functions correspond to positions in the CFA array.
 * e.g. in the case of RGGB, they mean:
 *
 *                     R (0b00) | G (0b01)
 *                     ---------+---------
 *                     G (0b10) | B (0b11)
 *
 * So calling SetColorScalerByIdx(0b10, x) will set the multiplier used for the
 * bottom-left 'G' values.
 *
 * Finally, SetClipZero() allows you to specify what should happen in the case
 * of underflow after the subtraction of the black pedestal.
 * If the core was built with this, SetClipZero(false) will mean underflows will
 * be reflected around zero. You can check if this is available with CanReflectAroundZero().
 *
 * The BLC is a Bayer-only IP, so you MUST specify the correct CFA Phase using SetCfaPhase().
 */    

class Blc : public VvpCoreBase 
{
public:
    static constexpr uint32_t CFA_SIZE = 4;

    static std::shared_ptr<Blc> Create(Hapi::VvpBlcPtr spBlc, uint32_t initialOutputWidth, uint32_t initialOutputHeight);    

    Blc(Hapi::VvpBlcPtr spBlc, uint32_t initialOutputWidth, uint32_t initialOutputHeight);

    bool SetBypass(bool bypass, UpdatePolicy policy = UpdatePolicy::Async());
    bool GetBypass();

    bool SetCfaPhase(TCfaPhase to, UpdatePolicy policy = UpdatePolicy::Async());
    TCfaPhase GetCfaPhase();

    bool SetClipZero(bool to, UpdatePolicy policy = UpdatePolicy::Async());
    bool GetClipZero();

    bool SetBlackPedestalByIdx(uint8_t index, uint32_t to, UpdatePolicy policy = UpdatePolicy::Async());
    uint32_t GetBlackPedestalByIdx(uint8_t index);
    std::vector<uint32_t> GetBlackPedestals();

    bool SetColorScalerByIdx(uint8_t index, uint32_t to, UpdatePolicy policy = UpdatePolicy::Async());
    uint32_t GetColorScalerByIdx(uint8_t index);

    bool SetPedestalsAndScalers(uint32_t pedestals[CFA_SIZE], uint32_t scalers[CFA_SIZE], UpdatePolicy policy = UpdatePolicy::Async());

    bool SetResolution(uint32_t width, uint32_t height);

private:
    bool CommitSettings() override;
    bool IsCommitPending() override;

    Hapi::VvpBlcPtr _spBlc = nullptr;

    bool _bypass;
    TCfaPhase _cfaPhase;
    bool _clipZero;
    uint32_t _pedestals[CFA_SIZE];
    uint32_t _scalers[CFA_SIZE];
};

} // namespace SwApi
