#ifndef __MOTOR_H__
#define __MOTOR_H__


#include <pico/stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


struct Motor {
    uint in1;
    uint in2;
    uint en;
    float s;
};


void motor_init(struct Motor * m, uint in1, uint in2, uint en);
void motor_set_speed(struct Motor * m, float s);
void motor_increment_speed(struct Motor * m, float ds);


#ifdef __cplusplus
}
#endif


#endif // __MOTOR_H__
