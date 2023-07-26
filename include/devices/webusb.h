#ifndef WEBUSB_H
#define WEBUSB_H

#include "progcc_includes.h"

typedef enum
{
  WEBUSB_CMD_RGB = 0x01,
  WEBUSB_CMD_SNAPBACK = 0x02,
  WEBUSB_CMD_SNAPBACK_GET = 0xA2,
  WEBUSB_CMD_SAVEALL = 0xF1,
} webusb_cmd_t;

void webusb_save_confirm();
void webusb_command_processor(uint8_t *data);

#endif