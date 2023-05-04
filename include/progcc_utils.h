#ifndef PROGCC_UTILS_H
#define PROGCC_UTILS_H

#include "progcc_includes.h"

// Set up all pin hardware
void progcc_utils_hardware_setup(void);
void progcc_utils_read_buttons(progcc_button_data_s *data);
void progcc_utils_read_sticks(progcc_analog_data_s *data);
void progcc_utils_set_rumble(progcc_rumble_t rumble);

void progcc_utils_scale_sticks(progcc_analog_data_s *data_in,
                                progcc_analog_data_s *data_out,
                                progcc_analog_calibration_data_s *calibration_data,
                                progcc_analog_scaler_data_s *scaler_data);

void progcc_utils_calculate_scalers(progcc_analog_calibration_data_s *calibration_data,
                                    progcc_analog_scaler_data_s *scaler_data);

void progcc_utils_calibration_capture(progcc_analog_data_s *data,
                                      progcc_analog_calibration_data_s *calibration_data);

#endif
