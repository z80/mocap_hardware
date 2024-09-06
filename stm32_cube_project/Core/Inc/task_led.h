/*
 * task_led.h
 *
 *  Created on: Jul 4, 2018
 *      Author: z80
 */

#ifndef TASK_LED_H_
#define TASK_LED_H_

#include <stdio.h>

void taskLedInit(void);
void toggleLed( int mask );

// HID Mouse
struct mouseHID_t {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
};


#endif /* TASK_LED_H_ */
