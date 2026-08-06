/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "ICsc.h"
#include "IspCommon.h"
#include "CoeffGen.h"
#include <cstdint>


namespace SwApi 
{
    class Ccm
    {
        
        public:
            static std::shared_ptr<Ccm> Create(const std::shared_ptr<SwApi::ICsc>& spCsc);
            Ccm(const std::shared_ptr<SwApi::ICsc>&  spCsc);

            void ApplyToyMatrix(vvp::ccm::FloatCoefficients matrix);
            void ApplyAEXMatrix(vvp::ccm::FloatCoefficients matrix);
            void ApplyAWBTintMatrix(vvp::ccm::FloatCoefficients matrix);
            void ApplyAWBMatrix(vvp::ccm::FloatCoefficients matrix);
            void SetAWBInterpFactor(float factor);

            void ApplySingularMatrix(vvp::ccm::FloatCoefficients matrix);

            void ApplyStoredMatrices();

        private:
            std::shared_ptr<SwApi::ICsc> _spCsc;

            vvp::ccm::FloatCoefficients _toyUIMatrix;
            vvp::ccm::FloatCoefficients _aexMatrix;
            vvp::ccm::FloatCoefficients _awbTintMatrix;
            vvp::ccm::FloatCoefficients _awbMatrix;
            vvp::ccm::FloatCoefficients _bpsMatrix;
            float _awbInterpFactor = 1.0f;

            

    };
} // namespace SwApi
