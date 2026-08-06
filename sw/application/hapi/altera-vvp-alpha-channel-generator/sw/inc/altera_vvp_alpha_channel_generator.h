/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the altera_vvp_alpha_channel_generator_instance and associated functions
 *
 * Driver for the Video & Vision Processing Alpha Channel Generator Altera FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see altera_vvp_alpha_channel_generator.h
 * \see altera_vvp_alpha_channel_generator_regs.h
 */
#ifndef __ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_H__
#define __ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "altera_vvp_alpha_channel_generator_regs.h"

#define ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_PRODUCT_ID                           0x0257u              ///< Alpha Channel Genertator product ID
#define ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_MIN_SUPPORTED_REGMAP_VERSION         0x0D                 ///< Minimum supported register map version
#define ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_MAX_SUPPORTED_REGMAP_VERSION         0x0D                 ///< Maximum supported register map version

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

#define ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< 1D LUT register read function
#define ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< 1D LUT register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kAlteraVvpAlphaChannelGeneratorRegMapVersionErr = -100,
    kAlteraVvpAlphaChannelGeneratorParameterErr = -101,
    kAlteraVvpAlphaChannelGeneratorValueErr = -102,
    kAlteraVvpAlphaChannelGeneratorPointerErr = -103,
    kAlteraVvpAlphaChannelGeneratorOutOfBoundsErr = -106,
} eAlteraVvpAlphaChannelGeneratorErrors;

typedef struct altera_vvp_alpha_channel_generator_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    uint32_t num_LUT_entries;
    uint32_t config_reg;
} altera_vvp_alpha_channel_generator_instance;


/**
 * \brief Initialize an ip instance
 *
 * IP initialization. The instance is not fully initialized and the application should not use
 * it further if returning a non-zero error code.
 *
 * \param[in]    instance - pointer to the altera_vvp_alpha_channel_generator_instance software driver instance structure
 * \param[in]    base - base address of the register map
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int altera_vvp_alpha_channel_generator_init(altera_vvp_alpha_channel_generator_instance* instance, intel_vvp_core_base base);

/**
 * \brief Writes the config register
 *
 * \param[in]  instance, pointer to the initialized altera_vvp_alpha_channel_generator_instance
 * \param[in]  pixel_start, pixel location to start incrementing LUT read address 
 * \param[in]  pixel_stop, pixel location to stop incrementing LUT read address 
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid altera_vvp_alpha_channel_generator_instance that was successfully initialized
 */
int altera_vvp_alpha_channel_generator_set_config(altera_vvp_alpha_channel_generator_instance* instance, uint16_t pixel_start, uint16_t pixel_stop);

/**
 * \brief Write into a single entry to the lookup table
 *
 * \param[in]  instance, pointer to the initialized altera_vvp_alpha_channel_generator_instance
 * \param[in]  addr, LUT address of to write to
 * \param[in]  alpha_odd, the alpha value to be written to the odd alpha LUT entry
 * \param[in]  alpha_even, the alpha value to be written to the even alpha LUT entry
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kAlteraVvpAlphaChannelGeneratorOutOfBoundsErr if address is out of range
 * \pre        instance is a valid altera_vvp_alpha_channel_generator_instance that was successfully initialized
 */
int altera_vvp_alpha_channel_generator_write_data(altera_vvp_alpha_channel_generator_instance* instance, uint32_t addr, uint16_t odd_alpha, uint16_t even_alpha );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_H__ */
