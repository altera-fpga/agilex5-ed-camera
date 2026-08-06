/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief  Common definitions for an intel_vvp_core_instance and associated functions
 *
 * The base definition of an intel_vvp_core_instance structure (derived for each
 * core) and the functions to access/modify it
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */
    
#ifndef __INTEL_VVP_CORE_H__
#define __INTEL_VVP_CORE_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core_io.h"

#define INTEL_VVP_VENDOR_ID 0x6AF7u   ///< IntelFPGA vendor id

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * \brief The base definition for an intel_vvp_core_instance.
 * 
 * This is the base definition for an intel_vvp_core_instance. It contains the fields that are shared
 * by all intel_vvp_core instances.
 */
typedef struct intel_vvp_core_instance_s
{
    intel_vvp_core_base base;          ///< accessor object, it partially defines how register accesses can be done \see intel_vvp_core_io.h
    uint16_t vendor_id;                ///< instance vendor_id, compile-time constant for the hardware and set at initialization
    uint16_t product_id;               ///< instance product_id, compile-time constant for the hardware and set at initialization
    uint8_t  qpds_major;               ///< instance major version, compile-time constant for the hardware and set at initialization
    uint8_t  qpds_update;              ///< instance update version, compile-time constant for the hardware and set at initialization
    uint8_t  qpds_patch;               ///< instance patch version, compile-time constant for the hardware and set at initialization
    uint8_t  regmap_version;           ///< instance register map version, compile-time constant for the hardware and set at initialization
} intel_vvp_core_instance;

typedef enum {
    kIntelVvpCoreOk          = 0,
    kIntelVvpCoreVidErr      = -1,
    kIntelVvpCorePidErr      = -2,
    kIntelVvpCoreInstanceErr = -3,
    kIntelVvpCoreNullPtrErr  = -4
} eIntelVvpCoreErrors;

/**
 * \brief Initialise a vvp_core instance
 * 
 * Initialization function for a VVP core instance, attempts accesses to the core
 * to initialize the fields of the intel_vvp_core_instance instance
 * 
 * \private
 * \param[in]  instance, the intel_vvp_core_instance to initialize
 * \param[in]  base, the accessor for the core (on Nios, this is simply the base address of the core)
 * \param[in]  expected_product_id, the expected product_id for hte core
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *             kIntelVvpCorePidErr if the product_id does not match with the expectation
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             The initialization stops early if the vendor ID/product ID are not a match
 *             and the second register, containing version numbers, is not read
 * \pre        base is a proper accessor for an Intel VVP core
 */
eIntelVvpCoreErrors intel_vvp_core_init(intel_vvp_core_instance *instance, intel_vvp_core_base base, uint16_t expected_product_id);

/**
 * \brief Query the vendor_id of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the vendor_id field in the intel_vvp_core_instance instance
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint16_t intel_vvp_core_get_vendor_id(void *instance);

/**
 * \brief Query the product_id of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the product_id field in the intel_vvp_core_instance instance
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint16_t intel_vvp_core_get_product_id(void *instance);

/**
 * \brief Query the QPDS major version of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the qpds_major field in the intel_vvp_core_instance instance
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_qpds_major(void *instance);

/**
 * \brief Query the QPDS update (minor) version of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the qpds_update field in the intel_vvp_core_instance instance
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_qpds_update(void *instance);

/**
 * \brief Query the QPDS patch (build) version of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the qpds_patch field in the intel_vvp_core_instance instance
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_qpds_patch(void *instance);

/**
 * \brief Query the register map version number of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the regmap_version field in the intel_vvp_core_instance instance
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_register_map_version(void *instance);

/**
 * \brief Query the image_info width register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the value returned from a read to the image_info width register
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint32_t intel_vvp_core_get_img_info_width(void *instance);

/**
 * \brief Query the image_info height register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the value returned from a read to the image_info height register
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint32_t intel_vvp_core_get_img_info_height(void *instance);

/**
 * \brief Query the image_info interlace register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the value returned from a read to the image_info interlace register
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_img_info_interlace(void *instance);

/**
 * \brief Query the image_info bps_code register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[out] fp_flag, the floating point flag value returned from a read to the image_info bps_code register
 * \param[out] bps_val, the bps value returned from a read to the image_info bps_code register
 * \return     kIntelVvpCoreInstanceErr in case of errors (output parameters are unchanged), kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_get_img_info_bps_code(void *instance, bool *fp_flag, uint8_t *bps_val);

/**
 * \brief Query the image_info colorspace register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the value returned from a read to the image_info colorspace register
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_img_info_colorspace(void *instance);

/**
 * \brief Query the image_info subsampling register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the value returned from a read to the image_info subsampling register
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_img_info_subsampling(void *instance);

/**
 * \brief Query the image_info cositing register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \return     the value returned from a read to the image_info cositing register
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
uint8_t  intel_vvp_core_get_img_info_cositing(void *instance);

/**
 * \brief Write the image_info width register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  width, the new width
 * \return     kIntelVvpCoreInstanceErr in case of errors (output parameters are unchanged), kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_width(void *instance, uint32_t width);

/**
 * \brief Write the image_info height register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  height, the new height
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_height(void *instance, uint32_t height);

/**
 * \brief Write the image_info interlace register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  interlace, the new interlace
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_interlace(void *instance, uint8_t interlace);

/**
 * \brief Write the image_info bps_code register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  fp_flag, the new floating pint flag
 * \param[in]  bps_val, the new bps value
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_bps_code(void *instance, uint8_t fp_flag, uint8_t bps_val);

/**
 * \brief Write the image_info colorspace register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  colorspace, the new colorspace
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_colorspace(void *instance, uint8_t colorspace);

/**
 * \brief Write the image_info subsampling register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  subsampling, the new subsampling
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_subsampling(void *instance, uint8_t subsampling);

/**
 * \brief Write the image_info cositing register of an instance
 * 
 * \param[in]  instance, an intel_vvp_core instance. Usually, this an instance from a structure derived from intel_vvp_core
 * \param[in]  cositing, the new cositing
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_core_instance and was successfully initialized 
 */
eIntelVvpCoreErrors intel_vvp_core_set_img_info_cositing(void *instance, uint8_t cositing);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_CLIPPER_H__ */