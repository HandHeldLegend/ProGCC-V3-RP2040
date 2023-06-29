#ifndef STICK_SCALING_H
#define STICK_SCALING_H

#include "progcc_includes.h"

void stick_scaling_get_last_angles(float *la_out, float *ra_out);

void stick_scaling_process_data(a_data_s *in, a_data_s *out);

void stick_scaling_reset_distances();

bool stick_scaling_capture_distances(a_data_s *input);

void stick_scaling_capture_center(a_data_s *input);

void stick_scaling_save_all();

void stick_scaling_init();

void stick_scaling_finalize();

#endif
