/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
/**
 * \brief Definition of the intel_vvp_vfr_instance and associated functions
 *
 * Driver for the Video & Vision Processing Frame Reader Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_vfr_regs.h
 */

#ifndef __INTEL_VVP_VFR_H__
#define __INTEL_VVP_VFR_H__

//#include <stddef.h>
//#include <stdbool.h>
//#include <stdint.h>
#include <linux/types.h>

#include "intel_vvp_core.h"
#include "intel_vvp_vfr_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_VFR_PRODUCT_ID                           0x024Au              ///< Frame reader product ID
#define INTEL_VVP_VFR_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_VFR_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_VFR_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< frame reader register read function
#define INTEL_VVP_VFR_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< frame reader register write function

typedef enum {
    kIntelVvpVfrRegMapVersionErr = -100,
} eIntelVvpVfrErrors;

typedef enum {
    kIntelVvpVfrPerfectPacking =  INTEL_VVP_VFR_PERFECT_PACKING,
    kIntelVvpVfrColorPacking   =  INTEL_VVP_VFR_COLOR_PACKING,
    kIntelVvpVfrPixelPacking   =  INTEL_VVP_VFR_PIXEL_PACKING,
    kIntelVvpVfrInvalidPacking = -1,
} eIntelVvpVfrPacking;

typedef enum {
    kIntelVvpVfrStop           =  INTEL_VVP_VFR_STOP,
    kIntelVvpVfrFreeRunning    =  INTEL_VVP_VFR_FREE_RUNNING,
    kIntelVvpVfrFsync          =  INTEL_VVP_VFR_FSYNC_RUNNING,
    kIntelVvpVfrSingleShot     =  INTEL_VVP_VFR_SINGLE_SHOT,
    kIntelVvpVfrInvalidRunMode = -1,
} eIntelVvpVfrRunMode;

typedef enum {
    kIntelVvpVfrSingleSet         =  INTEL_VVP_VFR_SINGLE_SET_BUFFER_MODE,
    kIntelVvpVfrAllSets           =  INTEL_VVP_VFR_ALL_SETS_BUFFER_MODE,
    kIntelVvpVfrInvalidBufferMode = -1,
} eIntelVvpVfrBufferMode;

typedef enum {
    kIntelVvpVfrFsyncBoth         =  INTEL_VVP_VFR_FSYNC_ON_BOTH_EDGES,
    kIntelVvpVfrFsyncRising       =  INTEL_VVP_VFR_FSYNC_ON_RISING_EDGE,
    kIntelVvpVfrInvalidFsyncMode  = -1,
} eIntelVvpVfrFsyncMode;

typedef struct intel_vvp_vfr_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool lite_mode;
    bool debug_enabled;
    uint32_t max_height;
    uint32_t max_width;
    uint8_t max_buffer_sets;
    uint8_t bps;
    uint8_t num_color_planes;
    uint8_t pip;
    eIntelVvpVfrPacking packing;
} intel_vvp_vfr_instance;

/**
 * \brief Initialise a frame reader instance
 *
 * Initialization function for a VVP frame reader instance.
 * Attempts to initialize the fields of the frame reader and its base core
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the frame reader product id (0x0237)
 *               kIntelVvpVfrRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 * \remarks      If already running, the frame reader is stopped during initialization. It must be
 *               enabled, or re-enabled to output any data
 */
int intel_vvp_vfr_init(intel_vvp_vfr_instance* instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a frame reader instance
 *
 * \param[in]  instance, an intel_vvp_vfr_instance
 * \return the lite_mode field in the intel_vvp_vfr_instance
 */
bool intel_vvp_vfr_get_lite_mode(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the debug_enabled field in the intel_vvp_vfr_instance
 */
bool intel_vvp_vfr_get_debug_enabled(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the max_height parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the max_height field in the intel_vvp_vfr_instance
 */
uint32_t intel_vvp_vfr_get_max_height(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the max_width parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the max_width field in the intel_vvp_vfr_instance
 */
uint32_t intel_vvp_vfr_get_max_width(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the max_buffer_sets parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the max_buffer_sets field in the intel_vvp_vfr_instance
 */
uint8_t intel_vvp_vfr_get_max_buffer_sets(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the bits per color sample parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the bps field in the intel_vvp_vfr_instance
 */
uint8_t intel_vvp_vfr_get_bps(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the number of color planes parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the num_color_planes field in the intel_vvp_vfr_instance
 */
uint8_t intel_vvp_vfr_get_number_of_color_planes(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the pixels_in_parallel parameter of a frame reader instance
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the pip field in the intel_vvp_vfr_instance
 */
uint8_t intel_vvp_vfr_get_pixels_in_parallel(intel_vvp_vfr_instance* instance);

/**
 * \brief Fetch and return the packing of video data in memory
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the packing algorithm used to store the video in memory
 *               kIntelVvpVfrPerfectPacking, memory words fully utilized, color samples may be split over consecutive memory words
 *               kIntelVvpVfrColorPacking, color samples are not split over consecutive memory words, up to bits_per_sample bits
 *                                         may be lost per memory word
 *               kIntelVvpVfrPixelPacking, pixels are not split over consecutive memory words, up to bits_per_sample * num_color_planes
 *               bits may be lost per memory word
 *               kIntelVvpVfrInvalidPacking, invalid instance
 */
eIntelVvpVfrPacking intel_vvp_vfr_get_mem_word_packing(intel_vvp_vfr_instance* instance);

/**
 * \brief Query the running bit of the frame reader
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the running bit of the status register
 * \remarks      true while processing a frame, false in-between frames
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized
 */
bool intel_vvp_vfr_is_running(intel_vvp_vfr_instance* instance);

/**
 * \brief Determine if the frame reader instance has any pending writes that have NOT been commited
 *
 * \param[in]  instance, pointer to the intel_vvp_vfr_instance
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_vfr_instance
 */
bool intel_vvp_vfr_get_commit_status(intel_vvp_vfr_instance* instance);

/**
 * \brief Get the frame reader status
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the status register of the frame reader
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized
 */
uint32_t intel_vvp_vfr_get_status(intel_vvp_vfr_instance* instance);

/**
 * \brief Get the address of the last buffer read
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the value in the last_buffer_read register of the frame reader
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized
 */
uint32_t intel_vvp_vfr_get_last_buffer_read_addr(intel_vvp_vfr_instance* instance);

/**
 * \brief Change the number of buffer sets in use
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    num_sets, number of sets to use
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized
 *               num_sets is a valid number of sets: 1 <= num_sets <= intel_vvp_vfr_get_max_buffer_sets(instance)
 */
int intel_vvp_vfr_set_num_buffer_sets(intel_vvp_vfr_instance* instance, uint8_t num_sets);

/**
 * \brief Read the number of buffer sets in use
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the last value written in the number of buffer sets register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_num_buffer_sets(intel_vvp_vfr_instance* instance);

/**
 * \brief Change the use mode of buffer sets (one or all)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    buffer_mode, buffer mode, either kIntelVvpVfrSingleSet to use only one set or
 *               kIntelVvpVfrAllSets to use all of them in the range [0..intel_vvp_vfr_get_num_buffer_sets(instance)-1]
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and buffer_mode is valid
 */
int intel_vvp_vfr_set_buffer_mode(intel_vvp_vfr_instance* instance, eIntelVvpVfrBufferMode buffer_mode);

/**
 * \brief Read the use mode of buffer sets (one or all)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the last value written in the buffer mode register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and compiled with debug enabled
 */
eIntelVvpVfrBufferMode intel_vvp_vfr_get_buffer_mode(intel_vvp_vfr_instance* instance);


/**
 * \brief Change the starting buffer set
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    starting_buffer_set, the indice for the starting buffer set (mode==kIntelVvpVfrAllSets) or the indice of the
 *               active buffer set (mode==kIntelVvpVfrSingleSet)
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, a valid starting_buffer_set value
 *               in the range [0..intel_vvp_vfr_get_num_buffer_sets(instance)-1] should be used or a different starting
 *               set will be used
 */
int intel_vvp_vfr_set_starting_buffer_set(intel_vvp_vfr_instance* instance, uint8_t starting_buffer_set);

/**
 * \brief Read the starting buffer set
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the last value written in the starting buffer set register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_starting_buffer_set(intel_vvp_vfr_instance* instance);

/**
 * \brief Change the run_mode
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    run_mode, new run mode
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and run_mode is valid
 */
int intel_vvp_vfr_set_run_mode(intel_vvp_vfr_instance* instance, eIntelVvpVfrRunMode run_mode);

/**
 * \brief Read the run_mode
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the last value written in the run_mode register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and compiled with debug enabled
 */
eIntelVvpVfrRunMode intel_vvp_vfr_get_run_mode(intel_vvp_vfr_instance* instance);


/**
 * \brief Change the fsync trigger behaviour
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    sync_mode, fsync trigger mode, either kIntelVvpVfrFsyncBoth to start reading a field on either edge or
 *               kIntelVvpVfrFsyncRising to start reading a field on the positive edge and ignore the negative edge
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and sync_mode is valid
 */
int intel_vvp_vfr_set_fsync_mode(intel_vvp_vfr_instance* instance, eIntelVvpVfrFsyncMode sync_mode);

/**
 * \brief Read the fsync trigger behaviour
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \return       the last value written in the fsync pulse mode register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and compiled with debug enabled
 */
eIntelVvpVfrFsyncMode intel_vvp_vfr_get_fsync_mode(intel_vvp_vfr_instance* instance);


/**
 * \brief Change a buffer set base address
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    base_addr, new base address
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 */
int intel_vvp_vfr_set_bufset_base_addr(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t base_addr);

/**
 * \brief Read a buffer set base address
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set base address register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfr_get_bufset_base_addr(intel_vvp_vfr_instance* instance, uint8_t bufset);


/**
 * \brief Change the number of buffers in the set
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    num_buffers, new number of buffers
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 * \remarks      Please check user guide for upper-limit on num_buffers (value is truncated)
 */
int intel_vvp_vfr_set_bufset_num_buffers(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t num_buffers);

/**
 * \brief Read the number of buffers in the set
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set number of buffers register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfr_get_bufset_num_buffers(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set inter-buffer offset (ie, the size of a buffer in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    inter_buffer_offset, new offset between buffers
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 */
int intel_vvp_vfr_set_bufset_inter_buffer_offset(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t inter_buffer_offset);

/**
 * \brief Read a buffer set inter-buffer offset (ie, the size of a buffer in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set inter-buffer offset register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfr_get_bufset_inter_buffer_offset(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set inter-line offset (ie, the maximum size of a line in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    inter_line_offset, new offset between adjacent lines  in bytes
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 */
int intel_vvp_vfr_set_bufset_inter_line_offset(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t inter_line_offset);

/**
 * \brief Read a buffer set inter-line offset (ie, the maximum size of a line in bytes)
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set inter-line offset register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfr_get_bufset_inter_line_offset(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set image height
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    height, new height
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 */
int intel_vvp_vfr_set_bufset_height(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t height);

/**
 * \brief Read a buffer set image height
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set image height register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfr_get_bufset_height(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set image width
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    width, new width
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 */
int intel_vvp_vfr_set_bufset_width(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t width);

/**
 * \brief Read a buffer set image width
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set inter-buffer offset register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint32_t intel_vvp_vfr_get_bufset_width(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set interlace nibble
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    interlace, new interlace nibble
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]
 */
int intel_vvp_vfr_set_bufset_interlace(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t interlace);

/**
 * \brief Read a buffer set interlace mibble
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set interlace register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_bufset_interlace(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set colorspace
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    interlace, new interlace nibble
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]. This call
 *               has no effect if not parameterized with full variant
 */
int intel_vvp_vfr_set_bufset_colorspace(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t colorspace);

/**
 * \brief Read a buffer set colorspace
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set colorspace register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_bufset_colorspace(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set subsampling
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    subsampling, new subsampling
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1].
 */
int intel_vvp_vfr_set_bufset_subsampling(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t subsampling);

/**
 * \brief Read a buffer set subsampling
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set subsampling register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_bufset_subsampling(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set cositing
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    cositing, new cositing
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]. This call
 *               has no effect if not parameterized with full variant
 */
int intel_vvp_vfr_set_bufset_cositing(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t cositing);

/**
 * \brief Read a buffer set cositing value
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set cositing register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_bufset_cositing(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set bps
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    bps, new bits per color sample value
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]. This call
 *               has no effect if not parameterized with full variant
 */
int intel_vvp_vfr_set_bufset_bps(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t bps);

/**
 * \brief Read a buffer set bps value
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set bps register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint8_t intel_vvp_vfr_get_bufset_bps(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief Change a buffer set field_count start value
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to parameterize
 * \param[in]    field_count, new field_count value for buffer 0 of the set
 * \return       kIntelVvpCoreInstanceErr in case of errors
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized and bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1]. This call
 *               has no effect if not parameterized with full variant
 */
int intel_vvp_vfr_set_bufset_field_count(intel_vvp_vfr_instance* instance, uint8_t bufset, uint16_t field_count);

/**
 * \brief Read a buffer set field_count start value
 *
 * \param[in]    instance, pointer to the intel_vvp_vfr_instance
 * \param[in]    bufset, the id of the buffer set to query
 * \return       the last value written in the buffer set field_count register if debug is enabled
 * \pre          instance is a valid intel_vvp_vfr_instance successfully initialized, bufset
 *               is in the range [0..intel_vvp_vfr_get_max_buffer_sets(instance)-1] and the instance was
 *               compiled with debug enabled
 */
uint16_t intel_vvp_vfr_get_bufset_field_count(intel_vvp_vfr_instance* instance, uint8_t bufset);

/**
 * \brief commit any outstanding pending writes from setxxxx commands
 *
 * \param[in]  instance, pointer to the intel_vvp_vfr_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_vfr_instance
 */
int intel_vvp_vfr_commit_writes(intel_vvp_vfr_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_VFR_H__
