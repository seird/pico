#ifndef __SLEEP_H__
#define __SLEEP_H__


#include "pico/stdlib.h"


/** Available sleep modes */
typedef enum SleepMode {
    SM_DEFAULT, /**< use sleep_ms */
    SM_SLEEP,   /**< Sleep; Note that the pico must sleep for at least 2 seconds in this mode. */
    SM_DORMANT  /**< Dormant / deep sleep */
} sleepmode_t;


/** Sleep configuration */
typedef struct SleepConfig {
    sleepmode_t mode;                       /**< the desired sleep mode */

    int8_t hours;                           /**< hours between executions [0-23] */
    int8_t minutes;                         /**< minutes between executions [0-59] */
    int8_t seconds;                         /**< seconds between executions [0-59] */

    void (* loop_function) (void * arg);    /**< a function that is called every loop after wake */
    void * arg;                             /**< an argument that is passed to the function */

    uint pin_wakeup;                        /**< trigger for dormant mode */
    bool edge;                              /**< Interrupt triggered on leading edge (true) or trailing edge (false) */
    bool active;                            /**< GPIO Pin on active HIGH (true) or active LOW (false) */
    
    datetime_t _t_start;                    /**< initial time: when sleep starts */
    datetime_t _t_end;                      /**< end time: when the pico wakes up again */
} sleepconfig_t;


/**
 * @brief Run a user defined function in a loop. Between executions, the pico goes to sleep 
 * for a desired amount of time
 * 
 * @param cfg 
 */
void sleep_run(sleepconfig_t * cfg);


#endif // __SLEEP_H__
