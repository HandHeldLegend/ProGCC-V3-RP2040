#ifndef PROGCC_TYPES_H
#define PROGCC_TYPES_H

#include <inttypes.h>

typedef enum
{
    PUSB_MODE_NS,
    PUSB_MODE_GC,
    PUSB_MODE_XI,
    PUSB_MODE_DI,
    PUSB_MODE_MAX,
} progcc_usb_mode_t;

typedef enum
{
    PUSB_STATUS_IDLE,
    PUSB_STATUS_INITIALIZED,
} progcc_usb_status_t;

typedef enum
{
  PROGCC_RUMBLE_OFF,
  PROGCC_RUMBLE_BRAKE,
  PROGCC_RUMBLE_ON,
} progcc_rumble_t;

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
} __attribute__ ((packed)) progcc_button_data_s;

typedef struct
{
    int lx_center;
    int ly_center;
    int rx_center;
    int ry_center;
} __attribute__ ((packed)) a_calibration_s;

typedef struct
{
    int lx;
    int ly;
    int rx;
    int ry;
} __attribute__ ((packed)) a_data_s;

#endif
