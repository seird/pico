#ifndef __PID_H__
#define __PID_H__


#include <pico/stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 *    r(k)       e(k)                       u(k)                        y(k)
 * -------->(+)------------------>[PID]------------->[PLANT]----------------->
 *       +   ^                                                          |
 *           |  -                                                       |
 *           |                                                          |
 *           |             m(k)                                         |
 *           ------------------------[MEASURE]---------------------------
 */

struct PID {
    float Kp;       // Proportional
    float Kid;      // Integral (Ki*tSample)
    float Kdd;      // Differential (Kd/tSample)
    float uMax;     // Max output
    float uMin;     // Min output
    uint tSample;   // Sample period in ms
    uint tLast;     // Last time the pid output was computed
    float eLast;    // Last error
    float eSum;     // Sum of errors
};


/**
 * @brief Initialize the PID object
 * 
 * @param pid 
 * @param Kp Proportional gain
 * @param Ki Integral gain
 * @param Kd Differential gain
 * @param uMax Maximum PID output
 * @param uMin Minimum PID output
 * @param tSample Sample period in ms
 */
void pid_init(struct PID * pid, float Kp, float Ki, float Kd, float uMax, float uMin, uint tSample);


/**
 * @brief Reset the PID errors and tLast
 * 
 * @param pid 
 */
void pid_reset(struct PID * pid);


void pid_update_Kp(struct PID * pid, float Kp);
void pid_update_Ki(struct PID * pid, float Ki);
void pid_update_Kd(struct PID * pid, float Kd);
void pid_update_uMax(struct PID * pid, float uMax);
void pid_update_uMin(struct PID * pid, float uMin);
void pid_update_tSample(struct PID * pid, float tSample);


/**
 * @brief Adjust the PID output u(k)
 * 
 * @param pid
 * @param u The PID output
 * @param r The reference / desired system (motor) output
 * @param m The measure system (motor) output
 * @return true if the output was adjusted
 * @return false if the output was not adjusted (called before sample period)
 */
bool pid_adjust(struct PID * pid, float * u, float r, float m);


#ifdef __cplusplus
}
#endif


#endif // __PID_H__
