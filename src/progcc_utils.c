#include "progcc_utils.h"

#define X_AXIS_CONFIG 0xD0
#define Y_AXIS_CONFIG 0xF0
#define BUFFER_TO_UINT16(buffer) (((uint16_t)(buffer[1] & 0x0F) << 8) | buffer[2])

void progcc_utils_setup_gpio_button(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  //printf("Set up GPIO: %d", (uint8_t) gpio);
}

void progcc_utils_hardware_setup(void)
{
  // Set up GPIO Inputs
  #if PROGCC_USE_KEYPAD
  #else
    // Iterate through the const array and initialize buttons gpio
    for(uint8_t i = 0; i < 12; i++)
    {
      progcc_utils_setup_gpio_button(PGPIO_INPUT_ARRAY[i]);
    }
  #endif

  // Set up Rumble GPIO
  // Taken from PhobGCC-SW code
  gpio_init(PGPIO_RUMBLE_MAIN);
  gpio_init(PGPIO_RUMBLE_BRAKE);
  gpio_set_dir(PGPIO_RUMBLE_MAIN, GPIO_OUT);
  gpio_set_dir(PGPIO_RUMBLE_BRAKE, GPIO_OUT);
  gpio_set_function(PGPIO_RUMBLE_MAIN, GPIO_FUNC_PWM);
  gpio_set_function(PGPIO_RUMBLE_BRAKE, GPIO_FUNC_PWM);

  const uint rumbleSlice_num = pwm_gpio_to_slice_num(PGPIO_RUMBLE_MAIN);
  const uint brakeSlice_num  = pwm_gpio_to_slice_num(PGPIO_RUMBLE_BRAKE);

  pwm_set_wrap(rumbleSlice_num, 255);
  pwm_set_wrap(brakeSlice_num,  255);

  pwm_set_chan_level(rumbleSlice_num, PWM_CHAN_B, 0);//B for odd pins
  pwm_set_chan_level(brakeSlice_num,  PWM_CHAN_B, 255);//B for odd pins

  pwm_set_enabled(rumbleSlice_num, true);
  pwm_set_enabled(brakeSlice_num,  true);

  #if PROGCC_USE_SPI_ADC
    //initialize SPI at 1 MHz
    //initialize SPI at 3 MHz just to test
    spi_init(spi0, 3000*1000);
    gpio_set_function(PGPIO_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PGPIO_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(PGPIO_SPI_RX, GPIO_FUNC_SPI);

    // Left stick
    gpio_init(PGPIO_LS_CS);
    gpio_set_dir(PGPIO_LS_CS, GPIO_OUT);
    gpio_put(PGPIO_LS_CS, true);//active low

    // Right stick
    gpio_init(PGPIO_RS_CS);
    gpio_set_dir(PGPIO_RS_CS, GPIO_OUT);
    gpio_put(PGPIO_RS_CS, true);//active low
  #endif




}

void progcc_utils_read_buttons(progcc_button_data_s *data)
{
  data->button_a  = !gpio_get(PGPIO_BUTTON_A);
  data->button_b  = !gpio_get(PGPIO_BUTTON_B);
  data->button_x  = !gpio_get(PGPIO_BUTTON_X);
  data->button_y  = !gpio_get(PGPIO_BUTTON_Y);

  data->dpad_up     = !gpio_get(PGPIO_BUTTON_DUP);
  data->dpad_down   = !gpio_get(PGPIO_BUTTON_DDOWN);
  data->dpad_left   = !gpio_get(PGPIO_BUTTON_DLEFT);
  data->dpad_right  = !gpio_get(PGPIO_BUTTON_DRIGHT);

  data->trigger_zl  = !gpio_get(PGPIO_BUTTON_L);
  data->trigger_zr  = !gpio_get(PGPIO_BUTTON_R);
  data->trigger_r   = !gpio_get(PGPIO_BUTTON_Z);
  data->button_plus = !gpio_get(PGPIO_BUTTON_START);
}

void progcc_utils_read_sticks(progcc_analog_data_s *data)
{
  // Set up buffers for each axis
  uint8_t buffer_lx[3] = {0};
  uint8_t buffer_ly[3] = {0};
  uint8_t buffer_rx[3] = {0};
  uint8_t buffer_ry[3] = {0};

  gpio_put(PGPIO_LS_CS, false);

  // Read first axis for left stick
  spi_read_blocking(spi0, X_AXIS_CONFIG, buffer_lx, 3);

  // Set up and read axis for left stick Y  axis
  spi_read_blocking(spi0, Y_AXIS_CONFIG, buffer_ly, 3);

  gpio_put(PGPIO_LS_CS, true);
  gpio_put(PGPIO_RS_CS, false);

  spi_read_blocking(spi0, Y_AXIS_CONFIG, buffer_ry, 3);

  spi_read_blocking(spi0, X_AXIS_CONFIG, buffer_rx, 3);

  gpio_put(PGPIO_RS_CS, true);

  // Convert data
  data->left_stick_x  = BUFFER_TO_UINT16(buffer_lx);
  data->left_stick_y  = BUFFER_TO_UINT16(buffer_lx);
  data->right_stick_x = BUFFER_TO_UINT16(buffer_rx);
  data->right_stick_y = BUFFER_TO_UINT16(buffer_ry);
}
