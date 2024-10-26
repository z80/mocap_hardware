

#include "task_led.h"
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

static void func_task_led( void * p );

uint8_t led_value = 0x01;
static osMutexDef_t mutex;
static osMutexId    mutexId;
//osThreadDef(imuTask, vTaskImu, osPriorityAboveNormal, 0, 512);
osThreadDef(task_led, func_task_led, osPriorityNormal, 0, 128);


void task_led_init()
{
	//mutexId = osMutexCreate( &mutex );
	//osThreadCreate( osThread(task_led), NULL );
}

void set_leds( uint8_t led )
{
	//osMutexWait( mutexId, osWaitForever );
	led_value = led;
	//osMutexRelease( mutexId );
}

void set_led( uint8_t index, uint8_t en )
{
	uint8_t bit = (1<<index);
	if ( en != 0 )
		led_value = led_value | bit;
	else
		led_value = led_value & (~bit);
}

void set_instant_leds( uint8_t led )
{
	if (led & 0x01)
		HAL_GPIO_WritePin( LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_SET );
	else
		HAL_GPIO_WritePin( LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_RESET );

	if (led & 0x02)
		HAL_GPIO_WritePin( LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET );
	else
		HAL_GPIO_WritePin( LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET );

	if (led & 0x04)
		HAL_GPIO_WritePin( LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET );
	else
		HAL_GPIO_WritePin( LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET );
}

void set_instant_led( uint8_t index, uint8_t en )
{
	if ( index == 0 )
	{
		if ( en != 0 )
			HAL_GPIO_WritePin( LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_SET );
		else
			HAL_GPIO_WritePin( LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_RESET );
	}
	else if ( index == 1 )
	{
		if ( en != 0 )
			HAL_GPIO_WritePin( LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET );
		else
			HAL_GPIO_WritePin( LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET );
	}
	else
	{
		if ( en != 0 )
			HAL_GPIO_WritePin( LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET );
		else
			HAL_GPIO_WritePin( LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET );
	}
}


void func_task_led( void * p )
{
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	for(;;)
	{
		osMutexWait( mutexId, osWaitForever );
		uint8_t v = led_value;
		osMutexRelease( mutexId );

		/*if (v & 0x01)
			HAL_GPIO_TogglePin( LED_0_GPIO_Port, LED_0_Pin );
		else
			HAL_GPIO_WritePin( LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_RESET );

		if (v & 0x02)
			HAL_GPIO_TogglePin( LED_1_GPIO_Port, LED_1_Pin );
		else
			HAL_GPIO_WritePin( LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET );

		if (v & 0x04)
			HAL_GPIO_TogglePin( LED_2_GPIO_Port, LED_2_Pin );
		else
			HAL_GPIO_WritePin( LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET );*/

		osDelay(500);
	}
}





