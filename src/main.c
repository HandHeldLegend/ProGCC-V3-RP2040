#include "progcc_includes.h"

// Set up local input vars
progcc_button_data_s button_data = {0};
progcc_analog_data_s analog_data = {0};

void main_two()
{
    for(;;)
    {
        progcc_utils_read_buttons(&button_data);
        progcc_utils_read_sticks(&analog_data);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(200);

    // Perform GPIO setup
    progcc_utils_hardware_setup();

    sleep_ms(200);

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

    multicore_launch_core1(main_two);

    //printf("Testing");
    while (true) {
        tud_task();
        progcc_usb_task(&button_data, &analog_data);
        sleep_ms(1);
    }

}
