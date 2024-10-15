
#include "task_bmi085.h"
#include "task_led.h"

#include "cmsis_os.h"
#include "bmi08x.h"
#include "bmi08_defs.h"

#include "main.h"

#define BMI08_READ_WRITE_LEN  UINT8_C(46)

#define MUL_ADDR_1      (0x70<<1)
#define MUL_ADDR_2      (0x71<<1)
#define I2C_TIMEOUT     168000000

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

uint8_t acc_dev_add_a,
		acc_dev_add_b;
uint8_t gyro_dev_add_a,
		gyro_dev_add_b;

static void bmi085_delay( uint32_t usec, void *intf_ptr );

static uint8_t bmi085_switch_1( uint8_t channel );
static uint8_t bmi085_switch_2( uint8_t channel );
static BMI08_INTF_RET_TYPE bmi085_bus_read_i2c_1( uint8_t reg_addr, uint8_t *reg_data, uint8_t qty, void *intf_ptr );
static BMI08_INTF_RET_TYPE bmi085_bus_write_i2c_1( uint8_t reg_addr, uint8_t *reg_data, uint8_t qty, void *intf_ptr );
static BMI08_INTF_RET_TYPE bmi085_bus_read_i2c_2( uint8_t reg_addr, uint8_t * reg_data, uint8_t qty, void *intf_ptr );
static BMI08_INTF_RET_TYPE bmi085_bus_write_i2c_2( uint8_t reg_addr, uint8_t * reg_data, uint8_t qty, void *intf_ptr );


struct T_BMI085
{
	uint8_t acc_addr;
	uint8_t gyro_addr;
	struct bmi08_dev bmi085;
};

//struct T_BMI085 imus_a[8],
//                imus_b[8];
struct T_BMI085 bmi085;

static uint8_t bmi08_interface_init(struct T_BMI085 *bma);


static void bmi085_delay( uint32_t usec, void *intf_ptr )
{
	(void)intf_ptr;

	uint32_t msec = usec / 1000;
	if (msec < 1)
		msec = 1;
	osDelay( msec );
}

static uint8_t bmi085_switch_1( uint8_t channel )
{
    unsigned char data = (1 << channel);
	HAL_StatusTypeDef res = HAL_I2C_Master_Transmit( &hi2c1, MUL_ADDR_1, &data, 1, I2C_TIMEOUT );
	int8_t result = ( res == HAL_OK ) ? 0 : 1;

    return result;
}

static uint8_t bmi085_switch_2( uint8_t channel )
{
    unsigned char data = (1 << channel);
	HAL_StatusTypeDef res = HAL_I2C_Master_Transmit( &hi2c2, MUL_ADDR_2, &data, 1, I2C_TIMEOUT );
	int8_t result = ( res == HAL_OK ) ? 0 : 1;

    return result;
}

static BMI08_INTF_RET_TYPE bmi085_bus_read_i2c_1( uint8_t reg_addr, uint8_t *reg_data, uint8_t qty, void *intf_ptr )
{
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	dev_addr *= 2;
	HAL_StatusTypeDef res = HAL_I2C_Mem_Read( &hi2c1, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, qty, I2C_TIMEOUT );
	int8_t result = ( res == HAL_OK ) ? BMI08_INTF_RET_SUCCESS : (BMI08_INTF_RET_SUCCESS+1);
	return result;
}

static BMI08_INTF_RET_TYPE bmi085_bus_write_i2c_1( uint8_t reg_addr, uint8_t *reg_data, uint8_t qty, void *intf_ptr )
{
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	dev_addr *= 2;
	HAL_StatusTypeDef res = HAL_I2C_Mem_Write( &hi2c1, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, qty, I2C_TIMEOUT );
	int8_t result = ( res == HAL_OK ) ? BMI08_INTF_RET_SUCCESS : (BMI08_INTF_RET_SUCCESS+1);
	return result;
}

static BMI08_INTF_RET_TYPE bmi085_bus_read_i2c_2( uint8_t reg_addr, uint8_t * reg_data, uint8_t qty, void *intf_ptr )
{
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	dev_addr *= 2;
	HAL_StatusTypeDef res = HAL_I2C_Mem_Read( &hi2c2, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, qty, I2C_TIMEOUT );
	int8_t result = ( res == HAL_OK ) ? BMI08_INTF_RET_SUCCESS : (BMI08_INTF_RET_SUCCESS+1);
	return result;
}

static BMI08_INTF_RET_TYPE bmi085_bus_write_i2c_2( uint8_t reg_addr, uint8_t * reg_data, uint8_t qty, void *intf_ptr )
{
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	dev_addr *= 2;
	HAL_StatusTypeDef res = HAL_I2C_Mem_Write( &hi2c2, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, qty, I2C_TIMEOUT );
	int8_t result = ( res == HAL_OK ) ? BMI08_INTF_RET_SUCCESS : (BMI08_INTF_RET_SUCCESS+1);
	return result;
}

static uint8_t bmi08_interface_init(struct T_BMI085 *bma )
{
	bma->acc_addr  = BMI08_ACCEL_I2C_ADDR_PRIMARY;
	bma->gyro_addr = BMI08_GYRO_I2C_ADDR_PRIMARY;

	struct bmi08_dev * bmi08 = &(bma->bmi085);

    bmi08->intf = BMI08_I2C_INTF;
    bmi08->read = bmi085_bus_read_i2c_2;
    bmi08->write = bmi085_bus_write_i2c_2;

    /* Selection of bmi085 or bmi088 sensor variant */
    bmi08->variant = BMI085_VARIANT;

    /* Assign accel device address to accel interface pointer */
    bmi08->intf_ptr_accel = &(bma->acc_addr);

    /* Assign gyro device address to gyro interface pointer */
    bmi08->intf_ptr_gyro = &(bma->gyro_addr);

    /* Configure delay in microseconds */
    bmi08->delay_us = bmi085_delay;

    /* Configure max read/write length (in bytes) ( Supported length depends on target machine) */
    bmi08->read_write_len = BMI08_READ_WRITE_LEN;

    bmi085_delay( 20000, 0 );
}

static int8_t init_bmi08( struct T_BMI085 * dev )
{
	struct bmi08_dev * bmi08 = &(dev->bmi085);

    int8_t rslt;

    rslt = bmi08xa_init(bmi08);
    if ( rslt != BMI08_OK )
    	return rslt;

    rslt = bmi08g_init(bmi08);
    if ( rslt != BMI08_OK )
    	return rslt;

    rslt = bmi08a_load_config_file(bmi08);
    if ( rslt != BMI08_OK )
    	return rslt;

 	bmi08->accel_cfg.odr = BMI08_ACCEL_ODR_1600_HZ;
   	bmi08->accel_cfg.range = BMI085_ACCEL_RANGE_16G;

    bmi08->accel_cfg.power = BMI08_ACCEL_PM_ACTIVE;
    bmi08->accel_cfg.bw = BMI08_ACCEL_BW_NORMAL;

    rslt = bmi08a_set_power_mode( bmi08 );
    if ( rslt != BMI08_OK )
    	return rslt;

    rslt = bmi08xa_set_meas_conf( bmi08 );
    if ( rslt != BMI08_OK )
    	return rslt;

    bmi08->gyro_cfg.odr = BMI08_GYRO_BW_230_ODR_2000_HZ;
    bmi08->gyro_cfg.range = BMI08_GYRO_RANGE_250_DPS;
    bmi08->gyro_cfg.bw = BMI08_GYRO_BW_230_ODR_2000_HZ;
    bmi08->gyro_cfg.power = BMI08_GYRO_PM_NORMAL;

    rslt = bmi08g_set_power_mode(bmi08);
    if ( rslt != BMI08_OK )
    	return rslt;

    rslt = bmi08g_set_meas_conf(bmi08);

    return rslt;
}





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
	bmi08_interface_init( &bmi085 );
	int ind;
	uint8_t rslt;

	//for ( ind=0; ind<8; ind++ )
	{
		bmi085_switch_2( 6 );
		rslt = init_bmi08( &bmi085 );
		//if ( rslt == BMI08_OK )
		//	break;
	}

	set_led( ind );

	for (;;)
	{
		bmi085_delay( 10000, 0 );
	}
}

