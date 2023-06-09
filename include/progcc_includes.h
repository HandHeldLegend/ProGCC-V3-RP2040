#ifndef PROGCC_INCLUDES_H
#define PROGCC_INCLUDES_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/rand.h"

#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "hardware/flash.h"

#include "progcc_types.h"
#include "progcc_defines.h"
#include "progcc_settings.h"

#include "stick_scaling.h"

#include "ws2812.pio.h"
#include "rgb.h"
#include "board_config.h"

#include "bsp/board.h"
#include "tusb.h"
// XInput TinyUSB Driver
#include "xinput_device.h"

#include "nsinput.h"
#include "gcinput.h"
#include "xinput.h"
#include "dinput.h"
#include "swpro.h"

#include "progcc_usb.h"

#include "remap.h"

#include "progcc.h"

// Switch pro includes
#include "switch_analog.h"
#include "switch_spi.h"
#include "switch_commands.h"



#endif
