/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_vfb_instance and associated functions
 *
 * Driver for the Video & Vision Processing Frame Buffer Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_vfb_regs.h
 */

#ifndef __INTEL_VVP_VFB_H__
#define __INTEL_VVP_VFB_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_VFB_PRODUCT_ID                           0x0237u              ///< Frame buffer product ID
#define INTEL_VVP_VFB_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_VFB_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_VFB_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Frame buffer register read function
#define INTEL_VVP_VFB_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Frame buffer register write function

    
typedef enum {
    kIntelVvpVfbRegMapVersionErr = -100,
} eIntelVvpVfbErrors;

typedef enum {
    kIntelVvpVfbPerfectPacking =  0,
    kIntelVvpVfbColorPacking   =  1,
    kIntelVvpVfbPixelPacking   =  2,
    kIntelVvpVfbInvalidPacking = -1,
} eIntelVvpVfbPacking;

typedef struct intel_vvp_vfb_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool lite_mode;
    bool debug_enabled;
    uint32_t max_width;
    uint32_t max_height;
    bool frame_drop_enabled;
    bool frame_repeat_enabled;
    bool invalid_frames_dropped;
} intel_vvp_vfb_instance;	   

/**
 * \brief Initialise a frame buffer instance
 * 
 * Initialization function for a VVP frame buffer instance.
 * Attempts to initialize the fields of the frame buffer and its base core
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the frame buffer product id (0x0237)
 *               kIntelVvpVfbRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 * \remarks      If already running, the frame buffer read side is stopped during initialization. It must be
 *               enabled, or re-enabled, for the frame buffer to output any data
 */
int intel_vvp_vfb_init(intel_vvp_vfb_instance* instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a frame buffer instance
 * 
 * \param[in]  instance, an intel_vvp_vfb_instance
 * \return the lite_mode field in the intel_vvp_vfb_instance
 */
bool intel_vvp_vfb_get_lite_mode(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the debug_enabled field in the intel_vvp_vfb_instance
 */
bool intel_vvp_vfb_get_debug_enabled(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the max_width parameter of a frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the max_width field in the intel_vvp_vfb_instance
 */
uint32_t intel_vvp_vfb_get_max_width(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the max_height parameter of a frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the max_height field in the intel_vvp_vfb_instance
 */
uint32_t intel_vvp_vfb_get_max_height(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the frame_drop_enabled parameter of a frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the frame_drop_enabled field in the intel_vvp_vfb_instance
 */
bool intel_vvp_vfb_is_frame_drop_enabled(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the frame_repeat_enabled parameter of a frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the frame_repeat_enabled field in the intel_vvp_vfb_instance
 */
bool intel_vvp_vfb_is_frame_repeat_enabled(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the invalid_frames_dropped parameter of a frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the invalid_frames_dropped field in the intel_vvp_vfb_instance
 */
bool intel_vvp_are_invalid_frames_dropped(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the compile-time memory base address used by the frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the base_address for the memory space allocated to teh frame buffer
 */
uint32_t intel_vvp_vfb_get_mem_base_address(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the compile-time stride between buffers for the frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the stride between frames on the memory space allocated to the frame buffer
 */
uint32_t intel_vvp_vfb_get_buffer_stride(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the compile-time stride between lines for the frame buffer instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the stride between frames on the memory space allocated to the frame buffer
 */
uint32_t intel_vvp_vfb_get_line_stride(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the compile-time bits per (color) sample  value
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the number of bits per color sample
 */
uint8_t intel_vvp_vfb_get_bits_per_sample(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the number of color planes compile-time parameter
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the number of color planes per pixel
 */
uint8_t intel_vvp_vfb_get_num_color_planes(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the number of pixels transmitted in parallel (per beat)
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the number of pixels transmitted in parallel on the video path
 */
uint8_t intel_vvp_vfb_get_pixels_in_parallel(intel_vvp_vfb_instance* instance);

/**
 * \brief Fetch and return the packing of video data in memory
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the packing algorithm used to store the video in memory
 *               kIntelVvpVfbPerfectPacking, memory words fully utilized, color samples may be split over consecutive memory words
 *               kIntelVvpVfbColorPacking, color samples are not split over consecutive memory words, up to bits_per_sample bits
 *                                         may be lost per memory word
 *               kIntelVvpVfbPixelPacking, pixels are not split over consecutive memory words, up to bits_per_sample * num_color_planes
 *               bits may be lost per memory word
 *               kIntelVvpVfbInvalidPacking, invalid instance
 */
eIntelVvpVfbPacking intel_vvp_vfb_get_mem_word_packing(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the running bit of the input/writer subsystem of a VFB instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the running bit of the input/writer status register
 * \remarks      true while processing a frame, false in-between frames
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
bool intel_vvp_vfb_is_input_running(intel_vvp_vfb_instance* instance);

/**
 * \brief Get the input/writer subsystem status of a VFB instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the status of the input/writer side
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_input_status(intel_vvp_vfb_instance* instance);

/**
 * \brief Query the running bit of the output/reader subsystem of a VFB instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the running bit of the output/reader status register
 * \remarks      true while outputting a frame, false in-between frames
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
bool intel_vvp_vfb_is_output_running(intel_vvp_vfb_instance* instance);

/**
 * \brief Get the output/reader subsystem status of a VFB instance
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       the status of the output/reader side
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_output_status(intel_vvp_vfb_instance* instance);


/**
 * \brief Read the value of the input fields counter
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       number of fields received on the input
 * \remarks      the counter is updated at the end of an input field on the
 *               input/writer side
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_num_input_fields(intel_vvp_vfb_instance* instance);

/**
 * \brief Read the value of the dropped fields counter
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       number of fields dropped on the input
 * \remarks      the counter is updated when the drop decision is made at the
 *               start of the next field on the input/writer side
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_num_dropped_fields(intel_vvp_vfb_instance* instance);

/**
 * \brief Read the value of the invalid fields counter
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       number of invalid fields on the input
 * \remarks      the counter is updated at the end of an input field on the
 *               input/writer side
 * \remarks      see user guide for more details and which fields/frames can
 *               be flagged as invalid
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_num_invalid_fields(intel_vvp_vfb_instance* instance);


/**
 * \brief Read the value of the output field counter
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       number of fields sent on the output
 * \remarks      the counter is updated at the end of each output field on the
 *               output/reader side. Each repetition of a source field is also
 *               included in the count
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_num_output_fields(intel_vvp_vfb_instance* instance);

/**
 * \brief Read the value of the repeat field counter
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance 
 * \return       number of fields repeated on the output
 * \remarks      the counter is updated at the end of each repeated field on the
 *               output/reader side. Each repetition of a source field is included
 *               in the count.
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
uint32_t intel_vvp_vfb_get_num_repeated_fields(intel_vvp_vfb_instance* instance);


/**
 * \brief Enable or disable the frame buffer output/read subsystem
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance
 * \param[in]    enabled, true to enable the output
 * \remarks      The output cannot produce output frames until the input/write
 *               subsystem has made at least on frame available in memory
 * \return       kIntelVvpCoreInstanceErr if the instance is invalid
 *               kIntelVvpCoreOk otherwise
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
int intel_vvp_vfb_output_enable(intel_vvp_vfb_instance* instance, bool enabled);


/**
 * \brief Enable the frame buffer output/read subsystem
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance
 * \remarks      alias for intel_vvp_vfb_output_enable(instance, true)
 * \return       kIntelVvpCoreInstanceErr if the instance is invalid
 *               kIntelVvpCoreOk otherwise
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
int intel_vvp_vfb_start_output(intel_vvp_vfb_instance* instance);

/**
 * \brief Disable the frame buffer output/read subsystem
 * 
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance
 * \remarks      alias for intel_vvp_vfb_output_enable(instance, false)
 * \return       kIntelVvpCoreInstanceErr if the instance is invalid
 *               kIntelVvpCoreOk otherwise
 * \pre          instance is a valid intel_vvp_vfb_instance successfully initialized
 */
int intel_vvp_vfb_stop_output(intel_vvp_vfb_instance* instance);


/**
 * \brief Returns the frame buffer enabled state
 *  
 * \param[in]    instance, pointer to the intel_vvp_vfb_instance
 * \return       true if the frame buffer output/read subsystem is enabled
 * \pre          instance is a valid intel_vvp_vfb_instance and debug is enabled
 */
bool intel_vvp_vfb_is_output_enabled(intel_vvp_vfb_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_VFB_H__
