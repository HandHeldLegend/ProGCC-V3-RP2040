#ifndef PROGCC_SETTINGS_H
#define PROGCC_SETTINGS_H

#include "progcc_includes.h"

// Corresponds to version number in web app.
#define FIRMWARE_VERSION 0x0800
#define SETTINGS_VERSION 0x7200

typedef struct
{
    // We use a settings version to
    // keep settings between updates
    uint16_t    settings_version;
    uint8_t     comms_mode;
    uint8_t     usb_mode;
    bool        performance_mode;
    uint64_t    remap_profile;

    uint8_t     switch_mac_address[6];

    int lx_center;
    int ly_center;

    int rx_center;
    int ry_center;

    // Angle Adjustments
    float l_angles[8];
    float r_angles[8];

    // Angle distances one for each of the 8 directions
    float l_angle_distances[8];
    float r_angle_distances[8];

    // IMU Calibration
    uint8_t imu_calibration[26];

} __attribute__ ((packed)) progcc_settings_s;

extern progcc_settings_s global_loaded_settings;

bool settings_load();
void settings_core1_save_check();
void settings_save();
void settings_reset_to_default();
void settings_set_centers(int lx, int ly, int rx, int ry);
void settings_set_distances(float *l_angle_distances, float *r_angle_distances);
void settings_set_angles(float *l_angles, float *r_angles);
void settings_set_mode(uint8_t comms_mode, uint8_t usb_mode);

#endif
