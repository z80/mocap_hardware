#include "task_bmi085.h"
#include "task_led.h"

#include "cmsis_os.h"
#include "bmi08.h"
#include "bmi08x.h"
#include "bmi08_defs.h"

#include "main.h"

#include "magdwick_imu.h"

#include "bmi085_io.h"




// Declare a task.
static void func_task_bmi085( void * p );
osThreadDef( task_bmi085, func_task_bmi085, osPriorityNormal, 0, 1024 );


// This function starts the task and exists.
// It doesn't do anything else.
void task_bmi085_init()
{
	osThreadCreate( osThread(task_bmi085), NULL );
}

static void init_all()
{
	uint8_t rslt;
	uint8_t index;

	for (index=0; index<16; index++)
	{
		rslt = bmi085_init( index );
	}

	for (index=16; index<32; index++)
	{
		rslt = bmi085_init( index );
		if ( index == 28 )
			osDelay( 1000 );
	}

//	rslt = bmi085_init( 28 );
//	osDelay( 1000 );
}

static void func_task_bmi085( void * p )
{

	init_all();

	//for ( ind=0; ind<8; ind++ )
	{
		//if ( rslt == BMI08_OK )
		//	break;
	}

	//set_led( ind );

	for (;;)
	{


		osDelay( 10 );
	}
}


