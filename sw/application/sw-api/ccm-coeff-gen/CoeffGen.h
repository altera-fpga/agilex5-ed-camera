/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <cstdint>
#include <sstream>
#include "CoeffData.h"

#pragma once

namespace vvp
{
    namespace ccm
    {
        struct FloatCoefficients
        {
            float A0;
            float B0;
            float C0;

            float A1;
            float B1;
            float C1;

            float A2;
            float B2;
            float C2;

            float S0;
            float S1;
            float S2;

            FloatCoefficients()
            {
                A0 = 0;
                B0 = 0;
                C0 = 0;
                
                A1 = 0;
                B1 = 0;
                C1 = 0;

                A2 = 0;
                B2 = 0;
                C2 = 0;

                S0 = 0;
                S1 = 0;
                S2 = 0;
            }

            std::string ToString()
            {
                std::stringstream ss;

                ss << "[\n";
                ss << " [" << A0 << ", " << A1 << ", " << A2 << ", " << S0 << "]\n";
                ss << " [" << B0 << ", " << B1 << ", " << B2 << ", " << S1 << "]\n";
                ss << " [" << C0 << ", " << C1 << ", " << C2 << ", " << S2 << "]\n";
                ss << "]";

                return ss.str();
            }

            FloatCoefficients flat_mul(const FloatCoefficients& other)
            {
                FloatCoefficients newMatrix;

                newMatrix.A0 = (A0 * other.A0);
                newMatrix.A1 = (A1 * other.A1);
                newMatrix.A2 = (A2 * other.A2);

                newMatrix.B0 = (B0 * other.B0);
                newMatrix.B1 = (B1 * other.B1);
                newMatrix.B2 = (B2 * other.B2);

                newMatrix.C0 = (C0 * other.C0);
                newMatrix.C1 = (C1 * other.C1);
                newMatrix.C2 = (C2 * other.C2);

                newMatrix.S0 = S0 * other.S0;
                newMatrix.S1 = S1 * other.S1;
                newMatrix.S2 = S2 * other.S2;

                return newMatrix;
            }

            FloatCoefficients operator*(const FloatCoefficients& other)
            {
                FloatCoefficients newMatrix;

                newMatrix.A0 = (A0 * other.A0) + (A1 * other.B0) + (A2 * other.C0);
                newMatrix.A1 = (A0 * other.A1) + (A1 * other.B1) + (A2 * other.C1);
                newMatrix.A2 = (A0 * other.A2) + (A1 * other.B2) + (A2 * other.C2);

                newMatrix.B0 = (B0 * other.A0) + (B1 * other.B0) + (B2 * other.C0);
                newMatrix.B1 = (B0 * other.A1) + (B1 * other.B1) + (B2 * other.C1);
                newMatrix.B2 = (B0 * other.A2) + (B1 * other.B2) + (B2 * other.C2);

                newMatrix.C0 = (C0 * other.A0) + (C1 * other.B0) + (C2 * other.C0);
                newMatrix.C1 = (C0 * other.A1) + (C1 * other.B1) + (C2 * other.C1);
                newMatrix.C2 = (C0 * other.A2) + (C1 * other.B2) + (C2 * other.C2);

                newMatrix.S0 = S0 * other.S0;
                newMatrix.S1 = S1 * other.S1;
                newMatrix.S2 = S2 * other.S2;

                return newMatrix;
            }

            FloatCoefficients operator+(const FloatCoefficients& other)
            {
                FloatCoefficients newMatrix;

                newMatrix.A0 = (A0 + other.A0);
                newMatrix.A1 = (A1 + other.A1);
                newMatrix.A2 = (A2 + other.A2);

                newMatrix.B0 = (B0 + other.B0);
                newMatrix.B1 = (B1 + other.B1);
                newMatrix.B2 = (B2 + other.B2);

                newMatrix.C0 = (C0 + other.C0);
                newMatrix.C1 = (C1 + other.C1);
                newMatrix.C2 = (C2 + other.C2);

                newMatrix.S0 = S0 + other.S0;
                newMatrix.S1 = S1 + other.S1;
                newMatrix.S2 = S2 + other.S2;

                return newMatrix;
            }

            FloatCoefficients operator-(const FloatCoefficients& other)
            {
                FloatCoefficients newMatrix;

                newMatrix.A0 = (A0 - other.A0);
                newMatrix.A1 = (A1 - other.A1);
                newMatrix.A2 = (A2 - other.A2);

                newMatrix.B0 = (B0 - other.B0);
                newMatrix.B1 = (B1 - other.B1);
                newMatrix.B2 = (B2 - other.B2);

                newMatrix.C0 = (C0 - other.C0);
                newMatrix.C1 = (C1 - other.C1);
                newMatrix.C2 = (C2 - other.C2);

                newMatrix.S0 = S0 - other.S0;
                newMatrix.S1 = S1 - other.S1;
                newMatrix.S2 = S2 - other.S2;

                return newMatrix;
            }
        };

        struct FixedPointCoefficients
        {
            uint32_t A0;
            uint32_t B0;
            uint32_t C0;

            uint32_t A1;
            uint32_t B1;
            uint32_t C1;

            uint32_t A2;
            uint32_t B2;
            uint32_t C2;

            uint32_t S0;
            uint32_t S1;
            uint32_t S2; 
        };

        /// Matrix Generators

        /*
            GenerateIdentityMatrix

            Returns a matrix that contains coefficients that will result in the exact input RGB
            being output without modification.
        */
        FloatCoefficients GenerateIdentityMatrix();


        /*
            GenerateScaleMatrix

            Inputs:

            float scale - Scaling factor
            
            Returns a matrix that contains coefficients that will result in the input RGB
            being output scaled by "scale".
        */
        FloatCoefficients GenerateScaleMatrix(float scale);


        /*
            GenerateIdentityMatrix

            Inputs:

            float hue - Accepted range: 0-360.0
            float sat - 1.0 results in no change. Go towards 0 to remove colours from the image, increase above 1 to boost saturation.
            float val - 1.0 results in no change. Go towards 0 for black, increasing this will increase the image brightness.

            Returns a matrix that contains coefficients that will apply a hue shift in degrees,
            increase/decrease the saturation and modify the brightness according to val.
        */
        FloatCoefficients GenerateHSVMatrix(float hue, float sat, float val);

        /// Matrix Modifiers

        /*
            GetColorCoefficientsForTemperature

            Input:

            float temp

            Returns the ColorCoefficients (see CoeffData.h) which provides an sRGB gamut value corresponding to the given color temperature.
            Also includes values for tint adjustment.
        */
        ColorCoefficients GetColorCoefficientsForTemperature(float temp);

        /*
            ModifyMatrixByTint

            Input:

            FloatCoefficients& matrix - The matrix to be modified
            float target_temp - Given a temperature in kelvin, will modify the image by the values pre-calculated in CoeffData.h
            float tint_adjustment - Valid between -1 and 1, modifies the tint of the image

            It is essentially a wrapper for ScaleChannelStrength, but fetches the values from GetColorCoefficientsForTemperature in order to perform the modification.
            Only modifies tint, does not scale temperature
        */
        void ModifyMatrixByTint(FloatCoefficients &matrix, float target_temp, float tint_adjustment);


        /*
            ModifyMatrixByColorTemperature

            Input:

            FloatCoefficients& matrix - The matrix to be modified
            float target_temp - Given a temperature in kelvin, will modify the image by the values pre-calculated in CoeffData.h
            float tint_adjustment - Valid between -1 and 1, modifies the tint of the image

            It is essentially a wrapper for ScaleChannelStrength, but fetches the values from GetColorCoefficientsForTemperature in order to perform the modification.
        */
        void ModifyMatrixByColorTemperature(FloatCoefficients &matrix, float target_temp, float tint_adjustment);

        // Per channel strength/blending
        /*
            ScaleChannelStrength

            Given a FloatCoefficients matrix, will multiply the RGB coefficients by the corresponding X_strength value. 
        */
        void ScaleChannelStrength(FloatCoefficients &matrix, float red_strength, float green_strength, float blue_strength);

        /*
            BlendMatricesByChannel

            Given two FloatCoefficient matrices and a set of channel strengths, this function returns a matrix that is a blend between them. 
            0.0 corresponds to matrix1, 1.0 corresponds to matrix2
        */
        FloatCoefficients BlendMatricesByChannel(FloatCoefficients &matrix1, FloatCoefficients &matrix2, float red_strength, float green_strength, float blue_strength);

        /*
            BlendToIdentity

            Given a FloatCoefficients matrix, will blend each channel with an identity matrix based on the corresponding channel blend strength.
        */
        void BlendWithIdentity(FloatCoefficients &matrix, float red_strength, float green_strength, float blue_strength);

        // Contrast modifiers
        /*
            RaiseChannelFloor

            Given a FloatCoefficients matrix: Will reduce the channel coefficients by the corresponding factor, but subsequently increment
            the summand by the given factor. Removes contrast while tending towards white.

            In reverse it can boost the contrast, but tends towards black instead of preserving contrast

            Combine with IncreaseChannelContrast to fine-tune your resulting contrast levels.
        */
        void RaiseChannelFloor(FloatCoefficients& matrix, float red_factor, float green_factor, float blue_factor);

        /*
            IncreaseChannelContrast

            Given a FloatCoefficients matrix: Will multiply the channel coefficients by the corresponding factor, while decrementing the
            channel summand by the mean of the incoming coefficients. Provides a strong visual contrast increase, while ensuring brightness is maintained.
            Can cause channel clipping at high values.

            In reverse, it removes contrast while tending towards grey.

            Combine with RaiseChannelFloor to fine-tune your resulting contrast levels.
        */
        void IncreaseChannelContrast(FloatCoefficients &matrix, float red_factor, float green_factor, float blue_factor);


        /// Outputting to hardware

        /*
            ConvertToFixedPoint

            Given a FloatCoefficients matrix, outputs a FixedPointCoefficients matrix. The fixed point ranges are given by coeff_frac_size
            and summand_frac_size, where they essentially highlight "where" the decimal point is from the LSB upwards.
        */
        FixedPointCoefficients ConvertToFixedPoint(FloatCoefficients &matrix, uint8_t coeff_frac_size, uint8_t summand_frac_size);
    }
}