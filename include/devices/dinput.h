#ifndef DINPUT_H
#define DINPUT_H

#include "progcc_includes.h"

extern const tusb_desc_device_t di_device_descriptor;
extern const uint8_t di_hid_report_descriptor[];
extern const uint8_t di_configuration_descriptor[];

void dinput_hid_report(progcc_button_data_s *button_data, progcc_analog_data_s *analog_data);

#endif
