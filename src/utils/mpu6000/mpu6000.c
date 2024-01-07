#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <string.h>

#include "utils/mpu6000.h"


// https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
// https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
// The MPU-60X0 automatically increments the register address and loads the data to the appropriate register
//  --> read multiple consecutive bytes without setting the register
static uint8_t address = 0b1101000;

// Registers
static uint8_t PWR_MGMT_1 = 0x6B;
static uint8_t SIGNAL_PATH_RESET = 0x68;
static uint8_t GYRO_CONFIG = 0x1B;
static uint8_t ACCEL_CONFIG = 0x1C;

static uint8_t ACCEL_XOUT_H = 0x3B;
static uint8_t GYRO_XOUT_H = 0x43;


static bool calibrated = false;
static int16_t GYRO_CAL[3];
static int16_t ACCEL_CAL[3];


void
mpu6000_reset()
{
    // Reset the Device
    uint8_t buf[2] = {PWR_MGMT_1, 1 << 7};
    i2c_write_blocking(i2c_default, address, buf, 2, false);
    sleep_ms(100);

    // Reset the analog and digital signal paths of the gyroscope and accelerometer sensors
    buf[0] = SIGNAL_PATH_RESET;
    buf[1] = 1 << 2 | 1 << 1;
    i2c_write_blocking(i2c_default, address, buf, 2, false);
    sleep_ms(100);

    // Disable temperature
    buf[0] = PWR_MGMT_1;
    buf[1] = 1 << 3;
    i2c_write_blocking(i2c_default, address, buf, 2, false);
    sleep_ms(100);

    // Set gyro full scale range to 500 degrees/s
    buf[0] = GYRO_CONFIG;
    buf[1] = 0b00001000;
    i2c_write_blocking(i2c_default, address, buf, 2, false);

    // Set accelerometer full scale range to +-2g
    buf[0] = ACCEL_CONFIG;
    buf[1] = 0b00000000;
    i2c_write_blocking(i2c_default, address, buf, 2, false);
}


void
mpu6000_calibrate(int nCalibrations)
{
    // TODO: add delay to avoid repeated readings

    calibrated = false; // This value is used in mpu6000_read

    int16_t accel[3];
    int16_t gyro[3];

    int64_t gyro_temp[3] = {0, 0, 0};
    int64_t accel_temp[3] = {0, 0, 0};

    // Ignore first 100 readings
    for (int i=0; i<100; ++i)
        mpu6000_read(accel, gyro);

    for (int i=0; i<nCalibrations; ++i) {
        mpu6000_read(accel, gyro);
        gyro_temp[0] += gyro[0];
        gyro_temp[1] += gyro[1];
        gyro_temp[2] += gyro[2];

        accel_temp[0] += accel[0];
        accel_temp[1] += accel[1];
        accel_temp[2] += accel[2];
    }

    GYRO_CAL[0] = gyro_temp[0] / nCalibrations;
    GYRO_CAL[1] = gyro_temp[1] / nCalibrations;
    GYRO_CAL[2] = gyro_temp[2] / nCalibrations;

    ACCEL_CAL[0] = accel_temp[0] / nCalibrations;
    ACCEL_CAL[1] = accel_temp[1] / nCalibrations;
    ACCEL_CAL[2] = accel_temp[2] / nCalibrations;

    calibrated = true;
}


/**
 * @brief Read 16 bit accelerometer (x, y, z), gyroscope (x, y, z)
 * 
 * @param accel 
 * @param gyro 
 */
bool
mpu6000_read(int16_t accel[3], int16_t gyro[3])
{
    int read = 0;
    int written = 0;

    uint8_t buffer[6];

    // Acceleration
    // In upright position: // TODO: change for flat position
    //      x -> 0g
    //      y -> -1g
    //      z -> 0g
    written += i2c_write_blocking(i2c_default, address, &ACCEL_XOUT_H, 1, true); // True to keep master control of bus
    read += i2c_read_blocking(i2c_default, address, buffer, 6, false); // False - finished with bus
    accel[0] = (buffer[0 * 2] << 8 | buffer[(0 * 2) + 1]) - calibrated * (ACCEL_CAL[0]);
    accel[1] = (buffer[1 * 2] << 8 | buffer[(1 * 2) + 1]) - calibrated * (-16384-ACCEL_CAL[1]);
    accel[2] = (buffer[2 * 2] << 8 | buffer[(2 * 2) + 1]) - calibrated * (ACCEL_CAL[2]);

    // Gyroscope
    written += i2c_write_blocking(i2c_default, address, &GYRO_XOUT_H, 1, true); // True to keep master control of bus
    read += i2c_read_blocking(i2c_default, address, buffer, 6, false);  // False - finished with bus
    for (int i = 0; i < 3; i++)
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]) - calibrated * GYRO_CAL[i];

    return read == 12 && written == 2;
}
