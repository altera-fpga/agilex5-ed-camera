/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "intel_vvp_core.h"

#ifndef __INTEL_VVP_3D_LUT_H__
#define __INTEL_VVP_3D_LUT_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_3D_LUT_PRODUCT_ID                           0x0165u              ///< 3D LUT product ID
#define INTEL_VVP_3D_LUT_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_3D_LUT_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_3D_LUT_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< 3D LUT register read function
#define INTEL_VVP_3D_LUT_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< 3D LUT register write function

typedef enum {
    kIntelVvp3dLutOk                    = 0,
    kIntelVvp3dLutInstanceErr           = -100, 
    kIntelVvp3dLutErrBuffer             = -102,
    kIntelVvp3dLutErrIndexRange         = -103,
    kIntelVvp3dLutErrValueRange         = -104,
    kIntelVvp3dLutErrAlpha              = -105,
    kIntelVvp3dLutErrRamIndexRange      = -106
} eIntelVvp3dLutError;

typedef struct intel_vvp_3d_lut_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
        
    uint8_t external_mode;
    uint8_t lut_alpha;
    uint8_t lut_depth;
    uint8_t lut_dimension;
    uint8_t lut_double_buffered;
    
    uint8_t input_depth;
    uint8_t output_depth;
    uint8_t pixels_per_clock;
    uint8_t cpu_readable;

    uint8_t data_width;
    uint64_t data_val_mask;
    uint32_t ram_address_mask;
} intel_vvp_3d_lut_instance;           

/**
 * \brief Initialise a 3D LUT instance
 * 
 * Initialization function for a VVP 3D LUT instance.
 * Attempts to initialize the fields of the 3D LUT and its base core
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk          success
 *               kIntelVvpCoreVidErr      if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr      if the product id of the core is not the 3D LUT product id (0x0165)
 *               kIntelVvpCoreInstanceErr if the instance parameter is zero (null pointer)
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_3d_lut_init(intel_vvp_3d_lut_instance* instance, intel_vvp_core_base base);
    
/**
 * \brief Query the double buffer attribute of the 3D LUT
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       false double buffer is not available
 *               true  double buffer is configured
 */
bool intel_vvp_3d_lut_get_double_buffered(intel_vvp_3d_lut_instance* instance);

/**
 * \brief Get 3D LUT depth parameter
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       Number of bits per 3D LUT colour planes in the LUT entries
 *               Range is 8 - 16
 */
uint8_t intel_vvp_3d_lut_get_lut_depth(intel_vvp_3d_lut_instance* instance);

/**
 * \brief Get 3D LUT dimension parameter
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       Number of values in each dimension of the LUT
 *               Total number of entries is (dimension * dimension * dimension)
 *               Range is 17, 33 or 65
 */
uint8_t intel_vvp_3d_lut_get_dimension(intel_vvp_3d_lut_instance* instance);

/**
 * \brief Get 3D LUT input depth parameter
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       Number of bits per 3D LUT colour planes in the LUT input
 */
uint8_t intel_vvp_3d_lut_get_input_depth(intel_vvp_3d_lut_instance* instance);

/**
 * \brief Get 3D LUT output depth parameter
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       Number of bits per 3D LUT colour planes in the LUT output
 */
uint8_t intel_vvp_3d_lut_get_output_depth(intel_vvp_3d_lut_instance* instance);

/**
 * \brief Get 3D LUT pixels per clock
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       Number of pixels processed per video clock cycle
 */
uint8_t intel_vvp_3d_lut_get_pixels_per_clock(intel_vvp_3d_lut_instance* instance);
  
/**
 * \brief Get 3D LUT alpha channel supported
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \return       false - Alpha channel NOT supported
 *               true  - Alpha channel supported 
 */
bool intel_vvp_3d_lut_get_alpha_channel(intel_vvp_3d_lut_instance* instance);
  
/**
 * \brief Enable 3D LUT operation
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    enable operation
 *               false - passthrough
 *               true  - enable 3D LUT processing 
 * \return       kIntelVvp3dLutOk           if successful
 *               kIntelVvp3dLutInstanceErr  invalid instance parameter
 */
int intel_vvp_3d_lut_enable(intel_vvp_3d_lut_instance* instance, bool enable);

/**
 * \brief Select buffer for processing
 *
 * Switch between available buffers. Only applicable if the 3D LUT has been configured as double buffered
 * 
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    buffer select (0 or 1)
 * \return       kIntelVvp3dLutOk           if successful
 *               kIntelVvp3dLutInstanceErr  invalid instance parameter
 *               kIntelVvp3dLutErrBuffer    if buffer parameter is out of range or double buffer is not configured
 */
int  intel_vvp_3d_lut_buffer_select(intel_vvp_3d_lut_instance* instance, uint8_t buffer);

/**
 * \brief Load values into 3D LUT table
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    r_idx, red index.   Range 0 to (LUT dimension - 1) 
 * \param[in]    g_idx, green index. Range 0 to (LUT dimension - 1) 
 * \param[in]    b_idx, blue index.  Range 0 to (LUT dimension - 1) 
 * \param[in]    buffer.   Range 0 to 1 (for double buffered configuration)
 * \param[in]    r_val, red value.   Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    g_val, green value. Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    b_val, blue value.  Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    a_val, alpha value. Range 0 to ((2 ^ LUT depth) - 1). Must be zero if unsupported
 * \return       kIntelVvp3dLutOk             if successful
 *               kIntelVvp3dLutInstanceErr    invalid instance parameter
 *               kIntelVvp3dLutErrIndexRange  if r_idx, g_idx or b_idx is out of range
 *               kIntelVvp3dLutErrBuffer      if buffer is out of range
 *               kIntelVvp3dLutErrAlpha       if alpha value set and not supported
 *               kIntelVvp3dLutErrValueRange  if r_val, g_val, b_val or a_val is out of range 
 */
int  intel_vvp_3d_lut_load(intel_vvp_3d_lut_instance* instance,
                           uint16_t r_idx, uint16_t g_idx, uint16_t b_idx, uint8_t buffer,
                           uint16_t r_val, uint16_t g_val, uint16_t b_val, uint16_t a_val);

/**
 * \brief Set RAM n Control register
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n. Range 0 to 7
 * \param[in]    address.      Range depends on n, lut_double_buffered, and lut_dimension.
 * \param[in]    write_enable (0 or 1)
 * \param[in]    auto_inc, auto increment mode (0 or 1)
 * \return       kIntelVvp3dLutOk               if successful
 *               kIntelVvp3dLutInstanceErr      invalid instance parameter
 *               kIntelVvp3dLutErrRamIndexRange if ram_index out of range
 */
int intel_vvp_3d_lut_set_ram_ctrl(intel_vvp_3d_lut_instance* instance,
                                 uint8_t ram_index, uint32_t address, bool write_enable, bool auto_inc);

/**
 * \brief Set RAM n Data Lower register
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n.
 * \param[in]    data, 32-bit word. (not checked for range)
 * \return       kIntelVvp3dLutOk               if successful
 *               kIntelVvp3dLutInstanceErr      invalid instance parameter
 *               kIntelVvp3dLutErrRamIndexRange if ram_index out of range
 */
int intel_vvp_3d_lut_set_ram_data_lower(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index, uint32_t data);

/**
 * \brief Set RAM n Data Upper register
 *
 * Does not check whether Data Upper exists to avoid overhead
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n.
 * \param[in]    data, 32-bit word. (not checked for range)
 * \return       kIntelVvp3dLutOk               if successful
 *               kIntelVvp3dLutInstanceErr      invalid instance parameter
 *               kIntelVvp3dLutErrRamIndexRange if ram_index out of range
 */
int intel_vvp_3d_lut_set_ram_data_upper(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index, uint32_t data);

/**
 * \brief Get RAM n Data Lower register
 *
 * Does not check whether Data Lower is readable to avoid overhead
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n.
 * \return       RAM n Data Lower 32-bit word
 */
uint32_t intel_vvp_3d_lut_get_ram_data_lower(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index);

/**
 * \brief Get RAM n Data Upper register
 *
 * Does not check whether Data Upper exists/is readable to avoid overhead
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n.
 * \return       RAM n Data Upper 32-bit word.
 */
uint32_t intel_vvp_3d_lut_get_ram_data_upper(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index);

/**
 * \brief Set RAM n Data registers using RGB values as arguments
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n.
 * \param[in]    r_val, red value.   Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    g_val, green value. Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    b_val, blue value.  Range 0 to ((2 ^ LUT depth) - 1) 
 * \return       kIntelVvp3dLutOk               if successful
 *               kIntelVvp3dLutInstanceErr      invalid instance parameter
 *               kIntelVvp3dLutErrRamIndexRange if ram_index out of range
 *               kIntelVvp3dLutErrValueRange    if r_val, g_val or b_val is out of range 
 */
int intel_vvp_3d_lut_set_ram_data_rgb(intel_vvp_3d_lut_instance* instance,
                                      uint8_t ram_index,
                                      uint16_t r_val, uint16_t g_val, uint16_t b_val);

/**
 * \brief Set RAM n Data registers using RGBA values as arguments
 *
 * \param[in]    instance, pointer to the intel_vvp_3d_lut_instance
 * \param[in]    ram_index, n.
 * \param[in]    r_val, red value.   Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    g_val, green value. Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    b_val, blue value.  Range 0 to ((2 ^ LUT depth) - 1) 
 * \param[in]    a_val, alpha value. Range 0 to ((2 ^ LUT depth) - 1) 
 * \return       kIntelVvp3dLutOk               if successful
 *               kIntelVvp3dLutInstanceErr      invalid instance parameter
 *               kIntelVvp3dLutErrRamIndexRange if ram_index out of range
 *               kIntelVvp3dLutErrValueRange    if r_val, g_val, b_val or a_val is out of range 
 */
int intel_vvp_3d_lut_set_ram_data_rgba(intel_vvp_3d_lut_instance* instance,
                                      uint8_t ram_index,
                                      uint16_t r_val, uint16_t g_val, uint16_t b_val, uint16_t a_val);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_3D_LUT_H__
