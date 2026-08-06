/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

//region Unused defines required by intel-vip-group.

//#undef HDMI_RX_TX_PMA_RECONFIG_BASE
#define HDMI_RX_TX_PMA_RECONFIG_BASE        0xFF700000
#define HDMI_RX_TX_PMA_RECONFIG_SPAN        0x4000

// These shouldn't do anything.
// Used by intel-vip-group, but we don't instantiate these (as far as I understand.)

// Suspicious. This is sitting inside the range of HDMI_RX_CORE_BASE:
// 0xFF30'0000 < 0xFF30'4000 <  0xFF30'000 + 0x2'0000
#define HDMI_TX_PLL_RECONFIG_BASE           0xFF304000
#define HDMI_TX_PLL_RECONFIG_SPAN           0x1000

#define HDMI_TX_IOPLL_REFCONFIG_BASE        0xFF305000
#define HDMI_TX_IOPLL_REFCONFIG_SPAN        0x800

#define HDMI_RX_EDID_BASE                   0xFF305800
#define HDMI_RX_EDID_SPAN                   0x400

#define HDMI_RX_INTERFACE_INFO_SLAVE_BASE   0xFF305C00
#define HDMI_RX_INTERFACE_INFO_SLAVE_SPAN   0x80

#define FMC_TI_I2C_BASE                     0xff305e00
#define FMC_TI_I2C_SPAN                     64

#define HDMI_TX_INTERFACE_BASE              0xFF305F00
#define HDMI_TX_INTERFACE_SPAN              0x80

#define HDMI_TX_I2C_BASE                    0xFF305F80
#define HDMI_TX_I2C_SPAN                    0x40

//endregion

// New Shell PD-HDMI Slaves

// 1 0xFF20'0000 + 0x10_0000 (av_mm_bridge) + 0x0000_0000
#define HDMI_RX_CORE_BASE                   0xFF300000
#define HDMI_RX_CORE_SPAN                   0x20000
// Checked

// 2
#define HDMI_RX_PHY_BASE                    0xFF340000
#define HDMI_RX_PHY_SPAN                    0x8000
// Checked

// 3
#define HDMI_TX_CORE_BASE                   0xFF320000
#define HDMI_TX_CORE_SPAN                   0x20000
//#define HDMI_TX_CORE_SPAN                   0x40000

#define HDMI_TX_PHY_BASE                    0xFF350000
#define HDMI_TX_PHY_SPAN                    0x10000

#define HDMI_TI_I2C_0_BASE                  0xFF360000
#define HDMI_TI_I2C_0_SPAN                  0x40

#undef BITEC_FMC_I2C_MASTER_FREQ

#define BITEC_FMC_I2C_MASTER_FREQ           100 * 1000 * 1000
#define BITEC_FMC_I2C_MAX_REGS              33
#define BITEC_FMC_I2C_TMDS181_ADDRESS       0xBA
#define BITEC_FMC_I2C_TPD158_ADDRESS        0xBC

#define SINK_I2C_MASTER_OFFSET              0x00

#define HDMI_TX_SINK_I2C_BASE               HDMI_TX_CORE_BASE + SINK_I2C_MASTER_OFFSET
#define HDMI_TX_SINK_I2C_SPAN               HDMI_TX_CORE_SPAN
// Set I2C to 100 KHz.
#define HDMI_TX_SINK_I2C_FREQ               100 * 1000 * 1000

