#include "imu.h"

#define IMU_READ_RATE 2350 //400Hz min

// LSM6DSR REGISTERS

#define FUNC_CFG_ACCESS 0x01
#define CTRL1_XL        0x10
#define CTRL2_G         0x11  // Gyro Activation Register - Write 0x60 to enable 416hz
#define CTRL3_C         0x12  // Used to set BDU
#define CTRL4_C         0x13
#define CTRL6_C         0x15
#define CTRL8_XL        0x17
#define CTRL9_XL        0x18
#define CTRL10_C        0x19

#define FUNC_MASK   (0b10000000) // Enable FUNC CFG access
#define CTRL1_MASK  (0b10101110) // 1.66kHz, 8G, output first stage filtering
#define CTRL2_MASK  (0b01011100) // 208Hz, 2000dps
#define CTRL3_MASK  (0b00000100) // BDU enabled and Interrupt out active low
#define CTRL4_MASK  (0b00000100) // I2C disable (Check later for LPF for gyro)
#define CTRL6_MASK  (0b00000000) // 12.2 LPF gyro
#define CTRL8_MASK  (0b11100000) //H P_SLOPE_XL_EN
#define CTRL9_MASK  (0x38)
#define CTRL10_MASK (0x38 | 0x4)

#define SPI_READ_BIT 0x80

#define IMU_OUTX_L_G 0x22
#define IMU_OUTX_L_X 0x28

int16_t imu_x_1[3] = {0};
int16_t imu_y_1[3] = {0};
int16_t imu_z_1[3] = {0};

int16_t imu_x_2[3] = {0};
int16_t imu_y_2[3] = {0};
int16_t imu_z_2[3] = {0};

int16_t imu_x_a[3] = {0};
int16_t imu_y_a[3] = {0};
int16_t imu_z_a[3] = {0};

int16_t acc_x_1[3] = {0};
int16_t acc_y_1[3] = {0};
int16_t acc_z_1[3] = {0};

int16_t acc_x_2[3] = {0};
int16_t acc_y_2[3] = {0};
int16_t acc_z_2[3] = {0};

int16_t acc_x_a[3] = {0};
int16_t acc_y_a[3] = {0};
int16_t acc_z_a[3] = {0};

uint8_t imu_8_x[6] = {0};
uint8_t imu_8_y[6] = {0};
uint8_t imu_8_z[6] = {0};

uint8_t acc_8_x[6] = {0};
uint8_t acc_8_y[6] = {0};
uint8_t acc_8_z[6] = {0};

auto_init_mutex(imu_mutex);

int16_t _imu_x_filter(int16_t new)
{
  static int16_t list[25] = {0};
  static int xavg = 0;
  static int xout = 0;
  static uint8_t idx = 0;

  xavg+=(int) new;
  list[idx] = new;

  uint8_t first = idx+1;
  if(first==25) first = 0;

  xavg+=list[idx];
  xavg-= list[first];

  idx+=1;
  if(idx==25) idx = 0;

  xout = xavg/25;

  return (int16_t) xout;

}

int16_t _imu_concat_16(uint8_t low, uint8_t high)
{
  return (int16_t) ((high<<8) | low);
}

void _imu_process_8(int16_t val, uint8_t *out)
{
  out[0] = (val&0xFF);
  out[1] = (val&0xFF00)>>8;
}

int16_t _imu_average_value (int16_t first, int16_t second)
{
  int total = ((int)first+(int)second)/2;
  if (total>32767) return 32767;
  if (total<-32768) return -32768;
  return total;
}

bool _imu_enabled = false;

// Gets the last 3 IMU datasets and puts them out
// for Nintendo Switch buffer
void imu_buffer_out(uint8_t *output)
{
  uint32_t owner_out;
  while(!mutex_try_enter(&imu_mutex, &owner_out))
  {

  }

  /*
  _imu_x_filter(imu_x_a[0]);
  _imu_x_filter(imu_x_a[1]);
  int16_t gx = _imu_x_filter(imu_x_a[2]);

  _imu_process_8(gx, &imu_8_x[0]);
  _imu_process_8(gx, &imu_8_x[2]);
  _imu_process_8(gx, &imu_8_x[4]);*/

  output[0] = acc_8_y[0];
  output[1] = acc_8_y[1];

  output[2] = acc_8_x[0];
  output[3] = acc_8_x[1];

  output[4] = acc_8_z[0];
  output[5] = acc_8_z[1];

  output[6]   = imu_8_y[0];
  output[7]   = imu_8_y[1];

  output[8]   = imu_8_x[0];
  output[9]   = imu_8_x[1];

  output[10]  = imu_8_z[0];
  output[11]  = imu_8_z[1];

  // Group 2

  output[12] = acc_8_y[2];
  output[13] = acc_8_y[3];

  output[14] = acc_8_x[2];
  output[15] = acc_8_x[3];

  output[16] = acc_8_z[2];
  output[17] = acc_8_z[3];

  output[18]   = imu_8_y[2];
  output[19]   = imu_8_y[3];

  output[20]   = imu_8_x[2];
  output[21]   = imu_8_x[3];

  output[22]  = imu_8_z[2];
  output[23]  = imu_8_z[3];

  // Group 3

  output[24] = acc_8_y[4];
  output[25] = acc_8_y[5];

  output[26] = acc_8_x[4];
  output[27] = acc_8_x[5];

  output[28] = acc_8_z[4];
  output[29] = acc_8_z[5];

  output[30]   = imu_8_y[4];
  output[31]   = imu_8_y[5];

  output[32]   = imu_8_x[4];
  output[33]   = imu_8_x[5];

  output[34]  = imu_8_z[4];
  output[35]  = imu_8_z[5];

  mutex_exit(&imu_mutex);
}

void _imu_write_register(const uint8_t reg, const uint8_t data)
{
  gpio_put(PGPIO_IMU0_CS, false);
  const uint8_t dat[2] = {reg, data};
  spi_write_blocking(spi0, dat, 2);
  gpio_put(PGPIO_IMU0_CS, true);
  sleep_ms(2);

  gpio_put(PGPIO_IMU1_CS, false);
  spi_write_blocking(spi0, dat, 2);
  gpio_put(PGPIO_IMU1_CS, true);
  sleep_ms(2);
}

void imu_init()
{
  _imu_write_register(CTRL1_XL, CTRL1_MASK);
  _imu_write_register(CTRL2_G, CTRL2_MASK);
  _imu_write_register(CTRL3_C, CTRL3_MASK);
  _imu_write_register(CTRL4_C, CTRL4_MASK);
  _imu_write_register(CTRL6_C, CTRL6_MASK);
  _imu_write_register(CTRL8_XL, CTRL8_MASK);
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

uint8_t imu_read_idx = 0;

void imu_reset_idx()
{
  imu_read_idx = 0;
}

void imu_set_enabled(bool enable)
{
  _imu_enabled = enable;
  imu_reset_idx();
}

bool _flip = false;

void imu_read_test(uint32_t timestamp)
{
  if (!_imu_enabled && (imu_read_idx < 3)) return;

  if(_imu_update_ready(timestamp))
  {
    _flip = !_flip;

    uint8_t i[12] = {0};
    const uint8_t reg = 0x80 | IMU_OUTX_L_G;

    gpio_put(PGPIO_IMU0_CS, false);
    spi_write_blocking(spi0, &reg, 1);
    spi_read_blocking(spi0, 0, &i[0], 12);
    gpio_put(PGPIO_IMU0_CS, true);

    imu_x_1[imu_read_idx] = -_imu_concat_16(i[0], i[1]);
    imu_y_1[imu_read_idx] = _imu_concat_16(i[2], i[3]);
    imu_z_1[imu_read_idx] = _imu_concat_16(i[4], i[5]);

    acc_x_1[imu_read_idx] = -_imu_concat_16(i[6], i[7]);
    acc_y_1[imu_read_idx] = _imu_concat_16(i[8], i[9]);
    acc_z_1[imu_read_idx] = _imu_concat_16(i[10], i[11]);

    gpio_put(PGPIO_IMU1_CS, false);
    spi_write_blocking(spi0, &reg, 1);
    spi_read_blocking(spi0, 0, &i[0], 12);
    gpio_put(PGPIO_IMU1_CS, true);

    imu_x_2[imu_read_idx] = _imu_concat_16(i[0], i[1]);
    imu_y_2[imu_read_idx] = -_imu_concat_16(i[2], i[3]);
    imu_z_2[imu_read_idx] = _imu_concat_16(i[4], i[5]);

    acc_x_2[imu_read_idx] = _imu_concat_16(i[6], i[7]);
    acc_y_2[imu_read_idx] = -_imu_concat_16(i[8], i[9]);
    acc_z_2[imu_read_idx] = _imu_concat_16(i[10], i[11]);

    imu_x_a[imu_read_idx] = _imu_average_value(imu_x_1[imu_read_idx], imu_x_2[imu_read_idx]);
    imu_y_a[imu_read_idx] = _imu_average_value(imu_y_1[imu_read_idx], imu_y_2[imu_read_idx]);
    imu_z_a[imu_read_idx] = _imu_average_value(imu_z_1[imu_read_idx], imu_z_2[imu_read_idx]);

    acc_x_a[imu_read_idx] = _imu_average_value(acc_x_1[imu_read_idx], acc_x_2[imu_read_idx]);
    acc_y_a[imu_read_idx] = _imu_average_value(acc_y_1[imu_read_idx], acc_y_2[imu_read_idx]);
    acc_z_a[imu_read_idx] = _imu_average_value(acc_z_1[imu_read_idx], acc_z_2[imu_read_idx]);

    uint32_t owner_out;
    while(!mutex_try_enter(&imu_mutex, &owner_out))
    {

    }

    uint8_t o = imu_read_idx*2;

    _imu_process_8(imu_x_a[imu_read_idx], &imu_8_x[o]);
    _imu_process_8(imu_y_a[imu_read_idx], &imu_8_y[o]);
    _imu_process_8(imu_z_a[imu_read_idx], &imu_8_z[o]);

    _imu_process_8(acc_x_a[imu_read_idx], &acc_8_x[o]);
    _imu_process_8(acc_y_a[imu_read_idx], &acc_8_y[o]);
    _imu_process_8(acc_z_a[imu_read_idx], &acc_8_z[o]);

    mutex_exit(&imu_mutex);

    if (imu_read_idx<3) imu_read_idx++;
  }
}
