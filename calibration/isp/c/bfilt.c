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

// User data fields
#define DIAMETER                  (int)user_data_ptr[0]
#define RADIUS                    (int)user_data_ptr[1]
#define K_NOISE                        user_data_ptr[2]
#define DARK_NOISE                     user_data_ptr[3]
#define BPS                       (int)user_data_ptr[4]
#define SPATIAL_DISTANCE_LUT_OFFSET    5

int bfilt(
    double*  buffer,
    intptr_t filter_size,
    double*  return_value,
    void*    user_data
) {
    double* user_data_ptr = (double*)user_data;
    double sigma_intensity;
    double dark_noise_squared = DARK_NOISE * DARK_NOISE;
    double weight;
    double pixel_acc = 0;
    double weight_acc = 0;

    // Estimate the noise from pixel intensity
    sigma_intensity = sqrt(fabs(buffer[RADIUS]) * K_NOISE + dark_noise_squared);
    for (int i = 0; i < filter_size; i++) {
        // Gaussian distance in intensity domain
        weight = buffer[RADIUS] - buffer[i];
        weight /= sigma_intensity;
        weight *= weight;
        weight = exp(-0.5 * weight);
        // Gaussian distance in spatial domain
        weight *= user_data_ptr[SPATIAL_DISTANCE_LUT_OFFSET + i];
        // Weigting and normaliztion factor calculation
        pixel_acc += weight * buffer[i];
        weight_acc += weight;
    }
    // Normalization
    *return_value = pixel_acc / weight_acc;
    return 1;
}
