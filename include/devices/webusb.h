#ifndef WEBUSB_H
#define WEBUSB_H

#include "progcc_includes.h"

typedef enum
{
  WEBUSB_CMD_RGB = 0x01,
} webusb_cmd_t;

void webusb_command_processor(uint8_t *data);

#endif