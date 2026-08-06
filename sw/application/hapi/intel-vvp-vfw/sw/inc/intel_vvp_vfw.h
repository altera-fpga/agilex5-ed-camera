/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_vfw_instance and associated functions
 *
 * Driver for the Video & Vision Processing Frame Writer Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_vfw_regs.h
 */

#ifndef __INTEL_VVP_VFW_H__
#define __INTEL_VVP_VFW_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_vfw_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_VFW_PRODUCT_ID                           0x0249u              ///< Frame writer product ID
#define INTEL_VVP_VFW_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_VFW_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_VFW_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< frame writer register read function
#define INTEL_VVP_VFW_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< frame writer register write function

typedef enum {
    kIntelVvpVfwRegMapVersionErr = -100,
} eIntelVvpVfwErrors;

typedef enum {
    kIntelVvpVfwPerfectPacking =  INTEL_VVP_VFW_PERFECT_PACKING,
    kIntelVvpVfwColorPacking   =  INTEL_VVP_VFW_COLOR_PACKING,
    kIntelVvpVfwPixelPacking   =  INTEL_VVP_VFW_PIXEL_PACKING,
    kIntelVvpVfwInvalidPacking = -1,
} eIntelVvpVfwPacking;

typedef enum {
    kIntelVvpVfwStop           =  INTEL_VVP_VFW_STOP,
    kIntelVvpVfwFreeRunning    =  INTEL_VVP_VFW_FREE_RUNNING,
    kIntelVvpVfwSingleShot     =  INTEL_VVP_VFW_SINGLE_SHOT,
    kIntelVvpVfwInvalidRunMode = -1,
} eIntelVvpVfwRunMode;

typedef struct intel_vvp_vfw_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool lite_mode;
    bool debug_enabled;
    uint32_t max_height;
    uint32_t max_width;
    uint8_t bps;
    uint8_t num_color_planes;
    uint8_t pip;
    eIntelVvpVfwPacking packing;
} intel_vvp_vfw_instance;

/**
 * \brief Initialise a frame writer instance
 *
 * Initialization function for a VVP frame writer instance.
 * Attempts to initialize the fields of the frame writer and its base core
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the frame writer product id (0x0237)
 *               kIntelVvpVfwRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 * \remarks      If already running, the frame writer is stopped during initialization. It must be
 *               enabled, or re-enabled to output any data
 */
int intel_vvp_vfw_init(intel_vvp_vfw_instance* instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a frame writer instance
 *
 * \param[in]  instance, an intel_vvp_vfw_instance
 * \return the lite_mode field in the intel_vvp_vfw_instance
 */
bool intel_vvp_vfw_get_lite_mode(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a frame writer instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the debug_enabled field in the intel_vvp_vfw_instance
 */
bool intel_vvp_vfw_get_debug_enabled(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the max_height parameter of a frame writer instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the max_height field in the intel_vvp_vfw_instance
 */
uint32_t intel_vvp_vfw_get_max_height(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the max_width parameter of a frame writer instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the max_width field in the intel_vvp_vfw_instance
 */
uint32_t intel_vvp_vfw_get_max_width(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the bits per color sample parameter of a frame writer instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the bps field in the intel_vvp_vfw_instance
 */
uint8_t intel_vvp_vfw_get_bps(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the number of color planes parameter of a frame writer instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the num_color_planes field in the intel_vvp_vfw_instance
 */
uint8_t intel_vvp_vfw_get_number_of_color_planes(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the pixels_in_parallel parameter of a frame writer instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the pip field in the intel_vvp_vfw_instance
 */
uint8_t intel_vvp_vfw_get_pixels_in_parallel(intel_vvp_vfw_instance* instance);

/**
 * \brief Fetch and return the packing of video data in memory
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the packing algorithm used to store the video in memory
 *               kIntelVvpVfwPerfectPacking, memory words fully utilized, color samples may be split over consecutive memory words
 *               kIntelVvpVfwColorPacking, color samples are not split over consecutive memory words, up to bits_per_sample bits
 *                                         may be lost per memory word
 *               kIntelVvpVfwPixelPacking, pixels are not split over consecutive memory words, up to bits_per_sample * num_color_planes
 *               bits may be lost per memory word
 *               kIntelVvpVfwInvalidPacking, invalid instance
 */
eIntelVvpVfwPacking intel_vvp_vfw_get_mem_word_packing(intel_vvp_vfw_instance* instance);

/**
 * \brief Query the running bit of the frame writer
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the running bit of the status register
 * \remarks      true while processing a frame, false in-between frames
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
bool intel_vvp_vfw_is_running(intel_vvp_vfw_instance* instance);

/**
 * \brief Determine if the frame writer instance has any pending writes that have NOT been commited
 *
 * \param[in]  instance, pointer to the intel_vvp_vfw_instance
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_vfw_instance
 */
bool intel_vvp_vfw_get_commit_status(intel_vvp_vfw_instance* instance);

/**
 * \brief Get the frame writer status
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the status register of the frame writer
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
uint32_t intel_vvp_vfw_get_status(intel_vvp_vfw_instance* instance);


/**
 * \brief Read the value in the buffer available register
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the boolean value in the buffer available register of the frame writer
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
bool intel_vvp_vfw_is_buffer_available(intel_vvp_vfw_instance* instance);

/**
 * \brief Read the buffer_write_count counter
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the value in the buffer_write_count counter of the frame writer, incremented for
 *               each input field that was not dropped
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
uint32_t intel_vvp_vfw_get_buffer_write_count(intel_vvp_vfw_instance* instance);

/**
 * \brief Read the address returned for the last buffer marked as available
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the value in the buffer_start_address register of the frame writer
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
uint32_t intel_vvp_vfw_get_buffer_start_addr(intel_vvp_vfw_instance* instance);

/**
 * \brief Read the value in the f1_flag register for the last buffer marked as available
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the boolean value in the f1_flag register of the frame writer
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
bool intel_vvp_vfw_is_buffer_f1(intel_vvp_vfw_instance* instance);

/**
 * \brief Read the width returned for the last buffer marked as available
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the value written in the width register when the buffer was marked as available
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
uint32_t intel_vvp_vfw_get_buffer_width(intel_vvp_vfw_instance* instance);

/**
 * \brief Read the height returned for the last buffer marked as available
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the value written in the height register when the buffer was marked as available
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
uint32_t intel_vvp_vfw_get_buffer_height(intel_vvp_vfw_instance* instance);

/**
 * \brief Read the field_count register (full variant)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the value extracted from the last eof packet and written in the field_count register (full variant only)
 *               if the field was not dropped
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and configured for full variant
 */
uint32_t intel_vvp_vfw_get_field_count(intel_vvp_vfw_instance* instance);

/**
 * \brief Acknowledge a buffer marked as available, freeing space for subsequent writes
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
int intel_vvp_vfw_acknowledge_buffer(intel_vvp_vfw_instance* instance);

/**
 * \brief Change the run_mode
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    run_mode, new run mode
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and run_mode is valid
 */
int intel_vvp_vfw_set_run_mode(intel_vvp_vfw_instance* instance, eIntelVvpVfwRunMode run_mode);

/**
 * \brief Read the run_mode
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the run_mode register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and compiled with debug enabled
 */
eIntelVvpVfwRunMode intel_vvp_vfw_get_run_mode(intel_vvp_vfw_instance* instance);

/**
 * \brief Change the number of buffers available for writing
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    num_buffers, new number of buffers
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized, num_buffers > 0
 * \remarks      Please check user guide for upper-limit on num_buffers (value is truncated)
 */
int intel_vvp_vfw_set_num_buffers(intel_vvp_vfw_instance* instance, uint32_t num_buffers);

/**
 * \brief Read back the number of buffers made available for writing
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the num buffers register
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfw_get_num_buffers(intel_vvp_vfw_instance* instance);


/**
 * \brief Change the base address for the buffers in memory
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    base_addr, new base address
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
int intel_vvp_vfw_set_base_addr(intel_vvp_vfw_instance* instance, uint32_t base_addr);


/**
 * \brief Read back the base address for the buffers in memory
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the base address register
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfw_get_base_addr(intel_vvp_vfw_instance* instance);


/**
 * \brief Change the inter-buffer offset (ie, the size of a field buffer in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    inter_buffer_offset, new offset between buffers
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
int intel_vvp_vfw_set_inter_buffer_offset(intel_vvp_vfw_instance* instance, uint32_t inter_buffer_offset);

/**
 * \brief Read back the inter-buffer offset (ie, the size of a buffer in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the inter-buffer offset register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and the
 *               instance was compiled with debug enabled
 */
uint32_t intel_vvp_vfw_get_inter_buffer_offset(intel_vvp_vfw_instance* instance);

/**
 * \brief Change the inter-line offset (ie, the maximum size of a line in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    inter_line_offset, new offset between adjacent lines in bytes
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
int intel_vvp_vfw_set_inter_line_offset(intel_vvp_vfw_instance* instance, uint32_t inter_line_offset);

/**
 * \brief Read back the inter-line offset (ie, the maximum size of a line in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the buffer set inter-line offset register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfw_get_inter_line_offset(intel_vvp_vfw_instance* instance);

/**
 * \brief Change the threshold value used to fire the line count based irq
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    line_threshold, new line threshold
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
int intel_vvp_vfw_set_irq_line_threshold(intel_vvp_vfw_instance* instance, uint32_t line_threshold);

/**
 * \brief Read back the threshold value used to fire the line count based irq
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the line_threshold register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized with debug enabled,
 *               line_threshold should be set to 1 or more for the interrupt to trigger
 */
uint32_t intel_vvp_vfw_get_irq_line_threshold(intel_vvp_vfw_instance* instance);


/**
 * \brief Change the broken field behaviour (keep or drop)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \param[in]    overwrite_broken, true to ignore/overwwrite fields that are marked as broken
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
int intel_vvp_vfw_overwrite_broken_fields(intel_vvp_vfw_instance* instance, bool overwrite_broken);


/**
 * \brief Read back the overwrite_broken register that sets the behaviour for broken field (keep or drop)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfw_instance
 * \return       the last value written in the overwrite_broken register if debug is enabled, false if the regiter is not readable
 * \pre          instance is a valid intel_vvp_vfw_instance successfully initialized
 */
bool intel_vvp_vfw_get_overwrite_broken_fields(intel_vvp_vfw_instance* instance);

/**
 * \brief commit any outstanding pending writes from setxxxx commands
 *
 * \param[in]  instance, pointer to the intel_vvp_vfw_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_vfw_instance
 */
int intel_vvp_vfw_commit_writes(intel_vvp_vfw_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_VFW_H__
