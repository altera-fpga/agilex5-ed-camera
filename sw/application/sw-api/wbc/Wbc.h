/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpWbc.h"
#include "VvpCoreBase.h"
#include "IspCommon.h"

#include <cstdint>

namespace SwApi 
{
    class Wbc : public VvpCoreBase
    {
        public:
            static std::shared_ptr<Wbc> Create(Hapi::VvpWbcPtr spWbc, uint32_t initialOutputWidth, uint32_t initialOutputHeight);

            static constexpr uint32_t CFA_SIZE = 4;
            
            Wbc(Hapi::VvpWbcPtr spWbc,
                            uint32_t initialOutputWidth, uint32_t initialOutputHeight);

            /// Enables/disables the core depending on the value of `bypass`, returning
            /// whether the operation succeeded.
            bool SetBypass(bool bypass, UpdatePolicy policy = UpdatePolicy::Async());
            bool GetBypass();

            bool IsRunning();

            bool SetCfaPhase(TCfaPhase to, UpdatePolicy policy = UpdatePolicy::Async());
            TCfaPhase GetCfaPhase();

            bool SetColorScalerByIdx(uint8_t index, uint32_t to, UpdatePolicy policy = UpdatePolicy::Async());
            uint32_t GetColorScalerByIdx(uint8_t index);

            bool SetAllColorScalers(const uint32_t scalers[CFA_SIZE], UpdatePolicy policy = UpdatePolicy::Async());

            bool SetResolution(uint32_t width, uint32_t height);
            std::pair<uint32_t, uint32_t> GetResolution() const;

        private:
            bool CommitSettings() override;
            bool IsCommitPending() override;

            Hapi::VvpWbcPtr _spWbc = nullptr;

            uint32_t _width{0};
            uint32_t _height{0};

            TCfaPhase _cfaPhase;
            uint32_t _scalers[CFA_SIZE];
    };

} // namespace SwApi
