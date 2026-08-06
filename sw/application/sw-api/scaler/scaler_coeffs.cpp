/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <cmath>
#include "scaler_coeffs.h"


#define PI 3.14159265


static int get_required_lobes(int in_size, int out_size, int taps)
{
    float float_lobes = (out_size < in_size) ? ((float)(taps *out_size)) / ((float)(2*in_size)) : (float)(taps/2);
    int lobes = (int)float_lobes;
    lobes = (lobes < 1) ? 1 : (lobes > 2) ? 2 : lobes;
    return lobes;
}


static int get_active_taps(int in_size, int out_size, int lobes, int taps)
{
    float ideal_taps = (in_size < out_size) ? (float)(2 * lobes) : ((float)(2 * lobes * in_size)) / ((float)out_size);
    int int_taps = (int)ideal_taps;
    if (((float)int_taps) < ideal_taps) {
        int_taps++;
    }
    if (int_taps > taps) {
        return taps;
    } else {
        return int_taps;
    }
}    


static float lanczos(float x, float lobes)
{
    // sinc(0) is explicity defined to be 1
    if (x == 0.0) {
        return 1.0;
    }
    // Outside the lobes is defined to be 0
    else if (!(x <= lobes && x >= -lobes)) {
        return 0.0;
    } else {
        float pi_x = PI * x;
        return (sin(pi_x)/pi_x) * (sin(pi_x/ lobes)/(pi_x/ lobes));
    }
} 


static void generate_lanczos_coefficients(float *coeffs, int phase, int taps, int phases, int lobes, int frac_bits, int active_taps)
{
    float lanczos_offset_per_tap = 0.0;
    float float_lobes = (float)lobes;
    float float_phases = (float)phases;

    //if not all taps are used....
    int taps_offset = ((taps-1)/2) - ((active_taps-1)/ 2);
    if (taps_offset > 0) {
        for (int j=0; j<taps_offset; j++) {
            coeffs[j] = 0.0;
        }
    }
    if ((taps_offset+active_taps) < taps) {
        for (int j=(taps_offset+active_taps); j<taps; j++) {
            coeffs[j] = 0.0;
        }
    }

    //if there is an odd number of active_taps we center the kernel at floor(active_taps/2)
    //in phase 0 all active_taps have no zero coefficients - divide lanczos kernel into active_taps + 1 samples
    //ignore the first and last sample as they will be zero (taken at +/- lobes exactly)
    //if there are even active_taps we again center at floor(active_taps/2)
    //in phase 0 the final tap will have coefficient of 0 - divide lanczos kernel into active_taps samples
    //only ignore the first sample (at -lobes) and use the last sample (at +lobes) to generate the 0 for phase 0
    if ((active_taps & 0x1) > 0) {
        lanczos_offset_per_tap = ((float)(2*lobes))/((float)(active_taps+1));
    } else {
        lanczos_offset_per_tap = ((float)(2*lobes))/((float)active_taps);
    }
        
    // We are going to add this value to move the function left.
    // Generally f(x+1) is to the right of f(x) so phaseOffset counts down
    float phase_offset = (float)phase * (lanczos_offset_per_tap / float_phases);
    float total = 0.0;
    for (int j = 0; j < active_taps; j++) {
        // Move by one tap each time, scaling these taps appropriately
        // to the natural size of the lanczos function (-lobes to lobes)
        float tap_offset = ((float)(j+1)) * lanczos_offset_per_tap;
        coeffs[j+taps_offset] = lanczos(-float_lobes + tap_offset - phase_offset, float_lobes);
        
        //keeps a running total for normalisation
        total += coeffs[j+taps_offset];
    }

    if(total != 0.0) {
        //normailise the coefficients so they sum to 1, and quantize to frac bits
        for (int j = 0; j < taps; j++) {
            coeffs[j+taps_offset] /= total;
        }
    }
}    


void scaler_create_and_load_horizontal_coefficents(intel_vvp_scaler_instance* scaler, int in_width, int out_width, int bank)
{
    int lobes = get_required_lobes(in_width, out_width, scaler->h_num_taps);
    int active_taps = get_active_taps(in_width, out_width, lobes, scaler->h_num_taps);
    float *coeffs = (float *) malloc(scaler->h_num_taps * scaler->h_num_phases * sizeof(float));

    for (int i=0; i<scaler->h_num_phases; i++) {
        generate_lanczos_coefficients(coeffs, i, scaler->h_num_taps, scaler->h_num_phases, lobes, scaler->h_coeffs_frac_bits, active_taps);
        intel_vvp_scaler_set_coeffs(scaler, true, coeffs, scaler->h_num_taps, bank, i);
    }

    free(coeffs);
}


void scaler_create_and_load_vertical_coefficents(intel_vvp_scaler_instance* scaler, int in_height, int out_height, int bank)
{
    int lobes = get_required_lobes(in_height, out_height, scaler->v_num_taps);
    int active_taps = get_active_taps(in_height, out_height, lobes, scaler->v_num_taps);
    float *coeffs = (float *) malloc(scaler->v_num_taps * scaler->v_num_phases * sizeof(float));

    for (int i=0; i<scaler->v_num_phases; i++) {
        generate_lanczos_coefficients(coeffs, i, scaler->v_num_taps, scaler->v_num_phases, lobes, scaler->v_coeffs_frac_bits, active_taps);
        intel_vvp_scaler_set_coeffs(scaler, false, coeffs, scaler->v_num_taps, bank, i);
    }

    free(coeffs);
}
