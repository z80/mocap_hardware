/*
 * task_led.c
 *
 *  Created on: Jul 4, 2018
 *      Author: z80
 */

#include "task_led.h"
#include "cmsis_os.h"
#include "gpio.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"

#include "task_host.h"
#include "task_imu.h"

#define LED_PORT GPIOA
#define LED_0    GPIO_PIN_6
#define LED_1    GPIO_PIN_7

struct mouseHID_t mouseHID;
struct mouseHID_t imuHID;

// Dimensions the buffer that the task being created will use as its stack.
// NOTE:  This is the number of words the stack will hold, not the number of
// bytes.  For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#define LED_STACK_SIZE 200

static USBD_HandleTypeDef hUsbDeviceFS;
static void taskDeviceInit( void )
{
	USBD_Init( &hUsbDeviceFS, &FS_Desc, DEVICE_FS );
	USBD_RegisterClass( &hUsbDeviceFS, &USBD_HID );
	USBD_Start( &hUsbDeviceFS );
}

// Function that implements the task being created.
static void vTaskLed( void * pvParameters )
{
	// The parameter value is expected to be 1 as 1 is passed in the
	// pvParameters value in the call to xTaskCreateStatic().
	//configASSERT( ( uint32_t ) pvParameters == 1UL );

	for( ;; )
	{
		// Task code goes here.
		toggleLed( 1 );
		osDelay( 100 );
	}
}

extern USBD_HandleTypeDef hUsbDeviceFS;
static void vTaskUsbIo( void * p )
{
	mouseHID.buttons = 0;
	mouseHID.x = 0;
	mouseHID.y = 0;
	mouseHID.wheel = 0;
	imuHID.buttons = 0;
	imuHID.x = 0;
	imuHID.y = 0;
	imuHID.wheel = 0;

	taskHostInit();
	taskDeviceInit();

	for ( ;; )
	{
		if ( taskHostProcess( &mouseHID ) )
		{
			//adjustMouse( &(mouseHID.x), &(mouseHID.y) );
			USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&mouseHID, sizeof(struct mouseHID_t));
			//osDelay( 8 );
			//mouseHID.x = mouseHID.y = 0;
			osDelay( 2 );
		}
		imuHID.buttons = mouseHID.buttons;
		imuHID.x = 0;
		imuHID.y = 0;
		imuHID.wheel   = mouseHID.wheel;
		if ( adjustMouse( &(imuHID.x), &(imuHID.y) ) )
		{
			USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&imuHID, sizeof(struct mouseHID_t));
			osDelay( 2 );
		}
	}
	//for (;;)
	//{
		//USBD_HID_SendReport(&hUsbDeviceFS, &mouseHID, sizeof(struct mouseHID_t));
		//osDelay( 500 );
	//}
}

// Function that creates a task.
osThreadDef(ledTask, vTaskLed, osPriorityAboveNormal, 0, 128);
osThreadDef(usbDeviceTask, vTaskUsbIo, osPriorityAboveNormal, 0, 512);
void taskLedInit( void )
{
	osThreadCreate(osThread(ledTask), NULL);
	osThreadCreate(osThread(usbDeviceTask), NULL);
}



static unsigned char leds[2] = {0, 0};
void toggleLed( int mask )
{
	if ( mask & 1 )
		leds[0] = ( leds[0] ) ? 0 : 1;
	if ( mask & 2 )
		leds[1] = ( leds[1] ) ? 0 : 1;
	if ( leds[0] )
		HAL_GPIO_WritePin( LED_PORT, LED_0, GPIO_PIN_SET );
	else
		HAL_GPIO_WritePin( LED_PORT, LED_0, GPIO_PIN_RESET );
	if ( leds[1] )
		HAL_GPIO_WritePin( LED_PORT, LED_1, GPIO_PIN_SET );
	else
		HAL_GPIO_WritePin( LED_PORT, LED_1, GPIO_PIN_RESET );
}





