#include "imu.h"

#define IMU_READ_RATE 500000

// LSM6DSR REGISTERS
#define IMU_CTRL2_G 0x11    // Gyro Activation Register
#define IMU_CTRLL1_LX 0x10  // Accelerometer Activation Register

#define ACTIVATE_IMU 0xA2

#define IMU_OUTX_L_G 0x22

uint16_t imu_x = 0;
uint16_t imu_y = 0;
uint16_t imu_z = 0;

uint16_t acc_x = 0;
uint16_t acc_y = 0;
uint16_t acc_z = 0;

void imu_init()
{
  gpio_put(PGPIO_IMU0_CS, false);
  const uint8_t dat0[2] = {IMU_CTRL2_G, ACTIVATE_IMU};
  spi_write_blocking(spi0, dat0, 2);
  gpio_put(PGPIO_IMU0_CS, true);
  sleep_ms(2);

  gpio_put(PGPIO_IMU0_CS, false);
  const uint8_t dat1[2] = {IMU_CTRLL1_LX, ACTIVATE_IMU};
  spi_write_blocking(spi0, dat1, 2);
  gpio_put(PGPIO_IMU0_CS, true);
  sleep_ms(2);
}

bool _imu_update_ready(uint32_t timestamp)
{
  static uint32_t last_time = 0;
  static uint32_t this_time = 0;

  this_time = timestamp;

  // Clear variable
  uint32_t diff = 0;

  // Handle edge case where time has
  // looped around and is now less
  if (this_time < last_time)
  {
    diff = (0xFFFFFFFF - last_time) + this_time;
  }
  else if (this_time > last_time)
  {
    diff = this_time - last_time;
  }
  else
    return false;

  // We want a target IMU rate defined
  if (diff > IMU_READ_RATE)
  {
    // Set the last time
    last_time = this_time;
    return true;
  }
  return false;
}



void imu_read_test(uint32_t timestamp)
{
  if(!_imu_update_ready(timestamp)) return;

  uint8_t i[12] = {0};
  gpio_put(PGPIO_IMU0_CS, false);
  uint8_t reg = 0x80 | IMU_OUTX_L_G;
  spi_write_blocking(spi0, &reg, 1);
  spi_read_blocking(spi0, 0, &i[0], 12);
  gpio_put(PGPIO_IMU0_CS, true);

  imu_x = (i[1]<<8) | i[0];
  imu_y = (i[3]<<8) | i[2];
  imu_z = (i[5]<<8) | i[4];

  acc_x = (i[7]<<8) | i[6];
  acc_y = (i[9]<<8) | i[8];
  acc_z = (i[11]<<8) | i[10];
}
