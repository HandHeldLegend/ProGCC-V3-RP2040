#ifndef STICK_SCALING_H
#define STICK_SCALING_H

#include "progcc_includes.h"

void stick_scaling_print_scalers();

void stick_scaling_print_centers();

void stick_scaling_create_scalers(a_data_s *analog_input);

void stick_scaling_capture_center(a_data_s *analog_input);

void stick_scaling_process_data(a_data_s *in, a_data_s *out);

#endif
