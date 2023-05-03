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
    uint8_t     usb_mode;

    uint8_t analog_scaler;
    bool performance_mode;

} __attribute__ ((packed)) progcc_settings_s;

void progcc_settings_load(progcc_settings_s * settings);
void progcc_settings_save(progcc_settings_s * settings);
void progcc_settings_reset_to_default(progcc_settings_s * settings);

#endif
