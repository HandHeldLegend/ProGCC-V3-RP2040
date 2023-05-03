#ifndef PROGCC_INCLUDES_H
#define PROGCC_INCLUDES_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "bsp/board.h"
#include "tusb.h"
// XInput TinyUSB Driver
#include "xinput_device.h"

#include "board_config.h"

#include "progcc_types.h"
#include "progcc_defines.h"
#include "progcc_settings.h"

#include "nsinput.h"
#include "gcinput.h"
#include "xinput.h"
#include "dinput.h"

#include "progcc_usb.h"

#endif
