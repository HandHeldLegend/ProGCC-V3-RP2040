#ifndef GCINPUT_H
#define GCINPUT_H

#include "progcc_includes.h"

extern const tusb_desc_device_t gc_device_descriptor;
extern const uint8_t gc_hid_report_descriptor[];
extern const uint8_t gc_configuration_descriptor[];
extern const uint8_t gc_configuration_descriptor_performance[];

void gcinput_hid_report(progcc_button_data_s *data);

#endif
