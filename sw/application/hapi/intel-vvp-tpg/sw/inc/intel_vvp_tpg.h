/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_tpg_instance and associated functions
 *
 * Driver for the Video & Vision Processing Test Pattern Generator Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_tpg_regs.h
 */
#ifndef __INTEL_VVP_TPG_H__
#define __INTEL_VVP_TPG_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_TPG_PRODUCT_ID                           0x0236u              ///< TPG product ID
#define INTEL_VVP_TPG_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_TPG_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_TPG_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< TPG register read function
#define INTEL_VVP_TPG_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< TPG register write function

typedef struct intel_vvp_tpg_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    uint8_t                 num_patterns;
    uint8_t                 bps;
    uint8_t                 pip;
    uint16_t                max_color_value;
    bool                    debug_enabled;
    bool                    lite_mode;
} intel_vvp_tpg_instance;

typedef enum 
{
    kIntelVvpTpgBarsPattern,
    kIntelVvpTpgUniformPattern,
    kIntelVvpTpgPathologicalPattern,
    kIntelVvpTpgZonePlatePattern,
    kIntelVvpTpgDigitalClockPattern,
    kIntelVvpTpgSignalTapPattern,
    kIntelVvpTpgInvalidPattern
} eIntelVvpTpgPatternType;

typedef enum 
{
    kIntelVvpTpgRgb,
    kIntelVvpTpgYcc444,
    kIntelVvpTpgYcc422,
    kIntelVvpTpgYcc420,
    kIntelVvpTpgMono,
    kIntelVvpTpgInvalidColor
} eIntelVvpTpgPatternColor;

typedef struct intel_vvp_tpg_pattern_s
{
    eIntelVvpTpgPatternType     type;
    eIntelVvpTpgPatternColor    color;
} intel_vvp_tpg_pattern;

typedef enum 
{
    kIntelVvpTpgRegMapVersionErr            = -100,
    kIntelVvpTpgPatternNumErr               = -101,
    kIntelVvpTpgPatternDataErr              = -102,
    kIntelVvpTpgBarTypeErr                  = -103,
    kIntelVvpTpgC1Err                       = -104,
    kIntelVvpTpgC2Err                       = -105,
    kIntelVvpTpgC3Err                       = -106,
    kIntelVvpTpgScaleFactorErr              = -107,
    kIntelVvpTpgFpsErr                      = -108
} eTpgErrors;

typedef enum 
{
    kIntelVvpTpgColorBars,
    kIntelVvpTpgGreyBars,
    kIntelVvpTpgBlackWhiteBars,
    kIntelVvpTpgMixedBars,
    kIntelVvpTpgInvalidBars
} eIntelVvpTpgBarsType;

/**
 * \brief Initialise a TPG instance
 * 
 * Initialization function for a VVP TPG instance.
 * Attempts to initialize the fields of the TPG and its base core
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance to initialize
 * \param[in]  base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreInstanceErr if instance is NULL
 *             kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *             kIntelVvpCorePidErr if the product id of the core is not the TPG product id (0x0236)
 *             kIntelVvpGuardBandsRegMapVersionErr if the register map is not supported
 * \remarks    On returning a non-zero error code the instance will not be initialized and
 *             cannot be used further by the application using this driver
 * \remarks    The TPG is stopped upon successful return and must be started/enabled to produce any data
 */
int intel_vvp_tpg_init(intel_vvp_tpg_instance* instance, intel_vvp_core_base base);
    
/**
 * \brief returns configuration status of lite mode in tpg core
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance 
 * \return     true if core is in lite mode
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
bool intel_vvp_tpg_get_lite_mode(intel_vvp_tpg_instance* instance);

/**
 * \brief returns configuration status of debug mode in tpg core
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance 
 * \return     true if debug (read of registers) is available
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
bool intel_vvp_tpg_get_debug_enabled(intel_vvp_tpg_instance* instance);

/**
 * \brief Returns the number of patterns implemented at compile-time
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     number of patterns set up in HW (0 in case of error)
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
uint8_t intel_vvp_tpg_get_num_patterns(intel_vvp_tpg_instance* instance);

/**
 *\brief Returns number of bits in a color plane (such as r,g,b)
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     number of bits per color sample 8-16 (0 in case of error)
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
uint8_t intel_vvp_tpg_get_bits_per_sample(intel_vvp_tpg_instance* instance);

/**
 * \brief Returns number of pixels processed in parallel
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     number of pixels processed in parallel 1-8 (0 in case of error)
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
uint8_t intel_vvp_tpg_get_pixels_in_parallel(intel_vvp_tpg_instance* instance);

/**
 * \brief Returns pattern type and subsampling/colorspace for specified pattern
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  pattern to get data for (0 to num_patterns-1)
 * \param[out] pattern_data, storage space for recovered pattern data (filled during the call)
 * \return     kIntelVvpCoreOk in case of success, error code otherwise
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpTpgPatternNumErr, if pattern is not in the valid range
 *             kIntelVvpCoreNullPtrErr if pattern_data is a NULL pointer
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
int intel_vvp_tpg_get_pattern_data(intel_vvp_tpg_instance* instance, uint8_t pattern, intel_vvp_tpg_pattern* pattern_data);

/**
 * \brief get running status of the tpg IP
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance 
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
bool intel_vvp_tpg_is_running(intel_vvp_tpg_instance* instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance 
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_tpg_instance parameterized in full mode
 */
bool intel_vvp_tpg_get_commit_status(intel_vvp_tpg_instance* instance);

/**
 * \brief Read the status register
 * 
 * \param[in]  instance, an intel_vvp_tpg_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
uint8_t intel_vvp_tpg_get_status(intel_vvp_tpg_instance *instance);

/**
 * \brief Enable or disable the TPG
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  enabled, true to enable the TPG
 * \return     errorcode if invalid instance
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
int intel_vvp_tpg_enable(intel_vvp_tpg_instance* instance, bool enabled);

/**
 * \brief Start the TPG
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \remarks    alias for intel_vvp_tpg_enable(instance, true)
 * \return     kIntelVvpCoreInstanceErr if invalid instance, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
int intel_vvp_tpg_start(intel_vvp_tpg_instance* instance);

/**
 * \brief Stop the TPG
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \remarks    alias for intel_vvp_tpg_enable(instance, false)
 * \return     kIntelVvpCoreInstanceErr if invalid instance, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
int intel_vvp_tpg_stop(intel_vvp_tpg_instance* instance);

/**
 * \brief Returns TPG enabled state
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     true if TPG is enabled
 * \pre        instance is a valid intel_vvp_tpg_instance and debug is enabled
 */
bool intel_vvp_tpg_is_enabled(intel_vvp_tpg_instance* instance);

/**
 * \brief Select required pattern
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  pattern, index of pattern to select 0 to (n-1)
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreInstanceErr if the instance is invalid,
 *             kIntelVvpTpgPatternNumErr if the pattern is not in the valid range
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized
 */
int intel_vvp_tpg_set_pattern(intel_vvp_tpg_instance* instance, uint8_t pattern);

/**
 * \brief Get selected pattern
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     the selected pattern, errorcode 255 if invalid instance or pattern
 * \pre        instance is a valid intel_vvp_tpg_instance and debug is enabled
 */
uint8_t intel_vvp_tpg_get_pattern(intel_vvp_tpg_instance* instance);


/**
 * \brief Returns number of fields since the current sequence was started
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     Number of fields since the last sequence reset was actioned.
 *             A reset is triggered with a change to the image specification
 *             (after a write to the image info area) or a pattern swap.
 * \remarks    The value returned is temporarilly undefined after reset until
 *             a pattern and/or output dimensions are set
 * \pre        instance is a valid intel_vvp_tpg_instance and debug is enabled
 */
uint32_t intel_vvp_tpg_get_fields_since_sequence_reset(intel_vvp_tpg_instance* instance);

/**
 * \brief Set number of control packets to insert before the field. This method is *currently unsupported*.
 *  
 * \param[in]   instance, pointer to the intel_vvp_tpg_instance
 * \param[in]   pkt_count, number of control packets to insert before the field
 * \return      kIntelVvpCoreOk if the number was set,
 *              kIntelVvpCoreInstanceErr if the instance is invalid (NULL or lite mode)
 * \pre         instance is a valid intel_vvp_tpg_instance and configured in full mode
 */
// Currently unsupported
// int intel_vvp_tpg_set_pre_field_ctrl_pkts(intel_vvp_tpg_instance* instance, uint32_t pkt_count);

/**
 * \brief Returns number of control packets to insert before the field. This method is *currently unsupported*.
 *  
 * \param[in]   instance, pointer to the intel_vvp_tpg_instance
 * \return      number of control packets to insert before the field
 * \pre         instance is a valid intel_vvp_tpg_instance, configured in full mode with debug enabled
 */
// Currently unsupported
// uint32_t intel_vvp_tpg_get_pre_field_ctrl_pkts(intel_vvp_tpg_instance* instance);

/**
 * \brief Set number of control packets to insert after the field. This method is *currently unsupported*.
 *  
 * \param[in]   instance, pointer to the intel_vvp_tpg_instance
 * \param[in]   pkt_count, number of control packets to insert after the field
 * \return      kIntelVvpCoreOk if the number was set,
 *              kIntelVvpCoreInstanceErr if the instance is invalid (NULL or lite mode)
 * \pre         instance is a valid intel_vvp_tpg_instance and configured in full mode
 */
// Currently unsupported
// int intel_vvp_tpg_set_post_field_ctrl_pkts(intel_vvp_tpg_instance* instance, uint32_t pkt_count);

/**
 * \brief Returns number of control packets to insert after the field. This method is *currently unsupported*.
 *  
 * \param[in]   instance, pointer to the intel_vvp_tpg_instance
 * \return      number of control packets to insert after the field
 * \pre         instance is a valid intel_vvp_tpg_instance, configured in full mode with debug enabled
 */
// Currently unsupported
// uint32_t intel_vvp_tpg_get_post_field_ctrl_pkts(intel_vvp_tpg_instance* instance);

/**
 * \brief Sets the type of color bars that will be generated
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  bar_type of type eIntelVvpTpgBarsType
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgBarsPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_bars_type(intel_vvp_tpg_instance* instance, eIntelVvpTpgBarsType bar_type);

/**
 * \brief Returns the type of color bars that will be generated by the color bar pattern
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \return     value of the enumerated type eIntelVvpTpgBarsType
 * \pre        instance is a valid intel_vvp_tpg_instance, configured with debug enabled
 */
eIntelVvpTpgBarsType intel_vvp_tpg_get_bars_type(intel_vvp_tpg_instance* instance);

/**
 * \brief Set the three colors for each color plane generated by the uniform pattern
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  color1, value for color1
 * \param[in]  color2, value for color2
 * \param[in]  color3, value for color3
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpTpgC1Err to kIntelVvpTpgC3Err if any of the color values has an invalid value based on
 *                 the number of bits per color sample
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgUniformPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_colors(intel_vvp_tpg_instance* instance, uint16_t color1, uint16_t color2, uint16_t color3);

/**
 * \brief Returns three colors for each color plane
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] color1, a pointer to a uint16_t value to store color1
 * \param[out] color2, a pointer to a uint16_t value to store color2
 * \param[out] color3, a pointer to a uint16_t value to store color3
 * \return     kIntelVvpCoreOk in case of success, the colors have been updated
 *             kIntelVvpCoreInstanceErr if the instance is NULL or does not support debug
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the color values.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_colors(intel_vvp_tpg_instance* instance, uint16_t* color1, uint16_t* color2, uint16_t* color3);

/**
 * \brief Sets the zone plate origin (centre)
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  x_coord, value for x coord of zone plate origin
 * \param[in]  y_coord, value for y coord of zone plate origin
 * \return     kIntelVvpCoreOk in case of success, the origin locaiton has been updated
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgZonePlatePattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_zone_plate_origin(intel_vvp_tpg_instance* instance, uint16_t x_coord, uint16_t y_coord);

/**
 * \brief Returns the zone plate origin (centre)
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] x_coord, a pointer to a uint16_t value to store x_coord
 * \param[out] y_coord, a pointer to a uint16_t value to store y_coord
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the x/y values.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_zone_plate_origin(intel_vvp_tpg_instance* instance, uint16_t* x_coord, uint16_t* y_coord);

/**
 * \brief Sets the zone plate scaling factors (coefficients)
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  scaling_factor, value for coarse scaling factor of zone plate
 * \param[in]  fine_tune_factor, value for fine tune scaling factor of zone plate
 * \return     kIntelVvpCoreOk in case of success, the scaling factors have been updated
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgZonePlatePattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_zone_plate_factors(intel_vvp_tpg_instance* instance, uint16_t scaling_factor, uint16_t fine_tune_factor);

/**
 * \brief Returns the zone plate origin (centre)
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] scaling_factor, pointer to a uint16_t to store scaling_factor
 * \param[out] fine_tune_factor, pointer to a uint16_t to store fine_tune_factor
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the x/y values.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_zone_plate_factors(intel_vvp_tpg_instance* instance, uint16_t* scaling_factor, uint16_t* fine_tune_factor);

/**
 * \brief Sets the background color the digital clock. The value set is shared with the signaltap counter; calling
 *        intel_vvp_tpg_set_signaltap_background_color will overwrite this value.
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  background_r, value to set for the r/Cr color plane
 * \param[in]  background_g, value to set for the g/Y color plane
 * \param[in]  background_b, value to set for the b/Cb color plane
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kkIntelVvpTpgC1Err to kIntelVvpTpgC3Err if any of the color values has an invalid value based on
 *                 the number of bits per color sample. C1 corresponds with R, C3 corresponds with B.
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgDigitalClockPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_digital_clock_background_color(intel_vvp_tpg_instance* instance, uint16_t background_r, uint16_t background_g, uint16_t background_b);

/**
 * \brief Returns the current background color of the digital clock
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] background_r, pointer to store the r/Cr color value
 * \param[out] background_g, pointer to store the g/Y color value
 * \param[out] background_b, pointer to store the b/Cb color value
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the color pointers.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_digital_clock_background_color(intel_vvp_tpg_instance* instance, uint16_t* font_r, uint16_t* font_g, uint16_t* font_b);

/**
 * \brief Sets the font color the digital clock. The value set is shared with the signaltap counter; calling
 *        intel_vvp_tpg_set_signaltap_font_color will overwrite this value.
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  background_r, value to set for the r/Cr color plane
 * \param[in]  background_g, value to set for the g/Y color plane
 * \param[in]  background_b, value to set for the b/Cb color plane
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kkIntelVvpTpgC1Err to kIntelVvpTpgC3Err if any of the color values has an invalid value based on
 *                 the number of bits per color sample. C1 corresponds with R, C3 corresponds with B.
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgDigitalClockPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_digital_clock_font_color(intel_vvp_tpg_instance* instance, uint16_t font_r, uint16_t font_g, uint16_t font_b);

/**
 * \brief Returns the current font color of the digital clock
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] background_r, pointer to store the r/Cr color value
 * \param[out] background_g, pointer to store the g/Y color value
 * \param[out] background_b, pointer to store the b/Cb color value
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the color pointers.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_digital_clock_font_color(intel_vvp_tpg_instance* instance, uint16_t* font_r, uint16_t* font_g, uint16_t* font_b);

/**
 * \brief Sets the digital clock location (coordinates of top left corner). This value is shared with
 *          the signaltap counter; calling intel_vvp_tpg_set_signaltap_location will overwrite these values
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  x_coord, value for x coord of top-left pixel of the digital clock
 * \param[in]  y_coord, value for y coord of top-left pixel of the digital clock
 * \return     kIntelVvpCoreOk in case of success, the origin locaiton has been updated
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgDigitalClockPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_digital_clock_location(intel_vvp_tpg_instance* instance, uint16_t location_x, uint16_t location_y);

/**
 * \brief Returns the current digital clock location
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] x_coord, a pointer to a uint16_t value to store x_coord
 * \param[out] y_coord, a pointer to a uint16_t value to store y_coord
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the x/y values.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_digital_clock_location(intel_vvp_tpg_instance* instance, uint16_t* location_x, uint16_t* location_y);

/**
 * \brief Sets the current digital clock scale factor
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  scale_factor, value corresponding to the scale factor
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for the scale factor
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgDigitalClockPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_digital_clock_scale_factor(intel_vvp_tpg_instance* instance, uint16_t scale_factor);

/**
 * \brief Returns the current digital clock scale factor
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] scale_factor, a pointer to a uint16_t value to store the scale factor
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for the scale factor
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_digital_clock_scale_factor(intel_vvp_tpg_instance* instance, uint16_t* scale_factor);

/**
 * \brief Sets the current digital clock FPS. This value is shared with
 *          the signaltap counter; calling intel_vvp_tpg_set_signaltap_fps will overwrite this value
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  scale_factor, value corresponding to the FPS
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for the FPS
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgDigitalClockPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_digital_clock_fps(intel_vvp_tpg_instance* instance, uint8_t fps);

/**
 * \brief Returns the current digital clock FPS setting
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] fps, a pointer to a uint16_t value to store the FPS
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for the FPS
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_digital_clock_fps(intel_vvp_tpg_instance* instance, uint8_t* fps);

/**
 * \brief Sets the background color the signaltap counter. The value set is shared with the digital clock; calling
 *        intel_vvp_tpg_set_digital_clock_background_color will overwrite this value.
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  background_r, value to set for the r/Cr color plane
 * \param[in]  background_g, value to set for the g/Y color plane
 * \param[in]  background_b, value to set for the b/Cb color plane
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kkIntelVvpTpgC1Err to kIntelVvpTpgC3Err if any of the color values has an invalid value based on
 *                 the number of bits per color sample. C1 corresponds with R, C3 corresponds with B.
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgSignalTapPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_signaltap_background_color(intel_vvp_tpg_instance* instance, uint16_t background_r, uint16_t background_g, uint16_t background_b);

/**
 * \brief Returns the current background color of the signaltap counter
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] background_r, pointer to store the r/Cr color value
 * \param[out] background_g, pointer to store the g/Y color value
 * \param[out] background_b, pointer to store the b/Cb color value
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the color pointers.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_signaltap_background_color(intel_vvp_tpg_instance* instance, uint16_t* font_r, uint16_t* font_g, uint16_t* font_b);

/**
 * \brief Sets the font color the signaltap counter. The value set is shared with the digital clock; calling
 *        intel_vvp_tpg_set_digital_clock_font_color will overwrite this value.
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  background_r, value to set for the r/Cr color plane
 * \param[in]  background_g, value to set for the g/Y color plane
 * \param[in]  background_b, value to set for the b/Cb color plane
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kkIntelVvpTpgC1Err to kIntelVvpTpgC3Err if any of the color values has an invalid value based on
 *                 the number of bits per color sample. C1 corresponds with R, C3 corresponds with B.
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgSignalTapPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_signaltap_font_color(intel_vvp_tpg_instance* instance, uint16_t font_r, uint16_t font_g, uint16_t font_b);

/**
 * \brief Returns the current font color of the signaltap counter
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] background_r, pointer to store the r/Cr color value
 * \param[out] background_g, pointer to store the g/Y color value
 * \param[out] background_b, pointer to store the b/Cb color value
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the color pointers.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_signaltap_font_color(intel_vvp_tpg_instance* instance, uint16_t* font_r, uint16_t* font_g, uint16_t* font_b);

/**
 * \brief Sets the signaltap location (coordinates of top left corner). This value is shared with
 *          the digital clock; calling intel_vvp_tpg_set_digital_clock_location will overwrite these values
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  x_coord, value for x coord of top-left pixel of the digital clock
 * \param[in]  y_coord, value for y coord of top-left pixel of the digital clock
 * \return     kIntelVvpCoreOk in case of success, the origin locaiton has been updated
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgSignalTapPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_signaltap_location(intel_vvp_tpg_instance* instance, uint16_t location_x, uint16_t location_y);

/**
 * \brief Returns the current signaltap counter location
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] x_coord, a pointer to a uint16_t value to store x_coord
 * \param[out] y_coord, a pointer to a uint16_t value to store y_coord
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for any of the x/y values.
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_signaltap_location(intel_vvp_tpg_instance* instance, uint16_t* location_x, uint16_t* location_y);

/**
 * \brief Sets the current signaltap FPS. This value is shared with
 *          the digital clock; calling intel_vvp_tpg_set_digital_clock_fps will overwrite this value
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[in]  scale_factor, value corresponding to the FPS
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for the FPS
 * \pre        instance is a valid intel_vvp_tpg_instance successfully initialized. 
 *             Although this is not checked, a pattern of type kIntelVvpTpgSignalTapPattern must
 *             have been configured and selected for this parameter to have an effect
 */
int intel_vvp_tpg_set_signaltap_fps(intel_vvp_tpg_instance* instance, uint8_t fps);

/**
 * \brief Returns the current signaltap FPS setting
 *  
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance
 * \param[out] fps, a pointer to a uint16_t value to store the FPS
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid
 *             kIntelVvpCoreNullPtrError if a NULL pointer is passed for the FPS
 * \pre        instance is a valid intel_vvp_tpg_instance with debug mode enabled
 */
int intel_vvp_tpg_get_signaltap_fps(intel_vvp_tpg_instance* instance, uint8_t* fps);

/**
 * 
 * \brief commit any outstanding writes from setxxxx commands
 * 
 * \param[in]  instance, pointer to the intel_vvp_tpg_instance 
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreInstanceErr if the instance is NULL or not configured in full mode
 * \pre        instance is a valid intel_vvp_tpg_instance configured in full mode
 */
int intel_vvp_tpg_commit_writes(intel_vvp_tpg_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_TPG_H__ */
