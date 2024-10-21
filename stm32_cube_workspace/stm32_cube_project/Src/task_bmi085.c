
#include "task_bmi085.h"
#include "task_led.h"

#include "cmsis_os.h"
#include "bmi08.h"
#include "bmi08x.h"
#include "bmi08_defs.h"

#include "main.h"

#include "bmi085_io.h"
#include "magdwick_imu.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

struct TImu
{
	uint8_t index;

	struct TMagdwickQuat           quat;
	struct TMagdwickBiasEstimation bias;
};

#define STATE_SET_CHANNEL 1
#define STATE_READ_ACC    2
#define STATE_READ_GYRO   3

struct TAllImus
{
	int imus_qty_a,
	    imus_qty_b;
	int imu_index_a,
	    imu_index_b;

	uint8_t state_a,
	        state_b;

	uint8_t raw_data_a[6],
	        raw_data_b[6];
	struct TImu imus_a[16];
	struct TImu imus_b[16];
	struct TMagdwickParams params;
};

static struct TAllImus all_imus;


static void enumerate_imus();
static void initiate_data_io();


// Declare a task.
static void func_task_bmi085( void * p );
osThreadDef( task_bmi085, func_task_bmi085, osPriorityNormal, 0, 1024 );


// This function starts the task and exists.
// It doesn't do anything else.
void task_bmi085_init()
{
	osThreadCreate( osThread(task_bmi085), NULL );
}


static void func_task_bmi085( void * p )
{

}

static void enumerate_imus()
{
	int16_t index;
	uint8_t ret;

	all_imus.imus_qty_a = 0;
	all_imus.imus_qty_b = 0;
	all_imus.imus_index_a = 0;
	all_imus.imus_index_b = 0;

	all_imus.state_a = STATE_SET_CHANNEL;
	all_imus.state_b = STATE_SET_CHANNEL;

	magdwick_init_params( &(all_imus.params), 0.1f, 0.01f );

	for ( index=0; index<16; index++ )
	{
		ret = bmi085_init( index );
		if ( ret == 0 )
		{
			struct TImu * imu = &(all_imus.imus_a[all_imus.imus_qty_a]);
			imu->index = index;
			magdwick_init_quat( &(imu->quat) );
			magdwick_init_bias( &(imu->bias) );

			all_imus.imus_qty_a += 1;
		}
	}

	for ( index=16; index<32; index++ )
	{
		ret = bmi085_init( index );
		if ( ret == 0 )
		{
			struct TImu * imu = &(all_imus.imus_a[all_imus.imus_qty_b]);
			imu->index = index;
			magdwick_init_quat( &(imu->quat) );
			magdwick_init_bias( &(imu->bias) );

			all_imus.imus_qty_b += 1;
		}
	}
}

static void initiate_data_io()
{
	if ( all_imus.imus_qty_a > 0 )
	{
		all_imus.imu_index_a = 0;
		struct TImu * imu = all_imus.imus_a[0];
		bmi085_switch_irq( imu->index );
	}

	if ( all_imus.imus_qty_b > 0 )
	{
		all_imus.imu_index_b = 0;
		struct TImu * imu = all_imus.imus_b[0];
		bmi085_switch_irq( imu->index );
	}
}


void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if ( hi2c == &hi2c1 )
	{
		all_imus.state_a = STATE_READ_ACC;
		uint8_t array_index_a = all_imus.imu_index_a;
		struct TImu * imu = all_imus.imus_a[array_index_a];
		uibt8_t imu_index = imu->index;

		bmi085_read_acc_irq( imu_index, all_imus.raw_data_a );
	}
	else
	{
		all_imus.state_b = STATE_READ_ACC;
		uint8_t array_index_b = all_imus.imu_index_b;
		struct TImu * imu = all_imus.imus_a[array_index_b];
		uibt8_t imu_index = imu->index;

		bmi085_read_acc_irq( imu_index, all_imus.raw_data_b );
	}
}

// Callback function for memory read complete
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if ( hi2c == &hi2c1 )
	{

	}
	else
	{

	}
}

// Error callback function
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    // Error handling
}







