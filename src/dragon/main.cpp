#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>
#include <pico/binary_info.h>
#include <hardware/i2c.h>

#include "utils/pin.hpp"
#include "utils/mpu6000.h"
#include "utils/motor.h"
#include "utils/pid.h"
#include "utils/radio.hpp"


#define TELEMETRY_INTERVAL 1 // Transmit telemetry every .. PID adjustments

struct Motor motor1;
uint motor1Ena = 0;
uint motor1In1 = 1;
uint motor1In2 = 2;

struct Motor motor2;
uint motor2Ena = 28;
uint motor2In1 = 27;
uint motor2In2 = 26;

struct PID pid; // Balancing PID -- x-axis
float Kp = 12.0f;
float Ki = 0.0f;
float Kd = 0.1f;
float uMax = 1.0f;
float uMin = -1.0f;
uint tSample = 10;

struct PID pidTurn; // Turning PID -- y-axis
float KpTurn = 1.0f;
float KiTurn = 0.0f;
float KdTurn = 0.1f;
float uMaxTurn = 1.0f;
float uMinTurn = -1.0f;
uint tSampleTurn = 50;

Radio radio;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);

const uint8_t writing_address[] = WRITING_ADDRESS;
const uint8_t address_read_node[6] = ADDRESS_READ;

queue_t queue_rx;
queue_t queue_tx;


enum HeaderTx {
    THDR_U = 0,
    THDR_KP,
    THDR_KI,
    THDR_KD,
    THDR_TSAMPLE,
    THDR_MODE,
    THDR_ERROR,
};

enum HeaderRx {
    RHDR_NOP,
    RHDR_KP,
    RHDR_KI,
    RHDR_KD,
    RHDR_TSAMPLE,
    RHDR_HELLO,
    RHDR_MODE,
    THDR_CALIBRATE,
};

enum Mode {
    MODE_STAY = 0,
    MODE_FORWARD,
    MODE_BACKWARD,
    MODE_LEFT,
    MODE_RIGHT,
    MODE_FORWARD_LEFT,
    MODE_FORWARD_RIGHT,
    MODE_BACKWARD_LEFT,
    MODE_BACKWARD_RIGHT,
};


bool
radio_init() {
    if (!radio.setup(PAYLOAD_SIZE))
        return false;

    radio.openWritingPipe(writing_address);
    radio.openReadingPipe(PIPE_READ, address_read_node);
    radio.startListening(); // set the radio in RX mode
    
    return true;
}


static bool
setup()
{
    stdio_init_all();

    /* pins */
    pin_LED.off();

    queue_init(&queue_rx, PAYLOAD_SIZE, 100);
    queue_init(&queue_tx, PAYLOAD_SIZE, 100);

    while (!radio_init())
        pin_LED.on();
    pin_LED.off();

    /* Set system clock */
#if LOW_POWER
    // Changing the system fequency will affect the PWM motor control
    if (!set_sys_clock_khz(SYSTEM_FREQUENCY_KHZ, true)) {
        return false;
    }
#endif

    return true;
}


/**
 * @brief "Radio" core -- Handles sending and receiving packets.
 * 
 * Dragon sends telemetry data.
 * Dragon receives input to change certain parameters or change navigation mode.
 * 
 */
void core1(void)
{
    // TODO: no reason not to use the same buffer
    
    uint8_t buf_tx[PAYLOAD_SIZE];
    uint8_t buf_rx[PAYLOAD_SIZE];
    uint8_t pipe;

    while (true) {
        // Check if there's a packet to transmit
        if (!queue_is_empty(&queue_tx)) {
            radio.stopListening();
            while (queue_try_remove(&queue_tx, buf_tx))
                radio.send_packet(buf_tx);
            radio.startListening();
        }
        
        // Check for a new packet
        while (radio.receive_packet(buf_rx, &pipe))
            if (pipe == PIPE_READ)
                queue_try_add(&queue_rx, buf_rx);

        sleep_ms(1);
    }
}


int
main()
{
    while (!setup())
        pin_LED.blink(1, 100);

    motor_init(&motor1, motor1In1, motor1In2, motor1Ena);
    motor_init(&motor2, motor2In1, motor2In2, motor2Ena);
    pid_init(&pid, Kp, Ki, Kd, uMax, uMin, tSample);
    pid_init(&pidTurn, KpTurn, KiTurn, KdTurn, uMaxTurn, uMinTurn, tSampleTurn);
    
    i2c_init(i2c_default, 400*1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    mpu6000_reset();
    mpu6000_calibrate(2000);

    multicore_launch_core1(&core1);

    float u = 0;
    float uTurn = 0;
    float target = 0;
    float targetTurn = 0;
    enum Mode mode = MODE_STAY;

    int16_t gyro[3];
    int16_t accel[3];

    // TODO: also no reason to use 2 buffers
    uint8_t buf_tx[PAYLOAD_SIZE];
    uint8_t buf_rx[PAYLOAD_SIZE];
    
    pin_LED.blink(2, 100); // Initialization finished -> main loop is starting
    uint32_t ctr = 0; // counter used to send telemetry less frequently
    bool adjusted = false;
    
    uint32_t tLastUs = time_us_32();
    float angle = 0;
    mpu6000_read(accel, gyro);
    float angleAccel = atan2f(accel[2], -accel[1]) * 180 / M_PI;

    while (true) {
        uint32_t t = time_us_32();
        mpu6000_read(accel, gyro);

        float angleAccel = atan2f(accel[2], -accel[1]) * 180 / M_PI;
        float angleGyro =  (gyro[0] / (2*16384.0f)) * 500.0f * (t - tLastUs) / 1e6f;

        float alpha = 0.75 / (0.75 + (t - tLastUs) / 1e6f);
        
        angle = alpha*(angle + angleGyro) + (1-alpha)*angleAccel;
        
        tLastUs = t;

        if (adjusted = pid_adjust(&pid, &u, target, angle)) {
            u /= (4*45.0f);
            // if (fabsf(u) > 0.01) {
                motor_set_speed(&motor1, -u);
                motor_set_speed(&motor2, -u);            
            // }
        }
        
        // if (mode == MODE_LEFT || mode == MODE_RIGHT) {
        //     if (pid_adjust(&pidTurn, &uTurn, targetTurn, gyroY)) {
        //         motor_increment_speed(&motor1, uTurn);
        //         motor_increment_speed(&motor2, -uTurn);
        //     }
        // }
        

        // Telemetry
        if (adjusted && ctr++ % TELEMETRY_INTERVAL == 0) {
            buf_tx[0] = THDR_U;
            *(float *)(buf_tx + 1) = u;
            queue_try_add(&queue_tx, buf_tx);
        }

        // Check for received commands and handle them accordingly
        if (queue_try_remove(&queue_rx, buf_rx)) {
            enum HeaderRx header = HeaderRx(buf_rx[0]);
            float value = *(float *)(buf_rx + 1);
            switch (header) {
            case RHDR_KP:
                pid_update_Kp(&pid, value);
                break;
            case RHDR_KI:
                pid_update_Ki(&pid, value);
                break;
            case RHDR_KD:
                pid_update_Kd(&pid, value);
                break;
            case RHDR_TSAMPLE:
                pid_update_tSample(&pid, value);
                break;
            case RHDR_HELLO:
                // Reply with parameters
                buf_tx[0] = THDR_KP;
                *(float *)(buf_tx + 1) = pid.Kp;
                queue_try_add(&queue_tx, buf_tx);

                buf_tx[0] = THDR_KD;
                *(float *)(buf_tx + 1) = pid.Kdd * pid.tSample;
                queue_try_add(&queue_tx, buf_tx);

                buf_tx[0] = THDR_KI;
                *(float *)(buf_tx + 1) = pid.Kid / pid.tSample;
                queue_try_add(&queue_tx, buf_tx);

                buf_tx[0] = THDR_TSAMPLE;
                *(float *)(buf_tx + 1) = pid.tSample;
                queue_try_add(&queue_tx, buf_tx);

                buf_tx[0] = THDR_MODE;
                *(float *)(buf_tx + 1) = mode;
                queue_try_add(&queue_tx, buf_tx);
                break;
            case RHDR_MODE:
                mode = Mode((int)value);
                pid_reset(&pidTurn); // Reset turn PID when switching modes
                targetTurn = (mode == MODE_LEFT) * 0.15f + (mode == MODE_RIGHT) * -0.15f;
                break;
            case THDR_CALIBRATE:
                mpu6000_calibrate(value);
                break;
            default:
                break;
            }
        }

        // sleep_ms(1); // Keep small such that tSample is relatively constant -- TODO: interrupts
    }

    return 0;
}
