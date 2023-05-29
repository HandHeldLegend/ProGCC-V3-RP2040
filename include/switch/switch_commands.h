#ifndef SWITCH_COMMANDS_H
#define SWITCH_COMMANDS_H

#include "progcc_includes.h"

#define SW_OUT_ID_RUMBLE_CMD 0x01
#define SW_OUT_ID_RUMBLE 0x10

#define SW_CMD_GET_STATE        0x00
#define SW_CMD_GET_DEVICEINFO   0x02
#define SW_CMD_SET_INPUTMODE    0x03
#define SW_CMD_GET_TRIGGERET    0x04
#define SW_CMD_GET_PAGELIST     0x05
#define SW_CMD_SET_HCI          0x06
#define SW_CMD_GET_SPI          0x10
#define SW_CMD_SET_SPI          0x11
#define SW_CMD_ENABLE_IMU       0x40
#define SW_CMD_SET_IMUSENS      0x41
#define SW_CMD_ENABLE_VIBRATE   0x48
#define SW_CMD_SET_PLAYER       0x30
#define SW_CMD_GET_PLAYER       0x31

void switch_commands_report_handle(uint8_t report_id, const uint8_t *data, uint16_t len);

#endif
