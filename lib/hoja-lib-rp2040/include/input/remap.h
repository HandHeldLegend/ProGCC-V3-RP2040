#ifndef REMAP_H
#define REMAP_H

#include "hoja_includes.h"

void remap_send_data_webusb();
void remap_reset_default();
void remap_init(button_data_s *in, button_data_s *out);
void remap_listen_stop();
void remap_listen_enable(mapcode_t mapcode);
void remap_buttons_task();

#endif
