
#ifndef __BMI085_IO_H_
#define __BMI085_IO_H_

#include "main.h"

uint8_t bmi085_init_a( uint8_t index );
uint8_t bmi085_init_b( uint8_t index );

uint8_t bmi085_switch_a_irq( uint8_t index );
uint8_t bmi085_switch_b_irq( uint8_t index );

uint8_t bmi085_read_acc_a_irq( uint8_t index, uint8_t * data );
uint8_t bmi085_read_gyro_b_irq( uint8_t index, uint8_t * data );


#endif


