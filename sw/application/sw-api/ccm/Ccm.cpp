/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "Ccm.h"
#include <unistd.h>
#include <set>
#include <cmath>

namespace SwApi
{
        std::shared_ptr<Ccm> Ccm::Create(const std::shared_ptr<SwApi::ICsc>& spCsc)
        {
            return std::make_shared<Ccm>(spCsc);
        }

        Ccm::Ccm(const std::shared_ptr<SwApi::ICsc>& spCsc)
        : _spCsc(spCsc)
        {
            float bps_scale = 1.0;
            uint8_t bpsIn = _spCsc->GetBitsPerSampleIn();
            uint8_t bpsOut = _spCsc->GetBitsPerSampleOut();
            float bitDif = (float)bpsOut - (float)bpsIn;
            bps_scale = (float)powf(2.0, bitDif);

            _toyUIMatrix = vvp::ccm::GenerateIdentityMatrix();
            _aexMatrix = vvp::ccm::GenerateIdentityMatrix();
            _awbTintMatrix = vvp::ccm::GenerateIdentityMatrix();
            _awbMatrix = vvp::ccm::GenerateIdentityMatrix();
            _bpsMatrix = vvp::ccm::GenerateScaleMatrix(bps_scale);
        }


        void Ccm::ApplyToyMatrix(vvp::ccm::FloatCoefficients matrix)
        {
            _toyUIMatrix = matrix;
            ApplyStoredMatrices();
        }

        void Ccm::ApplyAEXMatrix(vvp::ccm::FloatCoefficients matrix)
        {
            _aexMatrix = matrix;
            ApplyStoredMatrices();
        }

        void Ccm::ApplyAWBTintMatrix(vvp::ccm::FloatCoefficients matrix)
        {
            _awbTintMatrix = matrix;
            ApplyStoredMatrices();
        }

        void Ccm::ApplyAWBMatrix(vvp::ccm::FloatCoefficients matrix)
        {
            _awbMatrix = matrix;
            ApplyStoredMatrices();
        }

        void Ccm::SetAWBInterpFactor(float factor)
        {
            _awbInterpFactor = factor;
            ApplyStoredMatrices();
        }

        void Ccm::ApplySingularMatrix(vvp::ccm::FloatCoefficients matrix)
        {
            intel_vvp_coefficients coeffs;

            coeffs.coeffs[0].c1 = matrix.C2;
            coeffs.coeffs[0].c2 = matrix.B2;
            coeffs.coeffs[0].c3 = matrix.A2;
            coeffs.s[0]         = matrix.S2;

            coeffs.coeffs[1].c1 = matrix.C1;
            coeffs.coeffs[1].c2 = matrix.B1;
            coeffs.coeffs[1].c3 = matrix.A1;
            coeffs.s[1]         = matrix.S1;

            coeffs.coeffs[2].c1 = matrix.C0;
            coeffs.coeffs[2].c2 = matrix.B0;
            coeffs.coeffs[2].c3 = matrix.A0;
            coeffs.s[2]         = matrix.S0;

            _spCsc->ApplyCoefficientMatrix(&coeffs, 10, _spCsc->GetInputColorSpace(), _spCsc->GetOutputColorSpace());
        }

        void Ccm::ApplyStoredMatrices()
        {
            vvp::ccm::FloatCoefficients resultMatrix = _toyUIMatrix;
            resultMatrix = resultMatrix * _aexMatrix;
            resultMatrix = resultMatrix * _awbTintMatrix;
            auto identity = vvp::ccm::GenerateIdentityMatrix();
            auto awbMatrixDiff = _awbMatrix - identity;

            vvp::ccm::ScaleChannelStrength(awbMatrixDiff, _awbInterpFactor, _awbInterpFactor, _awbInterpFactor);
            auto interpolatedAwbMatrix = vvp::ccm::GenerateIdentityMatrix() + awbMatrixDiff;
            resultMatrix = resultMatrix * interpolatedAwbMatrix;
            resultMatrix = resultMatrix * _bpsMatrix;

            ApplySingularMatrix(resultMatrix);
        }

} // namespace SwApi
