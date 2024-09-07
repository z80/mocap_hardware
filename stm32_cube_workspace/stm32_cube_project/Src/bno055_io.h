
#ifndef __BNO055_IO_H_
#define __BNO055_IO_H_


#include "main.h"

void bno055_delay( uint32_t msec );

int8_t bno055_switch_1( uint8_t channel );
int8_t bno055_switch_2( uint8_t channel );
int8_t bno055_bus_read_i2c_1( uint8_t  dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t qty );
int8_t bno055_bus_write_i2c_1( uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t qty );
int8_t bno055_bus_read_i2c_2( uint8_t  dev_addr, uint8_t reg_addr, uint8_t * reg_data, uint8_t qty );
int8_t bno055_bus_write_i2c_2( uint8_t dev_addr, uint8_t reg_addr, uint8_t * reg_data, uint8_t qty );

#endif






