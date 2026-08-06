/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <time.h>
#include "config.h"
#include "system.h"
#include "intel_fpga_i2c.h"
#include "io.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define DP_CORE_PIO_BOARD_BASE DP_TX_DP_CORE_PIO_BOARD_BASE

#define I2C_BASE (DP_CORE_I2C_BOARD_BASE)
#define PIO_BASE (DP_CORE_PIO_BOARD_BASE)

int g_pio_base;

//==================================================================
// Agilex 5 Premium Devkit Infrastructure
//==================================================================
void board_configure(void)
{

#if (SELECTED_BOARD == 1)  
  printf("Programing Agilex 5 Premium Devkit SI548...\n");

  // program the SI548. We need 150MHz, it comes up as 135 Mhz
  intel_fpga_i2c_init(I2C_BASE, 100000000);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 255,0);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 69,0);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 17,0);

  // 150MHz settings were lifted direct from Table 5.9 in the SI548 DataSheet. They have 
  // a pre-calculated list of common frequencies.
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 23,0x48);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 24,0x00);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 26,0xA7);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 27,0x97);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 28,0xF4);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 29,0xC5);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 30,0x46);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 31,0x00);

  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 7,8);
  // Wait 50mS for SI548 to reboot...
  usleep(50000);
  intel_fpga_i2c_write_extended(I2C_BASE, 0x5A , 17,1);

#endif  
  // Release System PLL Reset, after clocks are stable.
  usleep(50000);
  g_pio_base = 1;
  IOWR(PIO_BASE,0,g_pio_base);

  // Release all resets
  usleep(50000);
  g_pio_base |= 2;
  IOWR(PIO_BASE,0,g_pio_base);
  
}

#if DP_SUPPORT_AXI
//==================================================================
// Agilex 5 Premium Devkit Tx Video Clock Control for AXIS Design
//==================================================================
void board_tx_freq(int freq)
{

  int divide = (297000000/freq)-1;
  
  g_pio_base &= 0x00ff;
  g_pio_base |= (divide & 0xff)<<8;
  IOWR(PIO_BASE,0,g_pio_base);
}
#endif
