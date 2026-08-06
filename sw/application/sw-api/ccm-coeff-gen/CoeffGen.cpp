/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "CoeffGen.h"
#include <cmath>

using namespace vvp::ccm;


FloatCoefficients vvp::ccm::GenerateIdentityMatrix()
{
    FloatCoefficients coeff;
    coeff.A0 = 1.0;
    coeff.B1 = 1.0;
    coeff.C2 = 1.0;
    return coeff;
}

FloatCoefficients vvp::ccm::GenerateScaleMatrix(float scale)
{
    FloatCoefficients coeff;
    coeff.A0 = scale;
    coeff.B1 = scale;
    coeff.C2 = scale;
    return coeff;
}

FloatCoefficients vvp::ccm::GenerateHSVMatrix(float hue, float sat, float val)
{
    // The values used in this matrix are explained here: https://beesbuzz.biz/code/16-hsv-color-transforms

    FloatCoefficients coeff;

    float vsu = val * sat * cos(hue * 3.141593 / 180.0);
    float vsw = val * sat * sin(hue * 3.141593 / 180.0);

    coeff.A0 = ((0.299 * val) + (0.701 * vsu) + (0.168 * vsw));
    coeff.B0 = ((0.587 * val) - (0.587 * vsu) + (0.330 * vsw));
    coeff.C0 = ((0.114 * val) - (0.114 * vsu) - (0.497 * vsw));

    coeff.A1 = ((0.299 * val) - (0.299 * vsu) - (0.328 * vsw));
    coeff.B1 = ((0.587 * val) + (0.413 * vsu) + (0.035 * vsw));
    coeff.C1 = ((0.114 * val) - (0.114 * vsu) + (0.292 * vsw));

    coeff.A2 = ((0.299 * val) - (0.300 * vsu) + (1.250 * vsw));
    coeff.B2 = ((0.587 * val) - (0.588 * vsu) - (1.050 * vsw));
    coeff.C2 = ((0.114 * val) + (0.886 * vsu) - (0.203 * vsw));

    return coeff;
}

// This function will handle any interpolation for the rgb/maxtint/mintint
ColorCoefficients vvp::ccm::GetColorCoefficientsForTemperature(float temp)
{
    float floating_index = std::max(0.0f, ((temp / 100) - 10)); // This formula is derived by: (temp / increment in the LUT) - offset from 0
    uint32_t int_index = int(floating_index);

    if (floating_index > int_index)
    {
        // In this case, we need to interpolate
        float interp_diff = floating_index - int_index;

        ColorCoefficients new_coeff;

        new_coeff.temperature = temp;
        // We don't care about the x and y here
        // new_coeff.x = (color_lut[int_index].x * (1.0 - interp_diff)) + (color_lut[int_index + 1].x * interp_diff);
        // new_coeff.y = (color_lut[int_index].y * (1.0 - interp_diff)) + (color_lut[int_index + 1].y * interp_diff);

        // FIXME_KYLE - Replace with the (v2-v1)*interp_diff, it's more efficient

        new_coeff.rgb[0] = (color_lut[int_index].rgb[0] * (1.0 - interp_diff)) + (color_lut[int_index + 1].rgb[0] * interp_diff);
        new_coeff.rgb[1] = (color_lut[int_index].rgb[1] * (1.0 - interp_diff)) + (color_lut[int_index + 1].rgb[1] * interp_diff);
        new_coeff.rgb[2] = (color_lut[int_index].rgb[2] * (1.0 - interp_diff)) + (color_lut[int_index + 1].rgb[2] * interp_diff);

        new_coeff.max_tint_rgb[0] = (color_lut[int_index].max_tint_rgb[0] * (1.0 - interp_diff)) + (color_lut[int_index + 1].max_tint_rgb[0] * interp_diff);
        new_coeff.max_tint_rgb[1] = (color_lut[int_index].max_tint_rgb[1] * (1.0 - interp_diff)) + (color_lut[int_index + 1].max_tint_rgb[1] * interp_diff);
        new_coeff.max_tint_rgb[2] = (color_lut[int_index].max_tint_rgb[2] * (1.0 - interp_diff)) + (color_lut[int_index + 1].max_tint_rgb[2] * interp_diff);

        new_coeff.min_tint_rgb[0] = (color_lut[int_index].min_tint_rgb[0] * (1.0 - interp_diff)) + (color_lut[int_index + 1].min_tint_rgb[0] * interp_diff);
        new_coeff.min_tint_rgb[1] = (color_lut[int_index].min_tint_rgb[1] * (1.0 - interp_diff)) + (color_lut[int_index + 1].min_tint_rgb[1] * interp_diff);
        new_coeff.min_tint_rgb[2] = (color_lut[int_index].min_tint_rgb[2] * (1.0 - interp_diff)) + (color_lut[int_index + 1].min_tint_rgb[2] * interp_diff);

        new_coeff.colour_chan_percent[0] = (color_lut[int_index].colour_chan_percent[0] * (1.0 - interp_diff)) + (color_lut[int_index + 1].colour_chan_percent[0] * interp_diff);
        new_coeff.colour_chan_percent[1] = (color_lut[int_index].colour_chan_percent[1] * (1.0 - interp_diff)) + (color_lut[int_index + 1].colour_chan_percent[1] * interp_diff);
        new_coeff.colour_chan_percent[2] = (color_lut[int_index].colour_chan_percent[2] * (1.0 - interp_diff)) + (color_lut[int_index + 1].colour_chan_percent[2] * interp_diff);
        return new_coeff;
    }
    else
    {
        // In this case, we can simply return the entry in color_lut
        return vvp::ccm::color_lut[int_index];
    }
}

void vvp::ccm::ModifyMatrixByTint(FloatCoefficients &matrix, float target_temp, float tint_adjustment)
{
    const float EPSILON = 1e-6;

    ColorCoefficients color_coeff = GetColorCoefficientsForTemperature(target_temp);

    float r_strength, g_strength, b_strength;
    float r_diff = 0.0f, g_diff = 0.0f, b_diff = 0.0f;

    r_strength = 1.0;
    g_strength = 1.0;
    b_strength = 1.0;

    if (tint_adjustment > 0)
    {
        r_diff = color_coeff.max_tint_rgb[0] / color_coeff.rgb[0];
        g_diff = color_coeff.max_tint_rgb[1] / color_coeff.rgb[1];
        b_diff = color_coeff.max_tint_rgb[2] / color_coeff.rgb[2];
    }
    else if (tint_adjustment < 0)
    {
        r_diff = color_coeff.min_tint_rgb[0] / color_coeff.rgb[0];
        g_diff = color_coeff.min_tint_rgb[1] / color_coeff.rgb[1];
        b_diff = color_coeff.min_tint_rgb[2] / color_coeff.rgb[2];

        tint_adjustment = -tint_adjustment;
    }

    r_strength += ((r_diff - 1.0f) * tint_adjustment);
    g_strength += ((g_diff - 1.0f) * tint_adjustment);
    b_strength += ((b_diff - 1.0f) * tint_adjustment);

    float min = std::min(r_strength, std::min(g_strength, b_strength));
    min = std::max(std::abs(min), EPSILON); // make sure no singularity
    r_strength /= min;
    g_strength /= min;
    b_strength /= min;
    ScaleChannelStrength(matrix, r_strength, g_strength, b_strength);
}

void vvp::ccm::ModifyMatrixByColorTemperature(FloatCoefficients &matrix, float target_temp, float tint_adjustment)
{
    if (target_temp < color_lut[0].temperature)
    {
        target_temp = color_lut[0].temperature;
    }
    if (target_temp > color_lut[color_lut_length - 1].temperature)
    {
        target_temp = color_lut[color_lut_length - 1].temperature;
    }

    ColorCoefficients color_coeff = GetColorCoefficientsForTemperature(target_temp);

    float r_strength, g_strength, b_strength;
    float r_diff = 0.0f, g_diff = 0.0f, b_diff = 0.0f;

    r_strength = color_coeff.rgb[0];
    g_strength = color_coeff.rgb[1];
    b_strength = color_coeff.rgb[2];

    if (tint_adjustment > 0)
    {
        r_diff = color_coeff.max_tint_rgb[0] / color_coeff.rgb[0];
        g_diff = color_coeff.max_tint_rgb[1] / color_coeff.rgb[1];
        b_diff = color_coeff.max_tint_rgb[2] / color_coeff.rgb[2];
    }
    else if (tint_adjustment < 0)
    {
        r_diff = color_coeff.min_tint_rgb[0] / color_coeff.rgb[0];
        g_diff = color_coeff.min_tint_rgb[1] / color_coeff.rgb[1];
        b_diff = color_coeff.min_tint_rgb[2] / color_coeff.rgb[2];

        tint_adjustment = -tint_adjustment;
    }

    if (tint_adjustment != 0)
    {
        r_strength += ((r_diff - r_strength) * tint_adjustment);
        g_strength += ((g_diff - g_strength) * tint_adjustment);
        b_strength += ((b_diff - b_strength) * tint_adjustment);
    }

    float min = std::min(r_strength, std::min(g_strength, b_strength));
    r_strength /= min;
    g_strength /= min;
    b_strength /= min;
    ScaleChannelStrength(matrix, r_strength, g_strength, b_strength);
}

void vvp::ccm::ScaleChannelStrength(FloatCoefficients &matrix, float red_strength, float green_strength, float blue_strength)
{
    matrix.A0 *= red_strength;
    matrix.B0 *= red_strength;
    matrix.C0 *= red_strength;
    matrix.S0 *= red_strength;

    matrix.A1 *= green_strength;
    matrix.B1 *= green_strength;
    matrix.C1 *= green_strength;
    matrix.S1 *= green_strength;

    matrix.A2 *= blue_strength;
    matrix.B2 *= blue_strength;
    matrix.C2 *= blue_strength;
    matrix.S2 *= blue_strength;
}

FloatCoefficients vvp::ccm::BlendMatricesByChannel(FloatCoefficients &matrix1, FloatCoefficients &matrix2, float red_strength, float green_strength, float blue_strength)
{
    FloatCoefficients new_coeff;

    if (red_strength == 0.0)
    {
        new_coeff.A0 = matrix1.A0;
        new_coeff.B0 = matrix1.B0;
        new_coeff.C0 = matrix1.C0;
        new_coeff.S0 = matrix1.S0;
    }
    else if (red_strength == 1.0)
    {
        new_coeff.A0 = matrix2.A0;
        new_coeff.B0 = matrix2.B0;
        new_coeff.C0 = matrix2.C0;
        new_coeff.S0 = matrix2.S0;
    }
    else
    {
        float red_strength_inv_scalar = 1.0 - red_strength;
        new_coeff.A0 = (red_strength_inv_scalar * matrix1.A0) + (red_strength * matrix2.A0);
        new_coeff.B0 = (red_strength_inv_scalar * matrix1.B0) + (red_strength * matrix2.B0);
        new_coeff.C0 = (red_strength_inv_scalar * matrix1.C0) + (red_strength * matrix2.C0);
        new_coeff.S0 = (red_strength_inv_scalar * matrix1.S0) + (red_strength * matrix2.S0);
    }

    if (green_strength == 0.0)
    {
        new_coeff.A1 = matrix1.A1;
        new_coeff.B1 = matrix1.B1;
        new_coeff.C1 = matrix1.C1;
        new_coeff.S1 = matrix1.S1;
    }
    else if (green_strength == 1.0)
    {
        new_coeff.A1 = matrix2.A1;
        new_coeff.B1 = matrix2.B1;
        new_coeff.C1 = matrix2.C1;
        new_coeff.S1 = matrix2.S1;
    }
    else
    {
        float green_strength_inv_scalar = 1.0 - green_strength;
        new_coeff.A1 = (green_strength_inv_scalar * matrix1.A1) + (green_strength * matrix2.A1);
        new_coeff.B1 = (green_strength_inv_scalar * matrix1.B1) + (green_strength * matrix2.B1);
        new_coeff.C1 = (green_strength_inv_scalar * matrix1.C1) + (green_strength * matrix2.C1);
        new_coeff.S1 = (green_strength_inv_scalar * matrix1.S1) + (green_strength * matrix2.S1);
    }

    if (blue_strength == 0.0)
    {
        new_coeff.A2 = matrix1.A2;
        new_coeff.B2 = matrix1.B2;
        new_coeff.C2 = matrix1.C2;
        new_coeff.S2 = matrix1.S2;
    }
    else if (blue_strength == 1.0)
    {
        new_coeff.A2 = matrix2.A2;
        new_coeff.B2 = matrix2.B2;
        new_coeff.C2 = matrix2.C2;
        new_coeff.S2 = matrix2.S2;
    }
    else
    {
        float blue_strength_inv_scalar = 1.0 - blue_strength;
        new_coeff.A2 = (blue_strength_inv_scalar * matrix1.A2) + (blue_strength * matrix2.A2);
        new_coeff.B2 = (blue_strength_inv_scalar * matrix1.B2) + (blue_strength * matrix2.B2);
        new_coeff.C2 = (blue_strength_inv_scalar * matrix1.C2) + (blue_strength * matrix2.C2);
        new_coeff.S2 = (blue_strength_inv_scalar * matrix1.S2) + (blue_strength * matrix2.S2);
    }

    return new_coeff;
}

void vvp::ccm::BlendWithIdentity(FloatCoefficients &matrix, float red_strength, float green_strength, float blue_strength)
{
    FloatCoefficients identity = GenerateIdentityMatrix();

    FloatCoefficients new_coeff = BlendMatricesByChannel(matrix, identity, red_strength, green_strength, blue_strength);

    matrix = new_coeff;
}

void vvp::ccm::RaiseChannelFloor(FloatCoefficients& matrix, float red_factor, float green_factor, float blue_factor)
{
    matrix.A0 = matrix.A0 * (1.0 - red_factor);
    matrix.B0 = matrix.B0 * (1.0 - red_factor);
    matrix.C0 = matrix.C0 * (1.0 - red_factor);

    matrix.S0 = matrix.S0 + red_factor;

    matrix.A1 = matrix.A1 * (1.0 - green_factor);
    matrix.B1 = matrix.B1 * (1.0 - green_factor);
    matrix.C1 = matrix.C1 * (1.0 - green_factor);

    matrix.S1 = matrix.S1 + green_factor;

    matrix.A2 = matrix.A2 * (1.0 - blue_factor);
    matrix.B2 = matrix.B2 * (1.0 - blue_factor);
    matrix.C2 = matrix.C2 * (1.0 - blue_factor);

    matrix.S2 = matrix.S2 + blue_factor;
}

void vvp::ccm::IncreaseChannelContrast(FloatCoefficients &matrix, float red_factor, float green_factor, float blue_factor)
{
    matrix.S0 = matrix.S0 - ((((matrix.A0 + matrix.B0 + matrix.C0)) * red_factor) / 3);

    matrix.A0 = matrix.A0 * (1.0 + red_factor);
    matrix.B0 = matrix.B0 * (1.0 + red_factor);
    matrix.C0 = matrix.C0 * (1.0 + red_factor);

    matrix.S1 = matrix.S1 - ((((matrix.A1 + matrix.B1 + matrix.C1)) * green_factor) / 3);

    matrix.A1 = matrix.A1 * (1.0 + green_factor);
    matrix.B1 = matrix.B1 * (1.0 + green_factor);
    matrix.C1 = matrix.C1 * (1.0 + green_factor);

    matrix.S2 = matrix.S2 - ((((matrix.A2 + matrix.B2 + matrix.C2)) * blue_factor) / 3);

    matrix.A2 = matrix.A2 * (1.0 + blue_factor);
    matrix.B2 = matrix.B2 * (1.0 + blue_factor);
    matrix.C2 = matrix.C2 * (1.0 + blue_factor);
}

FixedPointCoefficients vvp::ccm::ConvertToFixedPoint(FloatCoefficients &matrix, uint8_t coeff_frac_size, uint8_t summand_frac_size)
{
    FixedPointCoefficients fixed_coeff;

    fixed_coeff.A0 = (uint32_t)(matrix.A0 * (1 << coeff_frac_size));
    fixed_coeff.B0 = (uint32_t)(matrix.B0 * (1 << coeff_frac_size));
    fixed_coeff.C0 = (uint32_t)(matrix.C0 * (1 << coeff_frac_size));

    fixed_coeff.A1 = (uint32_t)(matrix.A1 * (1 << coeff_frac_size));
    fixed_coeff.B1 = (uint32_t)(matrix.B1 * (1 << coeff_frac_size));
    fixed_coeff.C1 = (uint32_t)(matrix.C1 * (1 << coeff_frac_size));

    fixed_coeff.A2 = (uint32_t)(matrix.A2 * (1 << coeff_frac_size));
    fixed_coeff.B2 = (uint32_t)(matrix.B2 * (1 << coeff_frac_size));
    fixed_coeff.C2 = (uint32_t)(matrix.C2 * (1 << coeff_frac_size));

    fixed_coeff.S0 = (uint32_t)(matrix.S0 * (1 << summand_frac_size));
    fixed_coeff.S1 = (uint32_t)(matrix.S1 * (1 << summand_frac_size));
    fixed_coeff.S2 = (uint32_t)(matrix.S2 * (1 << summand_frac_size));

    return fixed_coeff;
}