#ifndef HOJA_SETTINGS_H
#define HOJA_SETTINGS_H

#include "hoja_includes.h"

typedef struct
{
    button_remap_s  remap;
    buttons_unset_s disabled;
} remap_profile_s;

typedef enum
{
    GC_SP_MODE_NONE = 0, // No function. LT and RT are output full according to digital button.
    GC_SP_MODE_LT   = 1, // SP buttton inputs light trigger left
    GC_SP_MODE_RT   = 2, // SP buttton inputs light trigger right
    GC_SP_MODE_ADC  = 3, // Controlled fully by analog, SP button is unused
} gc_sp_mode_t;

typedef struct
{
    // We use a settings version to
    // keep settings between updates
    uint16_t    settings_version;
    input_mode_t input_mode;
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

    // IMU Offsets for more precision
    uint8_t imu_offsets[12];

    // RGB Color Store
    uint32_t rgb_colors[12];

    remap_profile_s remap_switch;
    remap_profile_s remap_xinput;
    remap_profile_s remap_gamecube;
    remap_profile_s remap_n64;
    remap_profile_s remap_snes;

    gc_sp_mode_t gc_sp_mode;

} hoja_settings_s;

extern hoja_settings_s global_loaded_settings;

bool settings_load();
void settings_core1_save_check();
void settings_save_webindicate();
void settings_save();
void settings_reset_to_default();
void settings_set_centers(int lx, int ly, int rx, int ry);
void settings_set_distances(float *l_angle_distances, float *r_angle_distances);
void settings_set_angles(float *l_angles, float *r_angles);
void settings_set_mode(input_mode_t mode);
void settings_set_gc_sp(gc_sp_mode_t sp_mode);
void settings_set_snapback(uint8_t axis, uint8_t level);

#endif
