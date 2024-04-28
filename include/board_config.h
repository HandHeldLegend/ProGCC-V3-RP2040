#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Device stuff
#define HOJA_DEVICE_ID  0xA001 // DO NOT CHANGE FOR PROGCC 3

#define HOJA_FW_VERSION 0x0A3F

// RGB Stuff
#define HOJA_RGB_PIN 15
#define HOJA_RGB_COUNT 20
#define HOJA_RGBW_EN 0

// GPIO definitions
#define HOJA_SERIAL_PIN 7
#define HOJA_CLOCK_PIN 8
#define HOJA_LATCH_PIN 9

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja_config"
#define HOJA_MANUFACTURER   "HHL"
#define HOJA_PRODUCT        "ProGCC 3"

#define HOJA_CAPABILITY_ANALOG_STICK_L 1
#define HOJA_CAPABILITY_ANALOG_STICK_R 1
#define HOJA_CAPABILITY_ANALOG_TRIGGER_L 0
#define HOJA_CAPABILITY_ANALOG_TRIGGER_R 0

#define HOJA_CAPABILITY_BLUETOOTH 0
#define HOJA_CAPABILITY_BATTERY 0
#define HOJA_CAPABILITY_RGB 1
#define HOJA_CAPABILITY_GYRO 1

#define HOJA_CAPABILITY_NINTENDO_SERIAL 1
#define HOJA_CAPABILITY_NINTENDO_JOYBUS 1

#define HOJA_CAPABILITY_RUMBLE_ERM 1
#define HOJA_CAPABILITY_RUMBLE_LRA 0

#define HOJA_RGB_GROUP_RS       {0, 1, 2, 3}
#define HOJA_RGB_GROUP_LS       {4, 5, 6, 7}
#define HOJA_RGB_GROUP_DPAD     {8, 9, 10, 11}
#define HOJA_RGB_GROUP_MINUS    {12}
#define HOJA_RGB_GROUP_CAPTURE  {13}
#define HOJA_RGB_GROUP_HOME     {14}
#define HOJA_RGB_GROUP_PLUS     {15}
#define HOJA_RGB_GROUP_Y        {16}
#define HOJA_RGB_GROUP_X        {17}
#define HOJA_RGB_GROUP_A        {18}
#define HOJA_RGB_GROUP_B        {19}

#endif
