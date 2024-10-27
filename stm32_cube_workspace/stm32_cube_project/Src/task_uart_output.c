
#include "task_uart_output.h"
#include "main.h"

#include "task_led.h"
#include "task_bmi085.h"

#include "cmsis_os.h"

#include <string.h>



static void func_task_uart_rx( void * p );
static void func_task_uart_tx( void * p );

osThreadDef(task_uart_rx, func_task_uart_rx, osPriorityNormal, 0, 512);
osThreadDef(task_uart_tx, func_task_uart_tx, osPriorityNormal, 0, 512);


#define CMD_STREAM_MODE 1
#define CMD_STOP_MODE   2

// Message queue for giving the IMU task commands out of other tasks.
osMessageQDef(command_queue, 2, uint8_t); // Message queue with 2 slots for uint8_t messages
osMessageQId command_queue_id;

osMessageQDef(rx_queue, 32, char); // Queue of characters received via UART.
osMessageQId rx_queue_id;

osMessageQDef(adc_queue, 2, uint32_t); // Queue of characters received via UART.
osMessageQId adc_queue_id;

void task_uart_output_init()
{
	osThreadCreate( osThread(task_uart_rx), NULL );
	osThreadCreate( osThread(task_uart_tx), NULL );
	command_queue_id = osMessageCreate( osMessageQ(command_queue), NULL );
	rx_queue_id      = osMessageCreate( osMessageQ(rx_queue), NULL );
	adc_queue_id     = osMessageCreate( osMessageQ(adc_queue), NULL );
}

extern ADC_HandleTypeDef  hadc1;
extern UART_HandleTypeDef huart2;

static char * read_cmd();
static void parse_cmd( char * cmd, int cmd_len );

static void stream_data_func();

static void update_crc8( uint8_t * data, uint16_t qty, uint8_t * p_crc8 );

// The character buffer for a single character.
// Reading in interrupt mode.
static char global_c;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
	    osMessagePut(rx_queue_id, global_c, 0);
	    // Wait for the next byte.
	    HAL_UART_Receive_IT( &huart2, &global_c, 1 );
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

    if (hadc->Instance == ADC1)
    {
        // Get the converted value
        uint32_t adc_value = HAL_ADC_GetValue( hadc );
        osMessagePut(adc_queue_id, adc_value, 0);
    }
}

void func_task_uart_rx( void * p )
{
	// Initiate interrupt-based reception.
	HAL_UART_Receive_IT( &huart2, &global_c, 1 );

	for (;;)
	{
		int cmd_len;
		char * cmd = read_cmd( &cmd_len );
		parse_cmd( cmd, cmd_len );
	}
}

void func_task_uart_tx( void * p )
{
	uint32_t PreviousWakeTime = osKernelSysTick();
	uint8_t stream_data = 0;
	static uint32_t adc_value = 0;

	for ( ;; )
	{
		// Request ADC conversion.
		HAL_ADC_Start_IT( &hadc1 );
		const osEvent event_adc = osMessageGet( adc_queue_id, 0 );
		if (event_adc.status == osEventMessage)
		{
			adc_value = event_adc.value.v;
		}

		if ( stream_data )
		{
			stream_data_func( adc_value );
		}

		// Check incoming commands.
		const osEvent event_cmd = osMessageGet(command_queue_id, 0);
		if (event_cmd.status == osEventMessage)
		{
		    switch (event_cmd.value.v)
		    {
		    case CMD_STREAM_MODE:
		    	stream_data = 1;
		    	break;
		    case CMD_STOP_MODE:
		    	stream_data = 0;
		    	break;
		    }
		}

		osDelayUntil( &PreviousWakeTime, 33 );
	}
}

char * read_cmd( int * cmd_len )
{
	static char buffer[256];
	static const int buffer_size = sizeof(buffer);
	int char_index = 0;
	while ( char_index < buffer_size )
	{
		//HAL_StatusTypeDef ret = HAL_UART_Receive( &huart2, cc, sizeof(cc), 100 );
		const osEvent event = osMessageGet( rx_queue_id, osWaitForever );
		if (event.status == osEventMessage)
		{
			char c = event.value.v;
			if (c == '\r')
				continue;
			if (c == '\t')
				c = ' ';
			buffer[char_index] = c;
			char_index += 1;
			if ( c == '\n' )
			{
				buffer[char_index] = '\0';
				if (cmd_len != 0)
				{
					*cmd_len = char_index;
				}
				char_index = 0;
				return buffer;
			}
		}
	}

	if (cmd_len != 0)
	{
		*cmd_len = buffer_size;
	}
	return buffer;
}

static void process_cmd( char ** words, int words_qty );

void parse_cmd( char * cmd, int cmd_len )
{
	// Array of pointers to char.
	const int words_max_qty = 3;
	char * words[words_max_qty];

	int word_index = 0;
	char *saveptr;
	char *word = strtok_r( cmd, " ", &saveptr );
	while ( (word != 0) && (word_index < words_max_qty) )
	{
		words[word_index] = word;
		word_index += 1;

	    word = strtok_r( NULL, " ", &saveptr );
	}

	// Need to replace all spaces and carrage returns with '\0' symbols.
	for ( int i=0; i<cmd_len; i++ )
	{
		const char c = cmd[i];
		if ( ( c == ' ' ) || ( c == '\r' ) || ( c == '\n' ) )
			cmd[i] = '\0';
	}

	process_cmd( words, word_index );
}

static void process_cmd( char ** words, int words_qty )
{
	if (words_qty < 1)
		return;

	if ( strcmp( words[0], "data" ) == 0 )
	{
		if (words_qty < 2)
			return;
		// Can be either magnetic or inertial.
		if ( strcmp( words[1], "stream" ) == 0 )
		{
			osMessagePut( command_queue_id, CMD_STREAM_MODE, 0 );
		}
		else if ( strcmp( words[1], "stop" ) == 0 )
		{
			osMessagePut( command_queue_id, CMD_STOP_MODE, 0 );
		}
	}
}

static void ubyte_to_hex( uint8_t val, char * hex_str );
static void short_to_hex( int16_t val, char * hex_str );
static void ushort_to_hex( uint16_t val, char * hex_str );
static void ulong_to_hex( uint32_t val, char * hex_str );

void stream_data_func( uint32_t adc_value )
{
	static struct TImuData16 imu_data;
	get_bmi085_data( &imu_data );

	// Max buffer size.
	// 4 - detected IMUs, 4 - read IMUs, 2x4 per IMU, 1 end of string '\n'.
	// All data bytes except the last '\n' actually take 2 bytes to encode into text format.
	// In total:
	// (4 + 4 + 2*4*32)x2 + 1 = 529.
	// But at most need 8 bytes at a time.
	static char buffer[8];

	uint8_t crc8 = 0;

	set_instant_led( 2, 1 );

	ushort_to_hex( (uint16_t)adc_value, buffer );
	update_crc8( buffer, 4, &crc8 );
	HAL_UART_Transmit( &huart2, buffer, 4, 10 );

	ubyte_to_hex( imu_data.imus_detected, buffer );
	update_crc8( buffer, 2, &crc8 );
	HAL_UART_Transmit( &huart2, buffer, 2, 10 );


	for ( int i=0; i<32; i++ )
	{
		struct TQuat16 * q = &(imu_data.quats[i]);

		short_to_hex( q->w, buffer );
		update_crc8( buffer, 4, &crc8 );
		HAL_UART_Transmit( &huart2, buffer, 4, 10 );

		short_to_hex( q->x, buffer );
		update_crc8( buffer, 4, &crc8 );
		HAL_UART_Transmit( &huart2, buffer, 4, 10 );

		short_to_hex( q->y, buffer );
		update_crc8( buffer, 4, &crc8 );
		HAL_UART_Transmit( &huart2, buffer, 4, 10 );

		short_to_hex( q->z, buffer );
		update_crc8( buffer, 4, &crc8 );
		HAL_UART_Transmit( &huart2, buffer, 4, 10 );
	}

	// Send control sum.
	ubyte_to_hex( crc8, buffer );
	HAL_UART_Transmit( &huart2, buffer, 2, 10 );

	buffer[0] = '\r';
	HAL_UART_Transmit( &huart2, buffer, 1, 10 );

	set_instant_led( 2, 0 );
}


static void ubyte_to_hex( uint8_t val, char * hex_str )
{
    const char hexDigits[] = "0123456789ABCDEF";
    uint16_t unsigned_val = (uint16_t)val; // Treat the number as unsigned for two's complement representation

    for (int i = 1; i>=0; i--)
    {
    	const int ind = unsigned_val % 16;
    	const char digit = hexDigits[ind];
        hex_str[i] = digit;
        unsigned_val /= 16;
    }
}

static void short_to_hex( int16_t val, char * hex_str )
{
    const char hexDigits[] = "0123456789ABCDEF";
    uint16_t unsigned_val = (uint16_t)val; // Treat the number as unsigned for two's complement representation

    for (int i = 3; i>=0; i--)
    {
    	const int ind = unsigned_val % 16;
    	const char digit = hexDigits[ind];
        hex_str[i] = digit;
        unsigned_val /= 16;
    }
}

static void ushort_to_hex( uint16_t val, char * hex_str )
{
    const char hexDigits[] = "0123456789ABCDEF";
    uint16_t unsigned_val = val; // Treat the number as unsigned for two's complement representation

    for (int i = 3; i>=0; i--)
    {
    	const int ind = unsigned_val % 16;
    	const char digit = hexDigits[ind];
        hex_str[i] = digit;
        unsigned_val /= 16;
    }
}

static void ulong_to_hex( uint32_t val, char * hex_str )
{
    const char hexDigits[] = "0123456789ABCDEF";
    uint32_t unsigned_val = val;

    for (int i = 7; i>=0; i--)
    {
    	const int ind = unsigned_val % 16;
    	const char digit = hexDigits[ind];
        hex_str[i] = digit;
        unsigned_val /= 16;
    }
}

static void update_crc8( uint8_t * data, uint16_t qty, uint8_t * p_crc8 )
{
	uint8_t crc8 = *p_crc8;

	for (uint16_t i = 0; i < qty; i++)
	{
	    uint8_t byte = data[i];
		crc8 ^= byte;
		for (uint8_t j = 0; j < 8; j++)
		{
			if (crc8 & 0x80)
			{
				// Polynomial 0x07
				crc8 = (crc8 << 1) ^ 0x07;
			}
			else
			{
				crc8 <<= 1;
			}
		}
	}

	*p_crc8 = crc8;
}

