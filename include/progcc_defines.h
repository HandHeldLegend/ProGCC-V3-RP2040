#ifndef PROGCC_DEFINES_H
#define PROGCC_DEFINES_H

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

extern const char* global_string_descriptor[];

#define RGB_PIO pio0
#define RGB_SM 1

#define GAMEPAD_PIO pio0
#define GAMEPAD_SM 0

#endif
