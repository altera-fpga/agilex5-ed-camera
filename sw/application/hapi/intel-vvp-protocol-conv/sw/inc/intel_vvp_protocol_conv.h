/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_VVP_PROTOCOL_CONV_H__
#define __INTEL_VVP_PROTOCOL_CONV_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"

#include "intel_vvp_protocol_conv_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_PROTOCOL_CONV_PRODUCT_ID                       0x023Du              ///< Protocol Converter product ID
#define INTEL_VVP_PROTOCOL_CONV_MIN_SUPPORTED_REGMAP_VERSION     1                    ///< Minimum supported register map version
#define INTEL_VVP_PROTOCOL_CONV_MAX_SUPPORTED_REGMAP_VERSION     1                    ///< Maximum supported register map version

#define INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Protocol Converter register read function
#define INTEL_VVP_PROTOCOL_CONV_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Protocol Converter register write function


/**
 * \brief The six possible values for the conversion mode
 *
 * This is the definition of the conversion mode. It indicates the input protocol and the requested output protocol
 */
typedef enum {
    kIntelVvpProtocolConvVipToVvpLite     = INTEL_VVP_PROTOCOL_CONV_VIP_TO_VVP_LITE,       ///< VIP to VVP Lite conversion
    kIntelVvpProtocolConvVvpLiteToVip     = INTEL_VVP_PROTOCOL_CONV_VVP_LITE_TO_VIP,       ///< VVP Lite to VIP conversion
    kIntelVvpProtocolConvVipToVvpFull     = INTEL_VVP_PROTOCOL_CONV_VIP_TO_VVP_FULL,       ///< VIP to VVP Full conversion
    kIntelVvpProtocolConvVvpFullToVip     = INTEL_VVP_PROTOCOL_CONV_VVP_FULL_TO_VIP,       ///< VVP Full to VIP conversion
    kIntelVvpProtocolConvVvpLiteToVvpFull = INTEL_VVP_PROTOCOL_CONV_VVP_LITE_TO_VVP_FULL,  ///< VVP Lite to VVP Full conversion
    kIntelVvpProtocolConvVvpFullToVvpLite = INTEL_VVP_PROTOCOL_CONV_VVP_FULL_TO_VVP_LITE,  ///< VVP Full to VVP Lite conversion
    kIntelVvpProtocolConvInvalid          = -1                                             ///< Invalid conversion mode
} eIntelVvpProtocolConvConversionMode;

typedef enum {
    kIntelVvpProtocolConvRegMapVersionErr = -100
} eIntelVvpProtocolConvErrors;

/**
 * \brief The definition of an intel_vvp_protocol_conv_instance.
 *
 * This is the definition for an intel_vvp_protocol_conv_instance. It starts with an intel_vvp_core_instance
 * and can be used as such in generic functions
 */
typedef struct intel_vvp_protocol_conv_instance_s
{
    // intel VVP base instance
    intel_vvp_core_instance core_instance;

    // Compile-time parameterization
    eIntelVvpProtocolConvConversionMode conv_mode;
    bool          debug_enabled;

    // Run-time register
    uint8_t       ctrl;
} intel_vvp_protocol_conv_instance;

/**
 * \brief Initialise a protocol converter instance
 *
 * Initialization function for a VVP protocol converter instance, attempts accesses to the core
 * to initialize the fields of the protocol converter instance and its base core instance
 *
 * \param[in]    instance, the intel_vvp_protocol_conv_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios, this is simply the base address of the core)
 * \return       kIntelVvpCoreOk in case of success
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the protocol converter product id (0x023C)
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpProtocolConvRegMapVersionErr if the register map is not supported
 *               The initialization stops early if the vendor ID/product ID are not a match
 *               and the second register, containing version numbers, is not read
 * \pre          base is a proper accessor for an Intel VVP Protocol Converter IP core
  * \remarks     The protocol converter is stopped upon successful return and must be started/enabled to produce any data
*/
int intel_vvp_protocol_conv_init(intel_vvp_protocol_conv_instance *instance, intel_vvp_core_base base);

/**
 * \brief Query the conversion_mode parameter of a protocol converter instance
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return the conversion_mode field in the intel_vvp_protocol_conv_instance
 * \pre    instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
eIntelVvpProtocolConvConversionMode intel_vvp_protocol_conv_get_conversion_mode(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Query the debug_enabled parameter of a protocol converter instance
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return the debug_enabled field in the intel_vvp_protocol_conv_instance
 * \pre    instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
bool intel_vvp_protocol_conv_get_debug_enabled(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the status register
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return the value returned from a read to the status register, note that the BROKEN_FIELD bit
 *         is undefined until the FIELD_RECEIVED bit is set
 * \pre    instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
uint8_t intel_vvp_protocol_conv_get_status(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the status register and return the is_running flag
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return whether the is_running flag on the status register is set,
 *         the core is declaring itself as idle in between fields
 * \pre    instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
bool intel_vvp_protocol_conv_is_running(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the status register and return the has_received_field flag
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return whether the has_received_field flag on the status register is set,
 *         the flag goes high after the first field has been processed
 * \pre    instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
#define intel_vvp_protocol_has_received_field intel_vvp_protocol_conv_has_received_field
bool intel_vvp_protocol_conv_has_received_field(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the status register and return the last_field_broken flag
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return whether the last_field_broken flag on the status register is valid and is set,
 *         the flag is set at the end of each frame to indicate whether the input field
 *         matched with expectations (typically, this flags mismatches between the
 *         video data received and the dimensions given by metadata control/information packets
 *         or the dimensions provided through the slave interface by the user.
 *         Note that the last field_broken bit on the status register is in an undefined state
 *         until the first field has been received (the has_received_field bit is set) and
 *         this function will return 0/false in such a case
 * \pre    instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
#define intel_vvp_protocol_was_last_field_broken intel_vvp_protocol_conv_was_last_field_broken
bool intel_vvp_protocol_conv_was_last_field_broken(intel_vvp_protocol_conv_instance *instance);


/**
 * \brief Read the field_count register
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     the value read from the field count register
 * \remarks    With VVP full input, the field_count value is set from the first EOF packet
 *             and is undefined until the first field has been receive. The call will return
 *             0 in this specific case
 * \pre        instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 */
uint16_t intel_vvp_protocol_conv_get_field_count(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the VIP width register
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     the value read from the VIP width register (extracted from the last VIP control packet)
 * \remarks    a default initialization value is returned if no control packet was received
 * \pre        instance is a valid intel_vvp_protocol_conv_instance that converts from VIP to VVP
 */
uint32_t intel_vvp_protocol_conv_get_vip_width(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the VIP height register
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     the value read from the VIP height register (extracted from the last VIP control packet)
 * \remarks    a default initialization value is returned if no control packet was yet received
 * \pre        instance is a valid intel_vvp_protocol_conv_instance that converts from VIP to VVP
 */
uint32_t intel_vvp_protocol_conv_get_vip_height(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Read the VIP interlace register
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     the value read from the VIP interlace register (extracted from the last VIP control packet)
 * \remarks    a default initialization value is returned if no control packet was yet received
 * \pre        instance is a valid intel_vvp_protocol_conv_instance that converts from VIP to VVP
 */
uint8_t intel_vvp_protocol_conv_get_vip_interlace(intel_vvp_protocol_conv_instance *instance);


/**
 * \brief Enable or disable the protocol converter
 *
 * \param[in]  instance, pointer to the intel_vvp_protocol_conv_instance
 * \param[in]  enabled, true to enable the converter
 * \return     errorcode if invalid instance
 * \pre        instance is a valid intel_vvp_protocol_conv_instance successfully initialized
 */
int intel_vvp_protocol_conv_enable(intel_vvp_protocol_conv_instance* instance, bool enabled);

/**
 * \brief Start the core
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \post       Sets the "go" bit on the control register.
 *             This enables the processing of a new frame
 * \pre        instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 * \see        intel_vvp_protocol_conv_is_running()
 */
int intel_vvp_protocol_conv_start(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Stop the core
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \post       Zero the "go" bit on the control register.
 *             This has no immediate effect is the core is currently processing a frame but will
 *             stall the processing before the start of the next frame
 * \pre        instance is a valid intel_vvp_protocol_conv_instance and was successfully initialized
 * \see        intel_vvp_protocol_conv_is_running()
 */
int intel_vvp_protocol_conv_stop(intel_vvp_protocol_conv_instance *instance);

/**
 * \brief Reset the field counter
 *
 * \param[in]  instance, an intel_vvp_protocol_conv_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid or the input is VVP full
 * \post       reset the field counter to 0
 * \pre        instance is a valid intel_vvp_protocol_conv_instance and the input is not VVP Full
 * \see        intel_vvp_protocol_conv_get_field_count()
 */
int intel_vvp_protocol_conv_reset_field_count(intel_vvp_protocol_conv_instance *instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_PROTOCOL_CONV_H__ */
