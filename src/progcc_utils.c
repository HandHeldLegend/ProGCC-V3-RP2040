#include "progcc_utils.h"

#define X_AXIS_CONFIG 0xD0
#define Y_AXIS_CONFIG 0xF0
#define BUFFER_TO_UINT16(buffer) (uint16_t)(((buffer[0] & 0b00000111) << 9) | buffer[1] << 1 | buffer[2] >> 7)

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
  data->left_stick_x  = BUFFER_TO_UINT16(buffer_lx);
  data->left_stick_y  = BUFFER_TO_UINT16(buffer_ly);
  data->right_stick_x = BUFFER_TO_UINT16(buffer_rx);
  data->right_stick_y = BUFFER_TO_UINT16(buffer_ry);
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

#define ADC_CENTER_VAL_U8 128

uint8_t progcc_utils_scale_stick(uint16_t val, uint16_t center, float scaler_low, float scaler_high)
{
  float f_val = val;
  float f_center = center;
  float dist = 0;
  uint8_t dist_8 = 0;

  if (val > center)
  {
    // Calculate distance between value and center point.
    dist = f_val - f_center;

    // Scale the input
    dist *= scaler_high;

    dist_8 = dist;

    if (dist_8 > 127)
    {
      return 255;
    }
    else
    {
      return (ADC_CENTER_VAL_U8 + dist_8);
    }
  }
  else if (val < center)
  {
    // Calculate distance between value and center point.
    dist = f_center - f_val;

    // Scale the input
    dist *= scaler_high;

    dist_8 = dist;

    if (dist_8 > 128)
    {
      return 0;
    }
    else
    {
      return (ADC_CENTER_VAL_U8 - dist_8);
    }
  }
  else return ADC_CENTER_VAL_U8;
}

void progcc_utils_scale_sticks(progcc_analog_data_s *data_in,
                                progcc_analog_data_s *data_out,
                                progcc_analog_calibration_data_s *calibration_data,
                                progcc_analog_scaler_data_s *scaler_data)
{
  uint8_t lx_scaled = progcc_utils_scale_stick(data_in->left_stick_x, calibration_data->ls_x_center,
                                              scaler_data->ls_x_low_scaler, scaler_data->ls_x_high_scaler);

  uint8_t ly_scaled = progcc_utils_scale_stick(data_in->left_stick_y, calibration_data->ls_y_center,
                                              scaler_data->ls_y_low_scaler, scaler_data->ls_y_high_scaler);

  uint8_t rx_scaled = progcc_utils_scale_stick(data_in->right_stick_x, calibration_data->rs_x_center,
                                              scaler_data->rs_x_low_scaler, scaler_data->rs_x_high_scaler);

  uint8_t ry_scaled = progcc_utils_scale_stick(data_in->right_stick_y, calibration_data->rs_y_center,
                                              scaler_data->rs_y_low_scaler, scaler_data->rs_y_high_scaler);

  data_out->left_stick_x  = lx_scaled;
  data_out->left_stick_y  = lx_scaled;
  data_out->right_stick_x = rx_scaled;
  data_out->right_stick_y = ry_scaled;
}

float subtract_return_as_float(uint16_t high_val, uint16_t low_val)
{
  float v1 = high_val;
  float v2 = low_val;

  float out = v1 - v2;
  return out;
}

// Function that pre-calculates the stick scalers that are used.
void progcc_utils_calculate_scalers(progcc_analog_calibration_data_s *calibration_data, progcc_analog_scaler_data_s *scaler_data)
{
  // Set up const for scaling
  float scaling_val = 127.0f;

  scaler_data->ls_x_high_scaler = scaling_val / subtract_return_as_float(calibration_data->ls_x_high, calibration_data->ls_x_center);
  scaler_data->ls_x_low_scaler = scaling_val / subtract_return_as_float(calibration_data->ls_x_center, calibration_data->ls_x_low);

  scaler_data->ls_y_high_scaler = scaling_val / subtract_return_as_float(calibration_data->ls_y_high, calibration_data->ls_y_center);
  scaler_data->ls_y_low_scaler = scaling_val / subtract_return_as_float(calibration_data->ls_y_center, calibration_data->ls_y_low);

  scaler_data->rs_x_high_scaler = scaling_val / subtract_return_as_float(calibration_data->rs_x_high, calibration_data->rs_x_center);
  scaler_data->rs_x_low_scaler = scaling_val / subtract_return_as_float(calibration_data->rs_x_center, calibration_data->rs_x_low);

  scaler_data->rs_y_high_scaler = scaling_val / subtract_return_as_float(calibration_data->rs_y_high, calibration_data->rs_y_center);
  scaler_data->rs_y_low_scaler = scaling_val / subtract_return_as_float(calibration_data->rs_y_center, calibration_data->rs_y_low);
}

uint16_t min_val(uint16_t new, uint16_t old)
{
  if (new < old)
  {
    return new;
  }
  else return old;
}

uint16_t max_val(uint16_t new, uint16_t old)
{
  if (new > old)
  {
    return new;
  }
  else return old;
}

bool started = false;

void progcc_utils_calibration_capture(progcc_analog_data_s *data, progcc_analog_calibration_data_s *calibration_data)
{
  if (!started)
  {
    calibration_data->ls_x_center = data->left_stick_x;
    calibration_data->ls_x_high = data->left_stick_x;
    calibration_data->ls_x_low = data->left_stick_x;

    calibration_data->ls_y_center = data->left_stick_y;
    calibration_data->ls_y_high = data->left_stick_y;
    calibration_data->ls_y_low = data->left_stick_y;

    calibration_data->rs_x_center = data->right_stick_x;
    calibration_data->rs_x_high = data->right_stick_x;
    calibration_data->rs_x_low = data->right_stick_x;

    calibration_data->rs_y_center = data->right_stick_y;
    calibration_data->rs_y_high = data->right_stick_y;
    calibration_data->rs_y_low = data->right_stick_y;

    started = true;
  }
  else
  {
    calibration_data->ls_x_high = max_val(data->left_stick_x, calibration_data->ls_x_high);
    calibration_data->ls_x_low  = min_val(data->left_stick_x, calibration_data->ls_x_low);

    calibration_data->ls_y_high = max_val(data->left_stick_y, calibration_data->ls_y_high);
    calibration_data->ls_y_low  = min_val(data->left_stick_y, calibration_data->ls_y_low);

    calibration_data->rs_x_high = max_val(data->right_stick_x, calibration_data->rs_x_high);
    calibration_data->rs_x_low  = min_val(data->right_stick_x, calibration_data->rs_x_low);

    calibration_data->rs_y_high = max_val(data->right_stick_y, calibration_data->rs_y_high);
    calibration_data->rs_y_low  = min_val(data->right_stick_y, calibration_data->rs_y_low);
  }
}
