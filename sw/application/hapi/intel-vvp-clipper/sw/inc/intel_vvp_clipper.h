/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_clipper_instance and associated functions
 *
 * Driver for the Video & Vision Processing Clipper Intel FPGA IP
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_clipper_regs.h
 */
#ifndef __INTEL_VVP_CLIPPER_H__
#define __INTEL_VVP_CLIPPER_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_CLIPPER_PRODUCT_ID                           0x022Du              ///< Clipper product ID
#define INTEL_VVP_CLIPPER_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_CLIPPER_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_CLIPPER_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Clipper register read function
#define INTEL_VVP_CLIPPER_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Clipper register write function

typedef enum {
    kIntelVvpClipperRegMapVersionErr = -100
} eIntelVvpClipperErrors;

/**
 * \brief The two possible values for the clipping mode
 * 
 * This is the definition of the clipping mode. In offset mode, the clipping is specified as a set of four offsets (top,left,bottom,right)
 * where the bottom-right offset is a coordinate relative to the bottom-right corner of the image. In rectangle mode, the clipping is specified
 * with a top-left offset and the dimension of the clipped area.
 */
typedef enum {
    kIntelVvpOffsetClipping    = 0,
    kIntelVvpRectangleClipping = 1,
    kIntelVvpInvalidClipping   = 2
} eIntelVvpClippingMode;

/**
 * \brief The definition of an intel_vvp_clipper_instance.
 * 
 * This is the definition for an intel_vvp_clipper_instance. It starts with an intel_vvp_core_instance
 * and can be used as such in generic functions
 */
typedef struct intel_vvp_clipper_instance_s
{
    // intel VVP base instance
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    
    // Compile-time parameterization
    bool                       lite_mode;            ///< instance lite mode parameter, compile-time constant for the hardware and set at initialization
    bool                       debug_enabled;        ///< instance debug_enabled parameter, compile-time constant for the hardware and set at initialization
    eIntelVvpClippingMode      clip_mode;            ///< instance clip_mode parameter, compile-time constant for the hardware and set at initialization
} intel_vvp_clipper_instance;

/**
 * \brief Initialise a clipper instance
 * 
 * Initialization function for a VVP clipper instance, attempts accesses to the core
 * to initialize the fields of the clipper instance and its base core instance
 * 
 * \param[in]    instance, the intel_vvp_clipper_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios, this is simply the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the clipper product id (0x022D)
 *               kIntelVvpClipperRegMapVersionErr if the register map is not supported
 *               The initialization stops early if the vendor ID/product ID are not a match
 *               and the second register, containing version numbers, is not read
 * \pre          base is a proper accessor for an Intel VVP Clipper IP core
 */
int intel_vvp_clipper_init(intel_vvp_clipper_instance *instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a clipper instance
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the lite_mode field in the intel_vvp_clipper_instance
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
bool intel_vvp_clipper_get_lite_mode(intel_vvp_clipper_instance *instance);

/**
 * \brief Query the debug_enabled parameter of a clipper instance
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the debug_enabled field in the intel_vvp_clipper_instance
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
bool intel_vvp_clipper_get_debug_enabled(intel_vvp_clipper_instance *instance);

/**
 * \brief Query the clipping_mode parameter of a clipper instance
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the clipping_mode field in the intel_vvp_clipper_instance
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
eIntelVvpClippingMode intel_vvp_clipper_get_clipping_mode(intel_vvp_clipper_instance *instance);

/**
 * \brief get running status of the clipper instance
 *
 * \param[in]  instance, pointer to the intel_vvp_clipper_instance 
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
bool intel_vvp_clipper_is_running(intel_vvp_clipper_instance *instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 * 
 * \param[in]    instance, pointer to the intel_vvp_clipper_instance 
 * \return       true if there are outstanding writes
 * \pre          instance is a valid intel_vvp_clipper_instance parameterized in full mode and was successfully initialized 
 */
bool intel_vvp_clipper_get_commit_status(intel_vvp_clipper_instance* instance);

/**
 * \brief Read the status register
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
uint8_t intel_vvp_clipper_get_status(intel_vvp_clipper_instance *instance);

/**
 * \brief Read the left offset register
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the left offset register
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
uint16_t intel_vvp_clipper_get_left_offset(intel_vvp_clipper_instance *instance);

/**
 * \brief Read the top offset register
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the top offset register
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
uint16_t intel_vvp_clipper_get_top_offset(intel_vvp_clipper_instance *instance);

/**
 * \brief Write the left offset register
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \param[in]  left_offset, the new left offset
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
int intel_vvp_clipper_set_left_offset(intel_vvp_clipper_instance *instance, uint16_t left_offset);

/**
 * \brief Write the top offset register
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \param[in]  top_offset, the new top offset
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_clipper_instance and was successfully initialized 
 */
int intel_vvp_clipper_set_top_offset(intel_vvp_clipper_instance *instance, uint16_t top_offset);

/**
 * \brief Read the right offset register in offset clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the right offset register
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for offset clipping
 *             and debug enabled
 */
uint16_t intel_vvp_clipper_get_right_offset(intel_vvp_clipper_instance *instance);

/**
 * \brief Read the bottom offset register in offset clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the bottom offset register
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for offset clipping
 *             and debug enabled
 */
uint16_t intel_vvp_clipper_get_bottom_offset(intel_vvp_clipper_instance *instance);

/**
 * \brief Read the four offset registers in offset clipping mode
 * 
 * \param[in]   instance, an intel_vvp_clipper_instance
 * \param[out]  left_offset,the value returned from a read to the left offset register
 * \param[out]  top_offset, the value returned from a read to the top offset register
 * \param[out]  right_offset, the value returned from a read to the right offset register
 * \param[out]  bottom_offset, the value returned from a read to the bottom offset register
 * \return      kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre         instance is a valid intel_vvp_clipper_instance parameterized for offset clipping
 *              and debug enabled
 */
int intel_vvp_clipper_get_clip_offsets(intel_vvp_clipper_instance *instance,
                                        uint16_t *left_offset, uint16_t *top_offset,
                                        uint16_t *right_offset, uint16_t *bottom_offset);

/**
 * \brief Write the right offset register in offset clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \param[in]  right_offset, the new right offset
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre         instance is a valid intel_vvp_clipper_instance parameterized for offset clipping
 */
int intel_vvp_clipper_set_right_offset(intel_vvp_clipper_instance *instance, uint16_t right_offset);

/**
 * \brief Write the bottom offset register in offset clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \param[in]  bottom_offset, the new bottom offset
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre         instance is a valid intel_vvp_clipper_instance parameterized for offset clipping
 */
int intel_vvp_clipper_set_bottom_offset(intel_vvp_clipper_instance *instance, uint16_t bottom_offset);

/**
 * \brief Write the four offset registers in offset clipping mode
 * 
 * \param[in]   instance, an intel_vvp_clipper_instance
 * \param[in]   left_offset, the new left offset
 * \param[in]   top_offset, the new top offset
 * \param[in]   right_offset, the new right offset
 * \param[in]   bottom_offset, the new bottom offset
 * \return      kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre         instance is a valid intel_vvp_clipper_instance parameterized for offset clipping
 */
int intel_vvp_clipper_set_clip_offsets(intel_vvp_clipper_instance *instance,
                                        uint16_t left_offset, uint16_t top_offset,
                                        uint16_t right_offset, uint16_t bottom_offset);

/**
 * \brief Read the clipping_width register in rectangle clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the clipping width register
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for rectangle
 *             clipping and debug enabled
 */
uint32_t intel_vvp_clipper_get_clip_width(intel_vvp_clipper_instance *instance);

/**
 * \brief Read the clipping_height register in rectangle clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \return     the value returned from a read to the clipping height register
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for rectangle
 *             clipping and debug enabled
 */
uint32_t intel_vvp_clipper_get_clip_height(intel_vvp_clipper_instance *instance);

/**
 * \brief Read the four clipping registers in rectangle clipping mode
 * 
 * \param[in]   instance, an intel_vvp_clipper_instance
 * \param[out]  left_offset,the value returned from a read to the left offset register
 * \param[out]  top_offset, the value returned from a read to the top offset register
 * \param[out]  clip_width, the value returned from a read to the clipping width register
 * \param[out]  clip_height, the value returned from a read to the clipping height register
 * \return      kIntelVvpCoreInstanceErr or kIntelVvpCoreNullPtrErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre         instance is a valid intel_vvp_clipper_instance parameterized for rectangle
 *              clipping and debug enabled
 */
int intel_vvp_clipper_get_clip_area(intel_vvp_clipper_instance *instance,
                                     uint16_t *left_offset, uint16_t *top_offset,
                                     uint32_t *clip_width, uint32_t *clip_height);

/**
 * \brief Write the clipping_width register in rectangle clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \param[in]  clip_width, the new width for the clipped area
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for rectangle clipping
 */
int intel_vvp_clipper_set_clip_width(intel_vvp_clipper_instance *instance, uint32_t clip_width);

/**
 * \brief Write the clipping_height register in rectangle clipping mode
 * 
 * \param[in]  instance, an intel_vvp_clipper_instance
 * \param[in]  clip_width, the new height for the clipped area
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for rectangle clipping
 */
int intel_vvp_clipper_set_clip_height(intel_vvp_clipper_instance *instance, uint32_t clip_height);

/**
 * \brief Write the four clipping registers in rectangle clipping mode
 * 
 * \param[in]   instance, an intel_vvp_clipper_instance
 * \param[in]   left_offset, the new left offset
 * \param[in]   top_offset, the new top offset
 * \param[in]   clip_width, the new width for the clipped area
 * \param[in]   clip_height, the new height for the clipped area
 * \return      kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized for rectangle clipping
 */
int intel_vvp_clipper_set_clip_area(intel_vvp_clipper_instance *instance,
                                     uint16_t left_offset, uint16_t top_offset,
                                     uint32_t clip_width, uint32_t clip_height);

/**
 * \brief commit any outstanding writes from setxxxx commands
 * 
 * \param[in]  instance, pointer to the intel_vvp_clipper_instance 
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_clipper_instance parameterized in full mode
 */
int intel_vvp_clipper_commit_writes(intel_vvp_clipper_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_CLIPPER_H__ */
