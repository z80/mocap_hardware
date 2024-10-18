
#include "magdwick_imu.h"

static float invSqrt(float x);


void magdwick_init_params( struct TMagdwickParams * params, float beta, float delta_t )
{
    params->beta    = beta;
    params->delta_t = delta_t;

    params->lp_alpha         = 0.001f;
    params->acc_threshold    = 0.01f;
    params->gyro_threshold   = 0.02f;
    params->zero_samples_qty = 300;
}

void magdwick_init_quat( struct TMagdwickQuat * quat )
{
    quat->q[0] = 1.0f;
    quat->q[1] = quat->q[2] = quat->q[3] = 0.0f;
}

void magdwick_update_imu( struct TMagdwickQuat * quat, struct TMagdwickParams * params, struct TMagdwickImuData * imu )
{
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

    float q0 = quat->q[0];
    float q1 = quat->q[1];
    float q2 = quat->q[2];
    float q3 = quat->q[3];

    float gx = imu->w[0];
    float gy = imu->w[1];
    float gz = imu->w[2];

    float ax = imu->a[0];
    float ay = imu->a[1];
    float az = imu->a[2];

    float beta    = params->beta;
    float delta_t = params->delta_t;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if ( (ax != 0.0f) || (ay != 0.0f) || (az != 0.0f) )
    {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_4q0 = 4.0f * q0;
		_4q1 = 4.0f * q1;
		_4q2 = 4.0f * q2;
		_8q1 = 8.0f * q1;
		_8q2 = 8.0f * q2;
		q0q0 = q0 * q0;
		q1q1 = q1 * q1;
		q2q2 = q2 * q2;
		q3q3 = q3 * q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * delta_t;
	q1 += qDot2 * delta_t;
	q2 += qDot3 * delta_t;
	q3 += qDot4 * delta_t;

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;

    quat->q[0] = q0;
    quat->q[1] = q1;
    quat->q[2] = q2;
    quat->q[3] = q3;
}

void magdwick_init_bias( struct TMagdwickBiasEstimation * params )
{
	params->lowpass_a[0] = params->lowpass_a[1] = params->lowpass_a[2] = 0.0f;
	params->lowpass_w[0] = params->lowpass_w[1] = params->lowpass_w[2] = 0.0f;
	params->bias_w[0]    = params->bias_w[1]    = params->bias_w[2] = 0.0f;
	params->zero_samples_qty = 0;
}

void magdwick_update_bias( struct TMagdwickBiasEstimation * params )
{

}





// Fast inverse square-root
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root

float invSqrt(float x)
{
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}





