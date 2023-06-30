#include "progcc_includes.h"

#define X_AXIS_CONFIG 0xD0
#define Y_AXIS_CONFIG 0xF0
#define BUFFER_TO_UINT16(buffer) (uint16_t)(((buffer[0] & 0x07) << 9) | buffer[1] << 1 | buffer[2] >> 7)

button_remap_s user_map = {
    .dpad_up = MAPCODE_DUP,
    .dpad_down = MAPCODE_DDOWN,
    .dpad_left = MAPCODE_DLEFT,
    .dpad_right = MAPCODE_DRIGHT,

    .button_a = MAPCODE_B_A,
    .button_b = MAPCODE_B_B,
    .button_x = MAPCODE_B_X,
    .button_y = MAPCODE_B_Y,

    .trigger_l = MAPCODE_T_ZL,
    .trigger_r = MAPCODE_T_ZR,
    .trigger_zl = MAPCODE_T_L,
    .trigger_zr = MAPCODE_T_R,

    .button_plus = MAPCODE_B_PLUS,
    .button_minus = MAPCODE_B_MINUS,
    .button_stick_left = MAPCODE_B_STICKL,
    .button_stick_right = MAPCODE_B_STICKR,
};

// Set up local input vars
button_data_s button_data = {0};
a_data_s analog_data = {0};

uint main_slice_num = 0;
uint brake_slice_num = 0;

void cb_progcc_hardware_setup()
{
    // Set up GPIO for input buttons
    progcc_setup_gpio_button(PGPIO_BUTTON_RS);
    progcc_setup_gpio_button(PGPIO_BUTTON_LS);

    progcc_setup_gpio_push(PGPIO_PUSH_A);
    progcc_setup_gpio_push(PGPIO_PUSH_B);
    progcc_setup_gpio_push(PGPIO_PUSH_C);
    progcc_setup_gpio_push(PGPIO_PUSH_D);

    progcc_setup_gpio_scan(PGPIO_SCAN_A);
    progcc_setup_gpio_scan(PGPIO_SCAN_B);
    progcc_setup_gpio_scan(PGPIO_SCAN_C);
    progcc_setup_gpio_scan(PGPIO_SCAN_D);

    // Set up Rumble GPIO
    gpio_init(PGPIO_RUMBLE_MAIN);
    gpio_init(PGPIO_RUMBLE_BRAKE);

    gpio_set_dir(PGPIO_RUMBLE_MAIN, GPIO_OUT);
    gpio_set_dir(PGPIO_RUMBLE_BRAKE, GPIO_OUT);

    gpio_set_function(PGPIO_RUMBLE_MAIN, GPIO_FUNC_PWM);
    gpio_set_function(PGPIO_RUMBLE_BRAKE, GPIO_FUNC_PWM);

    main_slice_num = pwm_gpio_to_slice_num(PGPIO_RUMBLE_MAIN);
    brake_slice_num = pwm_gpio_to_slice_num(PGPIO_RUMBLE_BRAKE);

    pwm_set_wrap(main_slice_num, 255);
    pwm_set_wrap(brake_slice_num, 255);

    pwm_set_chan_level(main_slice_num, PWM_CHAN_B, 0);    // B for odd pins
    pwm_set_chan_level(brake_slice_num, PWM_CHAN_B, 255); // B for odd pins

    pwm_set_enabled(main_slice_num, true);
    pwm_set_enabled(brake_slice_num, true);

    pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 255);
    pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 0);

    sleep_us(150); // Stabilize voltages

    // initialize SPI at 1 MHz
    // initialize SPI at 3 MHz just to test
    spi_init(spi0, 10 * 1000 * 1000);
    gpio_set_function(PGPIO_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PGPIO_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(PGPIO_SPI_RX, GPIO_FUNC_SPI);

    // Left stick initialize
    gpio_init(PGPIO_LS_CS);
    gpio_set_dir(PGPIO_LS_CS, GPIO_OUT);
    gpio_put(PGPIO_LS_CS, true); // active low

    // Right stick initialize
    gpio_init(PGPIO_RS_CS);
    gpio_set_dir(PGPIO_RS_CS, GPIO_OUT);
    gpio_put(PGPIO_RS_CS, true); // active low

    // IMU 0 initialize
    gpio_init(PGPIO_IMU0_CS);
    gpio_set_dir(PGPIO_IMU0_CS, GPIO_OUT);
    gpio_put(PGPIO_IMU0_CS, true); // active low

    // IMU 1 initialize
    gpio_init(PGPIO_IMU1_CS);
    gpio_set_dir(PGPIO_IMU1_CS, GPIO_OUT);
    gpio_put(PGPIO_IMU1_CS, true); // active low

    imu_init();
}

void cb_progcc_rumble_enable(bool enable)
{
    if (enable)
    {
        pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 0);
        pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 120);
    }
    else
    {
        pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 0);
        pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 255);
    }
}

void cb_progcc_read_buttons()
{
    // Keypad version
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(100);
    button_data.button_a = !gpio_get(PGPIO_PUSH_C);
    button_data.button_b = !gpio_get(PGPIO_PUSH_D);
    button_data.button_x = !gpio_get(PGPIO_PUSH_A);
    button_data.button_y = !gpio_get(PGPIO_PUSH_B);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(100);
    button_data.dpad_left = !gpio_get(PGPIO_PUSH_D);
    button_data.dpad_right = !gpio_get(PGPIO_PUSH_C);
    button_data.dpad_down = !gpio_get(PGPIO_PUSH_B);
    button_data.dpad_up = !gpio_get(PGPIO_PUSH_A);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(100);
    button_data.button_plus = !gpio_get(PGPIO_PUSH_A);
    button_data.button_home = !gpio_get(PGPIO_PUSH_B);
    button_data.button_capture = !gpio_get(PGPIO_PUSH_D);
    button_data.button_minus = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(100);
    button_data.trigger_r = !gpio_get(PGPIO_PUSH_B);
    button_data.trigger_l = !gpio_get(PGPIO_PUSH_D);
    button_data.trigger_zl = !gpio_get(PGPIO_PUSH_A);
    button_data.trigger_zr = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_D, true);

    button_data.button_stick_right = !gpio_get(PGPIO_BUTTON_RS);
    button_data.button_stick_left = !gpio_get(PGPIO_BUTTON_LS);
}

void cb_progcc_read_analog()
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
    analog_data.lx = BUFFER_TO_UINT16(buffer_lx);
    analog_data.ly = BUFFER_TO_UINT16(buffer_ly);
    analog_data.rx = BUFFER_TO_UINT16(buffer_rx);
    analog_data.ry = BUFFER_TO_UINT16(buffer_ry);
}

void cb_progcc_task_0_hook(uint32_t timestamp)
{
    rgb_tick(timestamp);
}

int main()
{
    stdio_init_all();
    sleep_ms(100);
    //board_init();

    printf("ProGCC Started.\n");

    progcc_setup_gpio_button(PGPIO_BUTTON_MODE);
    // Handle bootloader stuff
    if (!gpio_get(PGPIO_BUTTON_MODE))
    {
        reset_usb_boot(0, 0);
    }

    rgb_init();

    sleep_ms(200);

    rgb_s red = {
        .r = 150,
        .g = 128,
        .b = 200,
    };
    rgb_set_all(red.color);

    progcc_init(&button_data, &analog_data, &user_map);
}
