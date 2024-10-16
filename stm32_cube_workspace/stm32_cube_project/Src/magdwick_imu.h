
#ifndef __MAGDWICK_H_
#define __MAGDWICK_H_

struct TMagdwickParams
{
    float beta;
    float delta_t;
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

void magdwick_init_params( struct TMagdwickParams * params, float beta, float freq );
void magdwick_init_quat( struct TMagdwickQuat * quat );
void magdwick_update_imu( struct TMagdwickQuat * quat, struct TMagdwickParams * params, struct TMagdwickImuData * imu );







#endif


