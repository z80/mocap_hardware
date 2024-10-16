
#ifndef __MAGDWICK_H_
#define __MAGDWICK_H_

struct TMagdwickParams
{
    float beta;
    float delta_t;

    float lp_alpha;
    float acc_threshold;
    float gyro_threshold;
    uint32_t zero_samples_qty;
};

struct TMagdwickQuat
{
    float q[4];
};

struct TMagdwickImuData
{
    float w[3];
    float a[3];
};

struct TMagdwickBiasEstimation
{
	float lowpass_a[3];
	float lowpass_w[3];
	uint32_t zero_samples_qty;

	float bias_w[3];
};

void magdwick_init_params( struct TMagdwickParams * params, float beta, float freq );
void magdwick_init_quat( struct TMagdwickQuat * quat );
void magdwick_update_imu( struct TMagdwickQuat * quat, struct TMagdwickParams * params, struct TMagdwickImuData * imu );

void magdwick_init_bias( struct TMagdwickBiasEstimation * params );
void magdwick_update_bias( struct TMagdwickBiasEstimation * params );





#endif


