#ifndef PROGCC_SETTINGS_H
#define PROGCC_SETTINGS_H

#include "progcc_includes.h"

// Corresponds to version number in web app.
#define FIRMWARE_VERSION 0x0800
#define SETTINGS_VERSION 0x0800

typedef struct
{
    // We use a settings version to
    // keep settings between updates
    uint16_t    settings_version;
    uint8_t     comms_mode;
    uint8_t     usb_mode;
    bool    performance_mode;

    int lx_center;
    int ly_center;

    int rx_center;
    int ry_center;

    float l_angles[4];
    float r_angles[4];

    float l_angle_distances[8];
    float r_angle_distances[8];
} __attribute__ ((packed)) progcc_settings_s;

extern progcc_settings_s global_loaded_settings;

void settings_load();
void settings_save();
void settings_reset_to_default();

#endif
