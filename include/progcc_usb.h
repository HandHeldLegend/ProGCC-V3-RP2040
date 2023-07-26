/*
 * Copyright (c) [2023] [Mitch Cairns/Handheldlegend, LLC]
 * All rights reserved.
 *
 * This source code is licensed under the provisions of the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef PROGCC_USB_H
#define PROGCC_USB_H

#include "progcc_includes.h"

extern bool _usb_busy;

bool pusb_start(usb_mode_t mode, bool performance_mode);
uint8_t dir_to_hat(hat_mode_t hat_type, uint8_t leftRight, uint8_t upDown);
void pusb_task(button_data_s *button_data, a_data_s *analog_data);

#endif
