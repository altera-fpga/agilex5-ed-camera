/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the altera_vvp_roi_instance and associated functions
 *
 * Driver for the Video & Vision Processing Alpha Channel Generator Altera FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see altera_vvp_roi.h
 * \see altera_vvp_roi_regs.h
 */
#ifndef __ALTERA_VVP_ROI_H__
#define __ALTERA_VVP_ROI_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "altera_vvp_roi_regs.h"

#define ALTERA_VVP_ROI_PRODUCT_ID                           0x0258u              ///< Alpha Channel Genertator product ID
#define ALTERA_VVP_ROI_MIN_SUPPORTED_REGMAP_VERSION         0x0D                 ///< Minimum supported register map version
#define ALTERA_VVP_ROI_MAX_SUPPORTED_REGMAP_VERSION         0x0D                 ///< Maximum supported register map version

/**
 * \brief Define for a Nios register read access on a vvp_core instance.
 *
 * \param[in]  instance, pointer to a intel_vvp_core_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \return     the 32-bit value read
 * \pre        reg must be a valid register offset for the given vvp_core instance
 */
#define ALTERA_VVP_CORE_REG_IORD(instance, reg)          IORD( ( (intel_vvp_core_instance*)(instance) )->base, (reg))

/**
 * \brief Define for a Nios register write access on a vvp_core instance.
 *
 * \param[in]  instance, pointer to a intel_vvp_core_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \param[in]  value, the 32-bit value to write
 * \pre        reg must be a valid register offset for the given vvp_core instance
 */
#define ALTERA_VVP_CORE_REG_IOWR(instance, reg, value)   IOWR( ( (intel_vvp_core_instance*)(instance) )->base, (reg), (value))

#define ALTERA_VVP_ROI_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< 1D LUT register read function
#define ALTERA_VVP_ROI_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< 1D LUT register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kAlteraVvpROIRegMapVersionErr = -100,
    kAlteraVvpROIParameterErr = -101,
    kAlteraVvpROIValueErr = -102,
    kAlteraVvpROIPointerErr = -103,
    kAlteraVvpROIOutOfBoundsErr = -106,
} eAlteraVvpROIErrors;

// Normalized region of interest
typedef struct
{
    float _x;
    float _y;
    float _width;
    float _height;
} altera_vvp_roi;

typedef struct altera_vvp_roi_instance_s
{
    intel_vvp_core_instance core_instance;          ///< Base intel_vvp_core_instance
    uint32_t _control_reg;
    bool _enable;
    uint32_t _width;
    uint32_t _height;
    altera_vvp_roi _region_of_interest;             ///< Region of interest to highlight
} altera_vvp_roi_instance;

/**
 * \brief Initialize an ip instance
 *
 * IP initialization. The instance is not fully initialized and the application should not use
 * it further if returning a non-zero error code.
 *
 * \param[in]    instance - pointer to the altera_vvp_roi_instance software driver instance structure
 * \param[in]    base - base address of the register map
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int altera_vvp_roi_init(altera_vvp_roi_instance* instance, intel_vvp_core_base base);

/**
 * \brief Write into a single entry to the lookup table
 *
 * \param[in]  instance, pointer to the initialized altera_vvp_roi_instance
 * \param[in]  enable, true=enabled, false=disabled
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kAlteraVvpROIOutOfBoundsErr if address is out of range
 * \pre        instance is a valid altera_vvp_roi_instance that was successfully initialized
 */
int altera_vvp_roi_set_enable(altera_vvp_roi_instance* instance, bool enable);

/**
 * \brief Write into a single entry to the lookup table
 *
 * \param[in]  instance, pointer to the initialized altera_vvp_roi_instance
 * \param[in]  mode, shift mode
 *             "00" = 1 bit right shift on pixel values
 *             "01" = 2 bit right shift on pixel values
 *             "10" = 3 bit right shift on pixel values
 *             "11" = 4 bit right shift on pixel values
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kAlteraVvpROIOutOfBoundsErr if address is out of range
 * \pre        instance is a valid altera_vvp_roi_instance that was successfully initialized
 */
int altera_vvp_roi_set_mode(altera_vvp_roi_instance* instance, uint8_t mode);

/**
 * \brief Set resolution to use for normalised region of interest
 *
 * \param[in]  instance, pointer to the initialized altera_vvp_roi_instance
 * \param[in]  width, the horizontal resolution of the video
 * \param[in]  height, the vertical resolution of the video
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kAlteraVvpROIOutOfBoundsErr if address is out of range
 * \pre        instance is a valid altera_vvp_roi_instance that was successfully initialized
 */
int altera_vvp_roi_set_resolution(altera_vvp_roi_instance* instance, uint32_t width, uint32_t height);

/**
 * \brief Writes the horizontal and vertical region of interest registers
 *
 * \param[in]  instance, pointer to the initialized altera_vvp_roi_instance
 * \param[in]  roi, region of interest to highlight
 * 
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid altera_vvp_roi_instance that was successfully initialized
 */
int altera_vvp_roi_set_roi(altera_vvp_roi_instance* instance, altera_vvp_roi roi);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __ALTERA_VVP_ROI_H__ */
