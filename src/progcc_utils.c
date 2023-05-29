#include "progcc_utils.h"

#define X_AXIS_CONFIG 0xD0
#define Y_AXIS_CONFIG 0xF0
#define BUFFER_TO_UINT16(buffer) (uint16_t)(((buffer[0] & 0x07) << 9) | buffer[1] << 1 | buffer[2] >> 7)

void progcc_utils_setup_gpio_scan(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_OUT);
  gpio_put(gpio, true);
}

void progcc_utils_setup_gpio_push(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  //printf("Set up GPIO: %d", (uint8_t) gpio);
}

void progcc_utils_setup_gpio_button(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  //printf("Set up GPIO: %d", (uint8_t) gpio);
}

uint main_slice_num = 0;
uint brake_slice_num = 0;

void progcc_utils_hardware_setup(void)
{
  // Set up GPIO Inputs
  #if PROGCC_USE_KEYPAD
    progcc_utils_setup_gpio_button(PGPIO_BUTTON_RS);
    progcc_utils_setup_gpio_button(PGPIO_BUTTON_LS);
    progcc_utils_setup_gpio_button(PGPIO_BUTTON_MODE);

    progcc_utils_setup_gpio_push(PGPIO_PUSH_A);
    progcc_utils_setup_gpio_push(PGPIO_PUSH_B);
    progcc_utils_setup_gpio_push(PGPIO_PUSH_C);
    progcc_utils_setup_gpio_push(PGPIO_PUSH_D);

    progcc_utils_setup_gpio_scan(PGPIO_SCAN_A);
    progcc_utils_setup_gpio_scan(PGPIO_SCAN_B);
    progcc_utils_setup_gpio_scan(PGPIO_SCAN_C);
    progcc_utils_setup_gpio_scan(PGPIO_SCAN_D);
  #else
    // Iterate through the const array and initialize buttons gpio
    for(uint8_t i = 0; i < 12; i++)
    {
      progcc_utils_setup_gpio_button(PGPIO_INPUT_ARRAY[i]);
    }
  #endif

  // Set up Rumble GPIO

  gpio_init(PGPIO_RUMBLE_MAIN);
  gpio_init(PGPIO_RUMBLE_BRAKE);

  gpio_set_dir(PGPIO_RUMBLE_MAIN, GPIO_OUT);
  gpio_set_dir(PGPIO_RUMBLE_BRAKE, GPIO_OUT);

  gpio_set_function(PGPIO_RUMBLE_MAIN, GPIO_FUNC_PWM);
  gpio_set_function(PGPIO_RUMBLE_BRAKE, GPIO_FUNC_PWM);

  main_slice_num   = pwm_gpio_to_slice_num(PGPIO_RUMBLE_MAIN);
  brake_slice_num  = pwm_gpio_to_slice_num(PGPIO_RUMBLE_BRAKE);

  pwm_set_wrap(main_slice_num, 255);
  pwm_set_wrap(brake_slice_num,  255);

  pwm_set_chan_level(main_slice_num, PWM_CHAN_B, 0);//B for odd pins
  pwm_set_chan_level(brake_slice_num,  PWM_CHAN_B, 255);//B for odd pins

  pwm_set_enabled(main_slice_num,   true);
  pwm_set_enabled(brake_slice_num,  true);

  pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 255);
  pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 0);

  sleep_us(150); // Stabilize voltages

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
  // Keypad version
  gpio_put(PGPIO_SCAN_A, false);
  sleep_us(100);
  data->button_a    = !gpio_get(PGPIO_PUSH_C);
  data->button_b    = !gpio_get(PGPIO_PUSH_D);
  data->button_x    = !gpio_get(PGPIO_PUSH_A);
  data->button_y    = !gpio_get(PGPIO_PUSH_B);
  gpio_put(PGPIO_SCAN_A, true);

  gpio_put(PGPIO_SCAN_B, false);
  sleep_us(100);
  data->dpad_left   = !gpio_get(PGPIO_PUSH_D);
  data->dpad_right  = !gpio_get(PGPIO_PUSH_C);
  data->dpad_down   = !gpio_get(PGPIO_PUSH_B);
  data->dpad_up     = !gpio_get(PGPIO_PUSH_A);
  gpio_put(PGPIO_SCAN_B, true);

  gpio_put(PGPIO_SCAN_C, false);
  sleep_us(100);
  data->button_plus     = !gpio_get(PGPIO_PUSH_B);
  data->button_home     = !gpio_get(PGPIO_PUSH_A);
  data->button_capture  = !gpio_get(PGPIO_PUSH_D);
  data->button_minus    = !gpio_get(PGPIO_PUSH_C);
  gpio_put(PGPIO_SCAN_C, true);

  gpio_put(PGPIO_SCAN_D, false);
  sleep_us(100);
  data->trigger_r   = !gpio_get(PGPIO_PUSH_B);
  data->trigger_l   = !gpio_get(PGPIO_PUSH_D);
  data->trigger_zl  = !gpio_get(PGPIO_PUSH_A);
  data->trigger_zr  = !gpio_get(PGPIO_PUSH_C);
  gpio_put(PGPIO_SCAN_D, true);

  data->button_stick_right = !gpio_get(PGPIO_BUTTON_RS);
  data->button_stick_left = !gpio_get(PGPIO_BUTTON_LS);

  /*
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
  data->button_plus = !gpio_get(PGPIO_BUTTON_START);*/
}

void progcc_utils_read_sticks(a_data_s *data)
{
  // Set up buffers for each axis
  uint8_t buffer_lx[3] = {0};
  uint8_t buffer_ly[3] = {0};
  uint8_t buffer_rx[3] = {0};
  uint8_t buffer_ry[3] = {0};

  // CS left stick ADC
  gpio_put(PGPIO_LS_CS, false);
  // Read first axis for left stick
  spi_read_blocking(spi0, X_AXIS_CONFIG, buffer_lx, 3);

  // CS left stick ADC reset
  gpio_put(PGPIO_LS_CS, true);
  gpio_put(PGPIO_LS_CS, false);

  // Set up and read axis for left stick Y  axis
  spi_read_blocking(spi0, Y_AXIS_CONFIG, buffer_ly, 3);

  // CS right stick ADC
  gpio_put(PGPIO_LS_CS, true);
  gpio_put(PGPIO_RS_CS, false);

  spi_read_blocking(spi0, Y_AXIS_CONFIG, buffer_ry, 3);

  // CS right stick ADC reset
  gpio_put(PGPIO_RS_CS, true);
  gpio_put(PGPIO_RS_CS, false);

  spi_read_blocking(spi0, X_AXIS_CONFIG, buffer_rx, 3);

  // Release right stick CS ADC
  gpio_put(PGPIO_RS_CS, true);

  // Convert data
  data->lx = BUFFER_TO_UINT16(buffer_lx);
  data->ly = BUFFER_TO_UINT16(buffer_ly);
  data->rx = BUFFER_TO_UINT16(buffer_rx);
  data->ry = BUFFER_TO_UINT16(buffer_ry);
}

void progcc_utils_set_rumble(progcc_rumble_t rumble)
{
  switch(rumble)
  {
    default:
    case PROGCC_RUMBLE_OFF:
    case PROGCC_RUMBLE_BRAKE:
      pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 0);
      pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 255);
      break;

    case PROGCC_RUMBLE_ON:
      pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 0);
      pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 150);
      break;
  }
}
