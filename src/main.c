#include "progcc_includes.h"

const button_remap_s joybus_map = {
    .dpad_up    = MAPCODE_DUP,
    .dpad_down  = MAPCODE_DDOWN,
    .dpad_left  = MAPCODE_DLEFT,
    .dpad_right = MAPCODE_DRIGHT,

    .button_up  = MAPCODE_B_UP,
    .button_down = MAPCODE_B_DOWN,
    .button_left = MAPCODE_B_LEFT,
    .button_right = MAPCODE_B_RIGHT,

    .trigger_l = MAPCODE_T_ZL,
    .trigger_r = MAPCODE_T_ZR,
    .trigger_zl = MAPCODE_T_L,
    .trigger_zr = MAPCODE_T_R,

    .button_start = MAPCODE_B_START,
    .button_select = MAPCODE_B_SELECT,
    .button_stick_left = MAPCODE_B_STICKL,
    .button_stick_right = MAPCODE_B_STICKR,
};

// redefine _write() so that printf() outputs to UART
/*
int _write(int file, char *ptr, int len) {
    int i;
    for (i = 0; i < len; i++) {
        uart_putc_raw(uart0, ptr[i]);
    }
    return len;
}*/

// Set up local input vars
button_data_s button_data = {0};
a_data_s analog_data = {0};
a_data_s scaled_analog_data = {0};

bool calibrate = true;
bool centered = false;

void cb_progcc_hardware_setup()
{
    // Set up GPIO for input buttons
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

    //initialize SPI at 1 MHz
    //initialize SPI at 3 MHz just to test
    spi_init(spi0, 3000*1000);
    gpio_set_function(PGPIO_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PGPIO_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(PGPIO_SPI_RX, GPIO_FUNC_SPI);

    // Left stick initialize
    gpio_init(PGPIO_LS_CS);
    gpio_set_dir(PGPIO_LS_CS, GPIO_OUT);
    gpio_put(PGPIO_LS_CS, true);//active low

    // Right stick initialize
    gpio_init(PGPIO_RS_CS);
    gpio_set_dir(PGPIO_RS_CS, GPIO_OUT);
    gpio_put(PGPIO_RS_CS, true);//active low
}

void cb_progcc_read_buttons()
{
    // Keypad version
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(100);
    button_data.button_a    = !gpio_get(PGPIO_PUSH_C);
    button_data.button_b    = !gpio_get(PGPIO_PUSH_D);
    button_data.button_x    = !gpio_get(PGPIO_PUSH_A);
    button_data.button_y    = !gpio_get(PGPIO_PUSH_B);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(100);
    button_data.dpad_left   = !gpio_get(PGPIO_PUSH_D);
    button_data.dpad_right  = !gpio_get(PGPIO_PUSH_C);
    button_data.dpad_down   = !gpio_get(PGPIO_PUSH_B);
    button_data.dpad_up     = !gpio_get(PGPIO_PUSH_A);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(100);
    button_data.button_plus     = !gpio_get(PGPIO_PUSH_A);
    button_data.button_home     = !gpio_get(PGPIO_PUSH_B);
    button_data.button_capture  = !gpio_get(PGPIO_PUSH_D);
    button_data.button_minus    = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(100);
    button_data.trigger_r   = !gpio_get(PGPIO_PUSH_B);
    button_data.trigger_l   = !gpio_get(PGPIO_PUSH_D);
    button_data.trigger_zl  = !gpio_get(PGPIO_PUSH_A);
    button_data.trigger_zr  = !gpio_get(PGPIO_PUSH_C);
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

void main_two()
{
    for(;;)
    {
        progcc_utils_read_buttons(&button_data);
        progcc_utils_read_sticks(&analog_data);

        if(calibrate)
        {
            if (!centered)
            {
                stick_scaling_capture_center(&analog_data);
                centered = true;
            }

            stick_scaling_capture_distances(&analog_data);

            if (!gpio_get(PGPIO_BUTTON_MODE))
            {
                stick_scaling_finalize();
                calibrate = false;
                progcc_utils_set_rumble(PROGCC_RUMBLE_ON);
                sleep_ms(200);
                progcc_utils_set_rumble(PROGCC_RUMBLE_OFF);
            }
        }
        else
        {
            stick_scaling_process_data(&analog_data, &scaled_analog_data);
            static float la;
            static float ra;

            stick_scaling_get_last_angles(&la, &ra);
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(100);
    board_init();

    printf("ProGCC Started.\n");

    //switch_analog_calibration_init();

    sleep_ms(200);

    // Perform GPIO setup
    progcc_utils_hardware_setup();

    // Handle bootloader stuff
    if(!gpio_get(PGPIO_BUTTON_MODE))
    {
        reset_usb_boot(0, 0);
    }

    rgb_init();

    sleep_ms(200);

    rgb_s red = {
        .r = 0,
        .g = 128,
        .b = 200,
    };
    rgb_set_all(red.color);

    /** Core 0 loop
     * Handles the following
     * - Button reading
     * - Communication protocol
     * - LED animations and updates
     **/

    for(;;)
    {
        uint32_t timestamp = time_us_32();

        progcc_usb_task(&button_data, &scaled_analog_data);
        tud_task();
        rgb_tick(timestamp);
        //printf("Test");
        //sleep_ms(8);
    }

}
