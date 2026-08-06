/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_mixer_instance and associated functions
 *
 * Driver for the Video & Vision Processing alpha-blending mixer Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_mixer_regs.h
 */

#ifndef __INTEL_VVP_MIXER_H__
#define __INTEL_VVP_MIXER_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"
#include "intel_vvp_mixer_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_MIXER_PRODUCT_ID                           0x0233u              ///< Mixer product ID
#define INTEL_VVP_MIXER_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_MIXER_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_MIXER_MAX_NUM_LAYERS                       8                    ///< Maximum number of supported layers (including the background layer)

#define INTEL_VVP_MIXER_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Mixer register read function
#define INTEL_VVP_MIXER_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Mixer register write function

typedef enum {
    kIntelVvpMixerRegMapVersionErr = -100,       // register map issue at initialization
    kIntelVvpMixerParameterErr     = -101,       // parameter error in a function call
    kIntelVvpMixerLayerErr         = -102,       // layer parameter error in a function call
} eIntelVvpMixerErrors;

typedef enum {
    kIntelVvpMixerBlendConfigNone                 = 0, // Alpha blending not supported
    kIntelVvpMixerBlendConfigStaticAlpha          = 1, // Alpha blending using a static alpha value per input frame is supported
    kIntelVvpMixerBlendConfigInputAlpha           = 2, // Alpha blending using a per-pixel alpha value given with input pixels is supported
    kIntelVvpMixerBlendConfigStaticOrInputAlpha   = 3  // Alpha blending using either a constant alpha value or a per-pixel alpha value given with input pixels is supported
} eIntelVvpBlendModeConfig;           ///< Compile-time selection of the supported blending methods (per layer)

typedef struct intel_vvp_mixer_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool                    lite_mode;               ///< Whether the IP is configured in lite or full mode
    bool                    debug_enabled;           ///< Whether debug is enabled (for read-back of write registers)
    uint8_t                 num_layers;              ///< Number of layers (background included)
    uint8_t                 pixels_in_parallel;      ///< Number of pixels_in_parallel
    struct
    {
        bool                        restricted_offsets;      ///< With pixel_in_parallel > 1, restricted_offsets indicates that the layer only supports offsets that are a multiple of pixels_in_parallel
        eIntelVvpBlendModeConfig    blend_mode_config;       ///< Alpha-blending modes supported by the layer
    } foreground_layer_config[INTEL_VVP_MIXER_MAX_NUM_LAYERS-1]; // no config associated with the background layer, index 0 stores the config of layer1
} intel_vvp_mixer_instance;	   

typedef enum {
    kIntelVvpMixerBlendTransparent = INTEL_VVP_MIXER_BLEND_TRANSPARENT,   // Input not displayed
    kIntelVvpMixerBlendOpaque      = INTEL_VVP_MIXER_BLEND_OPAQUE,        // Input completely hides lower layers
    kIntelVvpMixerBlendStaticAlpha = INTEL_VVP_MIXER_BLEND_STATIC_ALPHA,  // Perform alpha-blending using a constant static alpha value to mix this layer with lower layers
    kIntelVvpMixerBlendInputAlpha  = INTEL_VVP_MIXER_BLEND_INPUT_ALPHA    // Use the per-pixel alpha value from the input to blend this layer with lower layers
} eIntelVvpBlendMode;                 ///< Run-time selection of the blending mode

/**
 * \brief Initialise a mixer instance
 * 
 * Initialization function for a VVP mixer instance.
 * Attempts to initialize the fields of the mixer and its base core
 * 
 * \param[in]    instance, pointer to the intel_vvp_guard_bands_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the mixer product id (0x0233)
 *               kIntelVvpMixerRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_mixer_init(intel_vvp_mixer_instance* instance, intel_vvp_core_base base);
    

/**
 * \brief Query the lite_mode parameter of a mixer instance
 * 
 * \param[in]  instance, an intel_vvp_mixer_instance
 * \return     the lite_mode parameter in the intel_vvp_mixer_instance
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
bool intel_vvp_mixer_get_lite_mode(intel_vvp_mixer_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a mixer instance
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \return     the debug_enabled parameter in the intel_vvp_mixer_instance
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
bool intel_vvp_mixer_get_debug_enabled(intel_vvp_mixer_instance* instance);

/**
 * \brief Returns the number of layers (background layer included)
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \return     the num_layers parameter in the intel_vvp_mixer_instance
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
uint8_t intel_vvp_mixer_get_num_layers(intel_vvp_mixer_instance* instance);

/**
 * \brief Returns the number of pixels transmitted in parallel (per beat/clock cycle)
 *        on each layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \return     the pixels_in_parallel parameter in the intel_vvp_mixer_instance
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
uint8_t intel_vvp_mixer_get_pixels_in_parallel(intel_vvp_mixer_instance* instance);

/**
 * \brief Returns the alpha blending modes supported by the layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \param[in]  layer, the ID of the foreground layer in the range [1..num_layers-1]
 * \return     kIntelVvpMixerBlendConfigNone, for invalid layers or layers that do not support blending,
 *             supported blending modes otherwise (static alpha, in-band alpha or both)
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
eIntelVvpBlendModeConfig intel_vvp_mixer_get_supported_blend_mode(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief Returns whether the layer can only handle offsets that are a multiple of the number of pixels
 *        transmitted in parallel
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \param[in]  layer, the ID of the foreground layer in the range [1..num_layers-1]
 * \return     true if there is a restriction on the offset values that can be implemented by the layer
 * \remarks    always return false if pixels_in_parallel == 1 or invalid instance
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
bool intel_vvp_mixer_requires_restricted_offsets(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief get running status of the mixer IP
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
bool intel_vvp_mixer_is_running(intel_vvp_mixer_instance* instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance 
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_mixer_instance parameterized in full mode
 */
bool intel_vvp_mixer_get_commit_status(intel_vvp_mixer_instance* instance);

/**
 * \brief Returns whether the specified layer is active of not
 * 
 * \param[in]  instance, an intel_vvp_mixer_instance
 * \param[in]  layer, index of the layer, in the range 1..(num_layers-1)
 * \return     true if layer is a valid layer ID and the layer is active
 * \remarks    layer 0 is always active
 * \pre        instance is a valid intel_vvp_mixer_instance and layer is valid
 */
bool intel_vvp_mixer_is_layer_active(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief Read the mixer status register
 * 
 * \param[in]  instance, an intel_vvp_mixer_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_mixer_instance successfully initialized
 */
uint32_t intel_vvp_mixer_get_status(intel_vvp_mixer_instance *instance);

/**
 * \brief        Returns the current input setup for a given layer
 *  
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer, 1-(num_layers-1)
 * \param[out] enabled, whether the layer is enabled
 * \param[out] consumed, whether the layer is displayed or consumed (the layer must be enabled)
 * \param[out] soft_start, whether the layer becomes active on the spot or once input video is seen
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if instance is NULL or debug is not enabled
 *             kIntelVvpCoreNullPtrErr if one of the pointers is NULL
 *             kIntelVvpMixerLayerErr if the layer is not within valid range
 * \remarks    Please refer to the user guide for the exact semantics of these flags and the
 *             corresponding mixer behaviours.
 * \pre        instance is valid and initialized, debug_mode is enabled and the layer is valid
 */
int intel_vvp_mixer_get_input_mode(intel_vvp_mixer_instance* instance, uint8_t layer, bool *enabled, bool *consumed, bool *soft_start);

/**
 * \brief Setup the input mode for a given layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer in the range [1..(num_layers-1)]
 * \param[in]  enable, true if the input layer is pulled in rather than disabled (backpressured)
 * \param[in]  consumed, true if input layer is accepted and discarded rather than being displayed
 * \param[in]  soft_start, whether the layer becomes active once input video is seen or on the spot
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if instance is NULL
 *             kIntelVvpMixerLayerErr if the layer is not within accpeted range
 *             kIntelVvpMixerParameterErr for other errors (enabled or soft_start parameters are overwritten as indicated below)
 * \remarks    If soft_start is true, the input will not become active until an initial valid beat is waiting
 *             at a frame sync (start of frame). If soft_start false, the mixer output will stall at the next frame sync 
 *             until the selected input is available
 * \remarks    A layer must be enabled to be discarded/consumed, enabled will be forced to true if consumed
 *             is set to true, soft_start will be forced to false if enabled is false, kIntelVvpMixerParameterErr
 *             is returned in such cases but the configuration will still be sent, after correction, to the hardware
 * \pre        instance is valid and initialized and the layer is valid
 */
int intel_vvp_mixer_set_input_mode(intel_vvp_mixer_instance* instance, uint8_t layer, bool enable, bool consume, bool soft_start);

/**
 * \brief Returns the alpha-blending mode for a given layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer, 1-(num_layers-1)
 * \return     blend mode value for specified layer,
 *             kIntelVvpMixerBlendTransparent will be returned in case of errors (invalid layer, debug not enabled)
 * \pre        instance is valid, initialized and with debug enabled. The layer is valid.
 */
eIntelVvpBlendMode intel_vvp_mixer_get_blend_mode(intel_vvp_mixer_instance* instance, uint8_t layer);


/**
 * \brief Sets the alpha-blending mode for a given layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer, 1-(num_layers-1)
 * \return     kIntelVvpCoreOk in case of succes
 *             kIntelVvpCoreInstanceErr if instance is NULL
 *             kIntelVvpMixerLayerErr if the layer is not within accpeted range
 *             kIntelVvpMixerParameterErr for other errors (blending mode not supported)
 * \pre        instance is valid and initialized and the layer is valid, The layer should support the
 *             requested blending mode.
 */
int intel_vvp_mixer_set_blend_mode(intel_vvp_mixer_instance* instance, uint8_t layer, eIntelVvpBlendMode blend_mode);

/**
 * \brief Retrieves the static alpha value for a given layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer, 1-(num_layers-1)
 * \return     The alpha value
 * \pre        instance is valid, debug is enabled and layer is a valid foreground layer
 */
uint32_t intel_vvp_mixer_get_static_alpha(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief Sets the static alpha value for a given layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer, 1-(num_layers-1)
 * \return     kIntelVvpCoreOk in case of succes
 *             kIntelVvpCoreInstanceErr if instance is NULL or the layer does not support static alpha blendingnew 
 *             kIntelVvpMixerLayerErr if the layer is not within accpeted range
 * \remarks    The layer must support static alpha blending and the blending mode should be set accordingly
 *             for this value to have an effect (the call does not perform such a check)
 * \pre        instance is valid, debug is enabled and layer is a valid foreground layer set for static alpha blending
 */
int intel_vvp_mixer_set_static_alpha(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t static_alpha);

/**
 * \brief  Get the horizontal offset for the layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer in the range [1..(num_layers-1)]
 * \pre        instance is valid, debug is enabled and layer is a valid foreground layer
 */
uint32_t intel_vvp_mixer_get_horiz_offset(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief Sets the horizontal offset for the layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer in the range [1..(num_layers-1)]
 * \return     kIntelVvpCoreOk in case of succes
 *             kIntelVvpCoreInstanceErr if instance is NULL
 *             kIntelVvpMixerLayerErr if the layer is not within accpeted range
 *             kIntelVvpMixerParameterErr for other errors (offset not respecting the restrictions)
 * \remarks    using an invalid offset on a layer with restricted offsets does not prevent the IP from being
 *             parameterized with a new offset but the HW will round the offset as appropriate
 * \pre        instance is valid and layer is a valid foreground layer,
 * \pre        offset is a multiple of pixels_in_parallel if the layer is restricting the range of valid offsets
 */
int intel_vvp_mixer_set_horiz_offset(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t offset);

/**
 * \brief Get the vertical offset for the layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer in the range [1..(num_layers-1)]
 * \pre        instance is valid, debug is enabled and layer is a valid foreground layer
 */
uint32_t intel_vvp_mixer_get_vert_offset(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief Sets the vertical offset for the layer
 * 
 * \param[in]  instance, pointer to the intel_vvp_mixer_instance
 * \param[in]  layer, index of layer in the range [1..(num_layers-1)]
 * \return     kIntelVvpCoreOk in case of succes
 *             kIntelVvpCoreInstanceErr if instance is NULL
 *             kIntelVvpMixerLayerErr if the layer is not within accpeted range
 * \pre        instance is valid and layer is a valid foreground layer,
 */
int intel_vvp_mixer_set_vert_offset(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t offset);

/**
 * \brief        Get the width for the layer (lite mode)
 * 
 * \param[in]    instance, pointer to the intel_vvp_mixer_instance
 * \param[in]    layer, index of layer in the range [1..(num_layers-1)]
 * \pre          instance is valid and parameterized in lite mode,
 *               debug is enabled and layer is a valid foreground layer
 */
uint32_t intel_vvp_mixer_get_width(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief        Sets the width for the layer (lite mode)
 * 
 * \param[in]    instance, pointer to the intel_vvp_mixer_instance
 * \param[in]    layer, index of layer in the range [1..(num_layers-1)]
 * \return       kIntelVvpCoreOk in case of succes
 *               kIntelVvpCoreInstanceErr if instance is NULL or lite_mode is false
 *               kIntelVvpMixerLayerErr if the layer is not within accpeted range
 * \pre          instance is valid, parameterized in lite mode,
 *               and layer is a valid foreground layer
 */
int intel_vvp_mixer_set_width(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t width);

/**
 * \brief        Get the height for the layer (lite mode)
 * 
 * \param[in]    instance, pointer to the intel_vvp_mixer_instance
 * \param[in]    layer, index of layer in the range [1..(num_layers-1)]
 * \pre          instance is valid and parameterized in lite mode,
 *               debug is enabled and layer is a valid foreground layer
 */
uint32_t intel_vvp_mixer_get_height(intel_vvp_mixer_instance* instance, uint8_t layer);

/**
 * \brief        Sets the height for the layer (lite mode)
 * 
 * \param[in]    instance, pointer to the intel_vvp_mixer_instance
 * \param[in]    layer, index of layer in the range [1..(num_layers-1)]
 * \return       kIntelVvpCoreOk in case of succes
 *               kIntelVvpCoreInstanceErr if instance is NULL or lite_mode is false
 *               kIntelVvpMixerLayerErr if the layer is not within accpeted range
 * \pre          instance is valid, parameterized in lite mode,
 *               and layer is a valid foreground layer
 */
int intel_vvp_mixer_set_height(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t height);

/**
 * \brief commit any outstanding writes from setxxxx commands
 * 
 * \param[in]    instance, pointer to the intel_vvp_mixer_instance 
 * \return       kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre          instance is valid and parameterized in lite mode, (commit is automatic in lite mode)
 */
int intel_vvp_mixer_commit_writes(intel_vvp_mixer_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_MIXER_H__
