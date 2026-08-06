/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_FPGA_I2C_H
#define __INTEL_FPGA_I2C_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define I2C_TFR_CMD 0x0
#define I2C_RX_DATA 0x1
#define I2C_CTRL 0x2
#define I2C_ISER 0x3
#define I2C_ISR 0x4
#define I2C_STATUS 0x5
#define I2C_TFR_CMD_FIFO_LEVEL 0x6
#define I2C_RX_DATA_FIFO_LEVEL 0x7
#define I2C_SCL_LOW 0x8
#define I2C_SCL_HIGH 0x9
#define I2C_SDA_HOLD 0xA

#define I2C_WRITE_FLAG 0x0
#define I2C_READ_FLAG 0x1
#define I2C_START_FLAG 0x200
#define I2C_STOP_FLAG 0x100
#define I2C_STATUS_BUSY_BIT 0x1

#define NBR_OF_TI_REGISTERS 33
#define TI_RETIMER_I2C_ADDRESS 0xBA
#define TI_TPD158_I2C_ADDRESS 0xBC

void intel_fpga_i2c_init(long i2c_base, int sys_clk);

void intel_fpga_i2c_write_simple(long i2c_base, unsigned char address, unsigned char data);

unsigned char intel_fpga_i2c_read_simple(long i2c_base, unsigned char address);

void intel_fpga_i2c_write_extended(long i2c_base, unsigned char address, unsigned char reg,
                                   unsigned char data);

unsigned char intel_fpga_i2c_read_extended(long i2c_base, unsigned char address, unsigned char reg);

void intel_fpga_i2c_burst_read_extended(long i2c_base, unsigned char address, unsigned char reg,
                                        unsigned int burst_size, unsigned char* data);

void intel_fpga_i2c_burst_read_two_reg_extended(long i2c_base, unsigned char address,
                                                unsigned char* reg, unsigned int burst_size,
                                                unsigned char* data);

void intel_fpga_i2c_burst_write_extended(long i2c_base, unsigned char address, unsigned char reg,
                                         unsigned char burst_size, unsigned char* data);

void intel_fpga_i2c_wait_busy(long i2c_base);

unsigned int intel_fpga_i2c_tfr_cmd_fill(long i2c_base);

unsigned int intel_fpga_i2c_rx_data_fill(long i2c_base);

void intel_fpga_i2c_mc_write(long i2c_base, unsigned char address, unsigned int reg,
                             unsigned char* data, unsigned int length);

void intel_fpga_i2c_mc_read(long i2c_base, unsigned char address, unsigned int reg,
                            unsigned char* data, unsigned int length);

void intel_fpga_i2c_enable(long i2c_base);
void intel_fpga_i2c_disable(long i2c_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INTEL_FPGA_I2C_H */
