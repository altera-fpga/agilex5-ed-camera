/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_swi_instance and associated functions
 *
 * Driver for the Video & Vision Processing Switch Intel FPGA IP
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_clipper_regs.h
 */
#ifndef __INTEL_VVP_SWITCH_H__
#define __INTEL_VVP_SWITCH_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    
#define INTEL_VVP_SWITCH_PRODUCT_ID                          0x0235u              ///< Switch product ID
#define INTEL_VVP_SWITCH_MIN_SUPPORTED_REGMAP_VERSION        2                    ///< Minimum supported register map version
#define INTEL_VVP_SWITCH_MAX_SUPPORTED_REGMAP_VERSION        2                    ///< Maximum supported register map version

#define INTEL_VVP_SWITCH_REG_IORD(instance, reg)             INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Switch register read function
#define INTEL_VVP_SWITCH_REG_IOWR(instance, reg, value)      INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Switch register write function

typedef enum {
    kIntelVvpSwitchRegMapVersionErr = -100,
    kIntelVvpSwitchParameterErr     = -101
} eIntelVvpSwitchErrors;

typedef enum {
    kIntelVvpSwitchInputDisabled      = 0,
    kIntelVvpSwitchInputEnabled       = 1,
    kIntelVvpSwitchInputConsumed      = 3,
    kIntelVvpSwitchInputConfigInvalid = -1
} eIntelVvpSwitchInputConfig;

typedef enum {
    kIntelVvpSwitchIntfFull       = 0,
    kIntelVvpSwitchIntfLite       = 1,
    kIntelVvpSwitchIntfFr         = 2,
    kIntelVvpSwitchIntfInvalid    = -1
} eIntelVvpSwitchIntfType;

    
typedef struct intel_vvp_switch_instance_s
{
    // intel VVP base instance
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    
    // Compile-time parameterization
    bool                      debug_enabled;
    eIntelVvpSwitchIntfType   intf_type;
    bool                      uninterrupted_inputs;
    bool                      auto_consume;
    uint8_t                   num_inputs;
    uint8_t                   num_outputs;
    bool                      crash_switch;
    bool                      use_treadies;
} intel_vvp_switch_instance;	   

/**
 * \brief Initialise a switch instance
 * 
 * Initialization function for a VVP switch instance, attempts accesses to the core
 * to initialize the fields of the switch instance and its base core instance
 * 
 * \param[in]    instance, the intel_vvp_switch_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios, this is simply the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the switch product id (0x0235)
 *               kIntelVvpSwitchRegMapVersionErr if the register map is not supported
 *               The initialization stops early if the vendor ID/product ID are not a match
 *               and the second register, containing version numbers, is not read
 * \pre          base is a proper accessor for an Intel VVP Switch IP core
 */
int intel_vvp_switch_init(intel_vvp_switch_instance *instance, intel_vvp_core_base base);

/**
 * \brief Query the intf_type parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     the intf_type field in the intel_vvp_switch_instance, kIntelVvpSwitchIntfInvalid in case of errors
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
eIntelVvpSwitchIntfType intel_vvp_switch_get_intf_type(intel_vvp_switch_instance *instance);

/**
 * \brief Query the debug_enabled parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     the debug_enabled field in the intel_vvp_switch_instance
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
bool intel_vvp_switch_get_debug_enabled(intel_vvp_switch_instance *instance);

/**
 * \brief Query the uninterrupted_inputs parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     whether the switch was configured for uninterrupted inputs
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 * \remarks    please refer to the switch user guide for the non-obvious meaning of this parameter,
 *             it is used by the system designer to indicate that inputs to the switch will not dry
 *             so a switch can safely be performed at start of frame without risking a lock up. This
 *             is a feature relevant to the Lite variant of the video streaming protocol and the
 *             parameter is irrelevant when in full variant. 
 */
bool intel_vvp_switch_has_uninterrupted_inputs(intel_vvp_switch_instance *instance);

/**
 * \brief Query the auto_consume parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     whether the switch was configured for auto_consume while performing a switch
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 * \remarks    please refer to the switch user guide for the non-obvious meaning of this parameter,
 *             it is a mode of operation where the switch delay opening new paths until old ones have
 *             closed thus preventing back-pressure at the inputs but possibly dropping video outputs
 *             while waiting for the new inputs to start their next frame.
 */
bool intel_vvp_switch_has_auto_consume(intel_vvp_switch_instance *instance);

/**
 * \brief Query the num_inputs parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     the number of inputs chosen when configuring the switch, 0 in case of error
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
uint8_t intel_vvp_switch_get_num_inputs(intel_vvp_switch_instance *instance);

/**
 * \brief Query the num_outputs parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     the number of outputs chosen when configuring the switch, 0 in case of error
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
uint8_t intel_vvp_switch_get_num_outputs(intel_vvp_switch_instance *instance);

/**
 * \brief Query the crash switching parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     whether the switch was configured for crash switching
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
bool intel_vvp_switch_is_crash_switch(intel_vvp_switch_instance *instance);

/**
 * \brief Query the use treadies parameter of a switch instance
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     whether the switch was configured to use treadies in FR mode
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
bool intel_vvp_switch_uses_tready(intel_vvp_switch_instance *instance);

/**
 * \brief get running status of the switch instance
 *
 * \param[in]  instance, pointer to the intel_vvp_switch_instance 
 * \return     true when a configuration is running, false while switching beteen configurations
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
bool intel_vvp_switch_is_running(intel_vvp_switch_instance *instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 * 
 * \param[in]    instance, pointer to the intel_vvp_switch_instance 
 * \return       true if there are outstanding writes
 * \pre          instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
bool intel_vvp_switch_get_commit_status(intel_vvp_switch_instance* instance);

/**
 * \brief Read the status register
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
uint8_t intel_vvp_switch_get_status(intel_vvp_switch_instance *instance);

/**
 * \brief Configure input behaviour
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \param[in]  input, the input to configure in the range [0..num_inputs-1]
 * \return     kIntelVvpCoreInstanceErr if the instance is invalid,
 *             kIntelVvpSwitchParameterErr if the input or configuration is invalid,
 *             kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 * \pre        intel_vvp_switch_commit_writes must be called for the changes to take effect
 */
int intel_vvp_switch_set_input_config(intel_vvp_switch_instance *instance, uint8_t input, eIntelVvpSwitchInputConfig input_config);

/**
 * \brief Query the current input behaviour
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \param[in]  input, the input to check in the range [0..num_inputs-1]
 * \return     configuration for the selected input,
 *             kIntelVvpSwitchInputConfigInvalid in case of error (invalid instance or invalid input)
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized input is a valid input and debug is enabled
 */
eIntelVvpSwitchInputConfig intel_vvp_switch_get_input_config(intel_vvp_switch_instance *instance, uint8_t input);

/**
 * \brief Configure output behaviour
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \param[in]  output, the output to configure in the range [0..num_outputs-1]
 * \param[in]  enabled, whether the output is enabled
 * \param[in]  input, when the output is enabled, the input used for the selected output in the range [0..num_inputs-1]
 * \return     kIntelVvpCoreInstanceErr if the instance is invalid,
 *             kIntelVvpSwitchParameterErr if the output is invalid,
 *             kIntelVvpSwitchParameterErr if attempting to enable an invalid input,
 *             note that trying to tie the output to an invalid input will disable the output
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized
 * \pre        intel_vvp_switch_commit_writes must be called for the changes to take effect
 */
int intel_vvp_switch_set_output_config(intel_vvp_switch_instance *instance, uint8_t output, bool enabled, uint8_t input);


/**
 * \brief Query the current output behaviour
 * 
 * \param[in]  instance, an intel_vvp_switch_instance
 * \param[in]  output, the output to check in the range [0..num_outputs-1]
 * \return     the input (in the range [0..num_input-1] currently tied to the given output
 *             -1 if the output is disabled, other negative values are for errors:
 *                kIntelVvpCoreInstanceErr, if the instance is invalid
 *                kIntelVvpSwitchParameterErr if the output is invalid,
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized, output is a valid output and debug is enabled
 */
int intel_vvp_switch_get_output_config(intel_vvp_switch_instance *instance, uint8_t output);

/**
 * \brief commit any outstanding writes from setxxxx commands
 * 
 * \param[in]  instance, pointer to the intel_vvp_switch_instance 
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_switch_instance and was successfully initialized 
 */
int intel_vvp_switch_commit_writes(intel_vvp_switch_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_SWITCH_H__
