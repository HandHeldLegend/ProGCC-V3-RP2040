#ifndef PROGCC_H
#define PROGCC_H

#include "progcc_includes.h"

void cb_progcc_hardware_setup();

void cb_progcc_read_buttons();

void cb_progcc_read_analog();

void cb_progcc_read_imu();

void cb_progcc_rumble_enable(bool enable);

void progcc_init(uint8_t mode, button_data_s *button_memory, a_data_s *analog_memory);

void progcc_load_remap(button_remap_s *remap_profile);

#endif
