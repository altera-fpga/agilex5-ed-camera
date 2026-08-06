/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Bit masks
#define TF32_MASK    0xffffe000 // FP32 to TF32 (bfloat16+, FP19) quantization mask

// User data fields
#define DIAMETER                  (int)user_data_ptr[0]
#define RADIUS                    (int)user_data_ptr[1]
#define K_NOISE                        user_data_ptr[2]
#define DARK_NOISE                     user_data_ptr[3]
#define BPS                       (int)user_data_ptr[4]
#define SPATIAL_DISTANCE_LUT_OFFSET    5

union float_view {
    float f;
    uint32_t u;
};

float to_tf32(float x) {
    union float_view fu;
    fu.f = x;
    fu.u &= TF32_MASK; 
    return fu.f;
}

int bfilt(
    double*  buffer,
    intptr_t filter_size,
    double*  return_value,
    void*    user_data
) {
    double* user_data_ptr = (double*)user_data;
    float sigma_intensity;
    float dark_noise_squared = DARK_NOISE * DARK_NOISE;
    float weight;
    float pixel_acc = 0;
    float weight_acc = 0;
    float weight_acc_inv;

    float* buffer_tf32 = malloc(filter_size * sizeof(float));
    if (buffer_tf32 == NULL) {
        fprintf(stderr, "Error: Malloc failed");
        return -1;
    }
    buffer_tf32[RADIUS] = to_tf32(buffer[RADIUS]);

    // Estimate the noise from pixel intensity
    sigma_intensity = to_tf32(sqrt(to_tf32(fabs(buffer_tf32[RADIUS]) * to_tf32(K_NOISE)) + to_tf32(dark_noise_squared)));
    for (int i = 0; i < filter_size; i++) {
        buffer_tf32[i] = to_tf32(buffer[i]);
        // Gaussian distance in intensity domain
        weight = to_tf32(buffer_tf32[RADIUS] - buffer_tf32[i]);
        weight = to_tf32(weight / sigma_intensity);
        weight = to_tf32(weight * weight);
        weight = to_tf32(exp(to_tf32(-0.5 * weight)));
        // Gaussian distance in spatial domain
        weight = to_tf32(weight * user_data_ptr[SPATIAL_DISTANCE_LUT_OFFSET+i]);
        // Weigting and normaliztion factor calculation
        pixel_acc += to_tf32(weight * buffer_tf32[i]);
        weight_acc += weight;
    }
    // Normalization
    weight_acc_inv = to_tf32(1 / weight_acc);
    *return_value = weight_acc_inv * pixel_acc;

    free(buffer_tf32);
    return 1;
}

// https://github.com/Maratyszcza/FP16/blob/0a92994d729ff76a58f692d3028ca1b64b145d91/include/fp16/fp16.h
// MIT License
/*
uint16_t fp16_ieee_from_fp32_value(uint32_t x) {
  uint32_t x_sgn = x & 0x80000000u;
  uint32_t x_exp = x & 0x7f800000u;
  x_exp = (x_exp < 0x38800000u) ? 0x38800000u : x_exp; // max(e, -14)
  x_exp += 15u << 23; // e += 15
  x &= 0x7fffffffu; // Discard sign

  union { uint32_t u; float f; } f, magic;
  f.u = x;
  magic.u = x_exp;

  // If 15 < e then inf, otherwise e += 2
  f.f = (f.f * 0x1.0p+112f) * 0x1.0p-110f;

  // If we were in the x_exp >= 0x38800000u case:
  // Replace f's exponent with that of x_exp, and discard 13 bits of
  // f's significand, leaving the remaining bits in the low bits of
  // f, relying on FP addition being round-to-nearest-even. If f's
  // significand was previously `a.bcdefghijk`, then it'll become
  // `1.000000000000abcdefghijk`, from which `bcdefghijk` will become
  // the FP16 mantissa, and `a` will add onto the exponent. Note that
  // rounding might add one to all this.
  // If we were in the x_exp < 0x38800000u case:
  // Replace f's exponent with the minimum FP16 exponent, discarding
  // however many bits are required to make that happen, leaving
  // whatever is left in the low bits.
  f.f += magic.f;

  uint32_t h_exp = (f.u >> 13) & 0x7c00u; // low 5 bits of exponent
  uint32_t h_sig = f.u & 0x0fffu; // low 12 bits (10 are mantissa)
  h_sig = (x > 0x7f800000u) ? 0x0200u : h_sig; // any NaN -> qNaN
  return (x_sgn >> 16) + h_exp + h_sig;
}
*/
