#ifndef IMU_H
#define IMU_H

#include "progcc_includes.h"

void imu_set_enabled(bool enable);
void imu_init();
void imu_buffer_out(uint8_t *output);
void imu_reset_idx();
void imu_read_test(uint32_t timestamp);

#endif
