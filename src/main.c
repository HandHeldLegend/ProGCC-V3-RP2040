#include "progcc_includes.h"

// Set up local input vars
progcc_button_data_s button_data = {0};
progcc_analog_data_s analog_data = {0};
progcc_analog_data_s analog_data_scaled = {0};
progcc_analog_calibration_data_s calibration_data = {0};
progcc_analog_scaler_data_s scaler_data = {0};

bool calibrate = true;

void main_two()
{
    for(;;)
    {
        progcc_utils_read_buttons(&button_data);
        progcc_utils_read_sticks(&analog_data);

        if(calibrate)
        {
            progcc_utils_calibration_capture(&analog_data, &calibration_data);
            if (!gpio_get(PGPIO_BUTTON_A))
            {
                calibrate = false;
                progcc_utils_calculate_scalers(&calibration_data, &scaler_data);
                progcc_utils_set_rumble(PROGCC_RUMBLE_ON);
                sleep_ms(200);
                progcc_utils_set_rumble(PROGCC_RUMBLE_OFF);
            }
        }
        else
        {
            progcc_utils_scale_sticks(&analog_data, &analog_data_scaled,
                                        &calibration_data, &scaler_data);
        }
    }
}

int main() {
    stdio_init_all();
    board_init();

    sleep_ms(200);

    // Perform GPIO setup
    progcc_utils_hardware_setup();

    if (!gpio_get(PGPIO_BUTTON_START))
    {
        reset_usb_boot(0, 0);
    }

    progcc_usb_set_mode(PUSB_MODE_XI, true);

    if (!progcc_usb_start())
    {
        // Fall back to bootloader if we fail to start.
        reset_usb_boot(0, 0);
    }

    sleep_ms(200);

    multicore_launch_core1(main_two);

    //printf("Testing");
    for(;;)
    {
        progcc_usb_task(&button_data, &analog_data_scaled);
        tud_task();
        sleep_ms(1);
    }

}
