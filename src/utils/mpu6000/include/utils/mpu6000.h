#ifndef __MPU6000_H__
#define __MPU6000_H__


#include <pico/stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


void mpu6000_reset();
void mpu6000_calibrate(int nCalibrations);
bool mpu6000_read(int16_t accel[3], int16_t gyro[3]);


#ifdef __cplusplus
}
#endif


#endif // __MPU6000_H__
