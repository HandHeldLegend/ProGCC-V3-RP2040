#ifndef NSINPUT_H
#define NSINPUT_H

#include "progcc_includes.h"

extern const tusb_desc_device_t ns_device_descriptor;
extern const uint8_t ns_hid_report_descriptor[];
extern const uint8_t ns_configuration_descriptor[];
extern const uint8_t ns_configuration_descriptor_performance[];

#define NS_REPORT_LEN 8

typedef struct
{
    union
        {
            struct
            {
                uint8_t button_y    : 1;
                uint8_t button_b    : 1;
                uint8_t button_a    : 1;
                uint8_t button_x    : 1;
                uint8_t trigger_l   : 1;
                uint8_t trigger_r   : 1;
                uint8_t trigger_zl  : 1;
                uint8_t trigger_zr  : 1;
            };
            uint8_t buttons_1;
        };

        union
        {
            struct
            {
                uint8_t button_minus  : 1;
                uint8_t button_plus   : 1;
                uint8_t stick_left    : 1;
                uint8_t stick_right   : 1;
                uint8_t button_home   : 1;
                uint8_t button_capture: 1;
                uint8_t dummy_1       : 2;
            };
            uint8_t buttons_2;
        };

    uint8_t dpad_hat;
    uint8_t stick_left_x;
    uint8_t stick_left_y;
    uint8_t stick_right_x;
    uint8_t stick_right_y;
    uint8_t dummy_2;

} __attribute__ ((packed)) ns_input_s;

void nsinput_hid_report(progcc_button_data_s *data);

#endif
