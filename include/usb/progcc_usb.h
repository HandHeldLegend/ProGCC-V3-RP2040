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

extern bool _progcc_usb_busy;

void progcc_usb_set_mode(progcc_usb_mode_t mode, bool performance_mode);
bool progcc_usb_start(void);
uint8_t dir_to_hat(hat_mode_t hat_type, uint8_t leftRight, uint8_t upDown);
void progcc_usb_task(progcc_button_data_s *button_data, progcc_analog_data_s *analog_data);

#endif
