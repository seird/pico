#include <pico/stdlib.h>

#include "utils/pid.h"


#define LIMIT(x, _min, _max) (MIN(MAX(_min, x), _max))


void
pid_init(struct PID * pid, float Kp, float Ki, float Kd, float uMax, float uMin, uint tSample)
{
    pid->Kp = Kp;
    pid->Kid = Ki*tSample;
    pid->Kdd = Kd/tSample;
    pid->uMax = uMax;
    pid->uMin = uMin;
    pid->tSample = tSample;
    pid->tLast = time_us_32()/1000 - tSample;
    pid->eLast = 0;
    pid->eSum = 0;
}


void
pid_reset(struct PID * pid)
{
    pid->tLast = time_us_32()/1000 - pid->tSample;
    pid->eLast = 0;
    pid->eSum = 0;
}


void
pid_update_Kp(struct PID * pid, float Kp)
{
    pid->Kp = Kp;
}


void
pid_update_Ki(struct PID * pid, float Ki)
{
    pid->Kid = Ki*pid->tSample;
}


void
pid_update_Kd(struct PID * pid, float Kd)
{
    pid->Kdd = Kd/pid->tSample;
}


void
pid_update_uMax(struct PID * pid, float uMax)
{
    pid->uMax = uMax;
}


void
pid_update_uMin(struct PID * pid, float uMin)
{
    pid->uMin = uMin;
}


void
pid_update_tSample(struct PID * pid, float tSample)
{
    float Kd = pid->Kdd*pid->tSample;
    float Ki = pid->Kid/pid->tSample;
    pid->Kdd = Kd/tSample;
    pid->Kid = Ki*tSample;
    pid->tSample = tSample;
}


bool
pid_adjust(struct PID * pid, float * u, float r, float m)
{
    // TODO: limit e_i
    uint32_t t = time_us_32()/1000;
    if (t < (pid->tLast + pid->tSample))
        return false;

    float e = r - m;
    float eSum = pid->eSum + e;

    float _u = pid->Kp  * e
             + pid->Kdd * (e-pid->eLast)
             + pid->Kid * eSum;

    // *u = LIMIT(_u, pid->uMin, pid->uMax);
    *u = _u;

    pid->eLast = e;
    pid->eSum = eSum;
    pid->tLast = t;

    return true;
}
