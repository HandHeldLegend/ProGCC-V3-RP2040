#ifndef PROGCC_UTILS_H
#define PROGCC_UTILS_H

#include "progcc_includes.h"

// Set up all pin hardware
void progcc_utils_hardware_setup(void);
void progcc_utils_read_buttons(progcc_button_data_s *data);
void progcc_utils_read_sticks(progcc_analog_data_s *data);

#endif
