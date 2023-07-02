#ifndef IMU_H
#define IMU_H

#include "progcc_includes.h"

extern volatile uint8_t imu_reads;

void imu_init();
void imu_buffer_out(uint8_t *output);
void imu_read_test(uint32_t timestamp);

#endif
