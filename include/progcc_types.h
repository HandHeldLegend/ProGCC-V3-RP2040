#ifndef PROGCC_TYPES_H
#define PROGCC_TYPES_H

#include <inttypes.h>

// Map code is used during remap
// operations and configuration
typedef enum
{
    MAPCODE_DUP = 0,
    MAPCODE_DDOWN,
    MAPCODE_DLEFT,
    MAPCODE_DRIGHT,

    MAPCODE_B_A,
    MAPCODE_B_B,
    MAPCODE_B_X,
    MAPCODE_B_Y,

    MAPCODE_T_L,
    MAPCODE_T_ZL,
    MAPCODE_T_R,
    MAPCODE_T_ZR,

    MAPCODE_B_PLUS,
    MAPCODE_B_MINUS,
    MAPCODE_B_STICKL,
    MAPCODE_B_STICKR,
} mapcode_t;

// Remapping struct used to determine
// remapping parameters
typedef struct
{
    union
    {
        struct
        {
            mapcode_t dpad_up     : 4;
            mapcode_t dpad_down   : 4;
            mapcode_t dpad_left   : 4;
            mapcode_t dpad_right  : 4;

            mapcode_t button_a      : 4;
            mapcode_t button_b      : 4;
            mapcode_t button_x      : 4;
            mapcode_t button_y      : 4;

            mapcode_t trigger_l       : 4;
            mapcode_t trigger_zl      : 4;
            mapcode_t trigger_r       : 4;
            mapcode_t trigger_zr      : 4;

            mapcode_t button_plus   : 4;
            mapcode_t button_minus  : 4;

            mapcode_t button_stick_left     : 4;
            mapcode_t button_stick_right    : 4;
        };
        uint64_t val;
    };
} button_remap_s;

typedef struct
{
    union
    {
        struct
        {
            uint8_t padding : 8;
            uint8_t b : 8;
            uint8_t r : 8;
            uint8_t g : 8;
        };
        uint32_t color;
    };
} rgb_s;

typedef enum
{
    COMM_MODE_USB,
    COMM_MODE_GC,
    COMM_MODE_N64,
    COMM_MODE_SNES,
    COMM_MODE_DEBUG,
} comm_mode_t;

typedef enum
{
    PUSB_MODE_NS,
    PUSB_MODE_GC,
    PUSB_MODE_XI,
    PUSB_MODE_DI,
    PUSB_MODE_SW,
    PUSB_MODE_MAX,
} usb_mode_t;

typedef enum
{
    CALIBRATE_START,
    CALIBRATE_CANCEL,
    CALIBRATE_SAVE,
} calibrate_set_t;

typedef enum
{
  RUMBLE_OFF,
  RUMBLE_BRAKE,
  RUMBLE_ON,
} rumble_t;

typedef enum
{
  NS_HAT_TOP          = 0x00,
  NS_HAT_TOP_RIGHT    = 0x01,
  NS_HAT_RIGHT        = 0x02,
  NS_HAT_BOTTOM_RIGHT = 0x03,
  NS_HAT_BOTTOM       = 0x04,
  NS_HAT_BOTTOM_LEFT  = 0x05,
  NS_HAT_LEFT         = 0x06,
  NS_HAT_TOP_LEFT     = 0x07,
  NS_HAT_CENTER       = 0x08,
} ns_input_hat_dir_t;

typedef enum
{
  XI_HAT_TOP          = 0x01,
  XI_HAT_TOP_RIGHT    = 0x02,
  XI_HAT_RIGHT        = 0x03,
  XI_HAT_BOTTOM_RIGHT = 0x04,
  XI_HAT_BOTTOM       = 0x05,
  XI_HAT_BOTTOM_LEFT  = 0x06,
  XI_HAT_LEFT         = 0x07,
  XI_HAT_TOP_LEFT     = 0x08,
  XI_HAT_CENTER       = 0x00,
} xi_input_hat_dir_t;

typedef enum
{
    HAT_MODE_NS,
    HAT_MODE_XI,
} hat_mode_t;

/** @brief This is a struct for containing all of the
 * button input data as bits. This saves space
 * and allows for easier handoff to the various
 * controller cores in the future.
**/
typedef struct
{
    union
    {
        struct
        {
            // D-Pad
            uint8_t dpad_up         : 1;
            uint8_t dpad_down       : 1;
            uint8_t dpad_left       : 1;
            uint8_t dpad_right      : 1;
            // Buttons
            uint8_t button_a       : 1;
            uint8_t button_b     : 1;
            uint8_t button_x     : 1;
            uint8_t button_y    : 1;
            // Triggers
            uint8_t trigger_l       : 1;
            uint8_t trigger_zl      : 1;
            uint8_t trigger_r       : 1;
            uint8_t trigger_zr      : 1;

            // Special Functions
            uint8_t button_plus     : 1;
            uint8_t button_minus    : 1;

            // Stick clicks
            uint8_t button_stick_left   : 1;
            uint8_t button_stick_right  : 1;
        };
        uint16_t buttons_all;
    };

    union
    {
        struct
        {
            // Menu buttons (Not remappable by API)
            uint8_t button_capture  : 1;
            uint8_t button_home     : 1;
            uint8_t padding         : 6;
        };
        uint8_t buttons_system;
    };
} __attribute__ ((packed)) button_data_s;

typedef struct
{
    int lx_center;
    int ly_center;

    int rx_center;
    int ry_center;
} __attribute__ ((packed)) a_calib_center_s;

typedef struct
{
    int lx_hi;
    int lx_lo;

    int ly_hi;
    int ly_lo;

    int rx_hi;
    int rx_lo;

    int ry_hi;
    int ry_lo;
} __attribute__ ((packed)) a_calib_hilo_s;

typedef struct
{
    int lx;
    int ly;
    int rx;
    int ry;
} __attribute__ ((packed)) a_data_s;

typedef struct
{
    int lx;
    int ly;
    int rx;
    int ry;
    int tl;
    int tr;
} __attribute__ ((packed)) analog_data_s;

typedef struct
{
    float lx;
    float ly;
    float rx;
    float ry;
    float lt;
    float rt;
} __attribute__ ((packed)) af_data_s;

typedef struct
{
    union
    {
        struct
        {
            // Y and C-Up (N64)
            uint8_t b_y       : 1;


            // X and C-Left (N64)
            uint8_t b_x       : 1;

            uint8_t b_b       : 1;
            uint8_t b_a       : 1;
            uint8_t t_r_sr    : 1;
            uint8_t t_r_sl    : 1;
            uint8_t t_r       : 1;

            // ZR and C-Down (N64)
            uint8_t t_zr      : 1;
        };
        uint8_t right_buttons;
    };
    union
    {
        struct
        {
            // Minus and C-Right (N64)
            uint8_t b_minus     : 1;

            // Plus and Start
            uint8_t b_plus      : 1;

            uint8_t sb_right    : 1;
            uint8_t sb_left     : 1;
            uint8_t b_home      : 1;
            uint8_t b_capture   : 1;
            uint8_t none        : 1;
            uint8_t charge_grip_active : 1;
        };
        uint8_t shared_buttons;
    };
    union
    {
        struct
        {
            uint8_t d_down    : 1;
            uint8_t d_up      : 1;
            uint8_t d_right   : 1;
            uint8_t d_left    : 1;
            uint8_t t_l_sr    : 1;
            uint8_t t_l_sl    : 1;
            uint8_t t_l       : 1;

            // ZL and Z (N64)
            uint8_t t_zl      : 1;

        };
        uint8_t left_buttons;
    };

    uint16_t ls_x;
    uint16_t ls_y;
    uint16_t rs_x;
    uint16_t rs_y;

} __attribute__ ((packed)) sw_input_s;

#endif
