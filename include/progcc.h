#ifndef PROGCC_H
#define PROGCC_H

#include "progcc_includes.h"

void cb_progcc_hardware_setup();

void cb_progcc_read_buttons();

void cb_progcc_read_analog();

void cb_progcc_read_imu();

void cb_progcc_rumble_enable(bool enable);

void cb_progcc_task_0_hook(uint32_t timestamp);

void progcc_init(button_data_s *button_memory, a_data_s *analog_memory, button_remap_s *remap_profile);

void progcc_load_remap(button_remap_s *remap_profile);

void progcc_setup_gpio_scan(uint8_t gpio);
void progcc_setup_gpio_push(uint8_t gpio);
void progcc_setup_gpio_button(uint8_t gpio);

#endif
