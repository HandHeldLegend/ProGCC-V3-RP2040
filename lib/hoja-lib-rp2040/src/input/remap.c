#include "remap.h"

#define REMAP_SHIFT(button, shift) (button << shift)

button_data_s *_buttons_in;
button_data_s *_buttons_out;
button_remap_s _buttons_remap;

mapcode_t _button_remap_arr[MAPCODE_MAX];
mapcode_t _button_remap_code;
bool _button_remap_listen = false;

const button_remap_s default_user_map = {
    .dpad_up = MAPCODE_DUP,
    .dpad_down = MAPCODE_DDOWN,
    .dpad_left = MAPCODE_DLEFT,
    .dpad_right = MAPCODE_DRIGHT,

    .button_a = MAPCODE_B_A,
    .button_b = MAPCODE_B_B,
    .button_x = MAPCODE_B_X,
    .button_y = MAPCODE_B_Y,

    .trigger_l = MAPCODE_T_L,
    .trigger_r = MAPCODE_T_R,
    .trigger_zl = MAPCODE_T_ZL,
    .trigger_zr = MAPCODE_T_ZR,

    .button_plus = MAPCODE_B_PLUS,
    .button_minus = MAPCODE_B_MINUS,
    .button_stick_left = MAPCODE_B_STICKL,
    .button_stick_right = MAPCODE_B_STICKR,
};

void _remap_load_remap()
{
  // Load the struct values into the remap array.
  // The exact loading depends on how the struct is defined and how the buttons are encoded,
  // but here's an example:
  _button_remap_arr[MAPCODE_DUP] = _buttons_remap.dpad_up;
  _button_remap_arr[MAPCODE_DDOWN] = _buttons_remap.dpad_down;
  _button_remap_arr[MAPCODE_DLEFT] = _buttons_remap.dpad_left;
  _button_remap_arr[MAPCODE_DRIGHT] = _buttons_remap.dpad_right;

  _button_remap_arr[MAPCODE_B_A] = _buttons_remap.button_a;
  _button_remap_arr[MAPCODE_B_B] = _buttons_remap.button_b;
  _button_remap_arr[MAPCODE_B_X] = _buttons_remap.button_x;
  _button_remap_arr[MAPCODE_B_Y] = _buttons_remap.button_y;

  _button_remap_arr[MAPCODE_T_L] = _buttons_remap.trigger_l;
  _button_remap_arr[MAPCODE_T_ZL] = _buttons_remap.trigger_zl;
  _button_remap_arr[MAPCODE_T_R] = _buttons_remap.trigger_r;
  _button_remap_arr[MAPCODE_T_ZR] = _buttons_remap.trigger_zr;

  _button_remap_arr[MAPCODE_B_PLUS] = _buttons_remap.button_plus;
  _button_remap_arr[MAPCODE_B_MINUS] = _buttons_remap.button_minus;
  _button_remap_arr[MAPCODE_B_STICKL] = _buttons_remap.button_stick_left;
  _button_remap_arr[MAPCODE_B_STICKR] = _buttons_remap.button_stick_right;
}

void _remap_pack_remap()
{
  _buttons_remap.dpad_up = _button_remap_arr[MAPCODE_DUP];
  _buttons_remap.dpad_down = _button_remap_arr[MAPCODE_DDOWN];
  _buttons_remap.dpad_left = _button_remap_arr[MAPCODE_DLEFT];
  _buttons_remap.dpad_right = _button_remap_arr[MAPCODE_DRIGHT];

  _buttons_remap.button_a = _button_remap_arr[MAPCODE_B_A];
  _buttons_remap.button_b = _button_remap_arr[MAPCODE_B_B];
  _buttons_remap.button_x = _button_remap_arr[MAPCODE_B_X];
  _buttons_remap.button_y = _button_remap_arr[MAPCODE_B_Y];

  _buttons_remap.trigger_l = _button_remap_arr[MAPCODE_T_L];
  _buttons_remap.trigger_zl = _button_remap_arr[MAPCODE_T_ZL];
  _buttons_remap.trigger_r = _button_remap_arr[MAPCODE_T_R];
  _buttons_remap.trigger_zr = _button_remap_arr[MAPCODE_T_ZR];

  _buttons_remap.button_plus = _button_remap_arr[MAPCODE_B_PLUS];
  _buttons_remap.button_minus = _button_remap_arr[MAPCODE_B_MINUS];
  _buttons_remap.button_stick_left = _button_remap_arr[MAPCODE_B_STICKL];
  _buttons_remap.button_stick_right = _button_remap_arr[MAPCODE_B_STICKR];

  global_loaded_settings.remap_profile = _buttons_remap.val;
}

void _remap_listener(uint16_t buttons)
{
  /* Now we need to listen for which button has been pressed
   *  We do this by taking in the raw value and shifting
   *  right one bit at a time. If we have a bit set
   *  we know that we want to assign our set mapcode TO
   *  that button
   */
  mapcode_t m = 0;
  bool got_map = false;

  for (uint8_t i = 0; i < 16; i++)
  {
    if ((buttons >> i) & 0x1)
    {
      got_map = true;
      m = i;
      break;
    }
  }

  if (got_map)
  {
    _button_remap_arr[_button_remap_code] = m;
    _button_remap_listen = false;
    _remap_pack_remap();
    remap_send_data_webusb();
  }
}

void remap_send_data_webusb()
{
  remap_listen_stop();
  uint8_t b[64] = {0};
  b[0] = WEBUSB_CMD_REMAP_GET;
  button_remap_s rm = {.val = global_loaded_settings.remap_profile};
  b[1] = rm.dpad_up;
  b[2] = rm.dpad_down;
  b[3] = rm.dpad_left;
  b[4] = rm.dpad_right;
  b[5] = rm.button_a;
  b[6] = rm.button_b;
  b[7] = rm.button_x;
  b[8] = rm.button_y;
  b[9] = rm.trigger_l;
  b[10] = rm.trigger_zl;
  b[11] = rm.trigger_r;
  b[12] = rm.trigger_zr;
  b[13] = rm.button_plus;
  b[14] = rm.button_minus;
  b[15] = rm.button_stick_left;
  b[16] = rm.button_stick_right;
  tud_vendor_n_write(0, b, 64);
  tud_vendor_n_flush(0);
}

void remap_reset_default()
{
  global_loaded_settings.remap_profile = default_user_map.val;
}

void remap_init(button_data_s *in, button_data_s *out)
{
  _buttons_in = in;
  _buttons_out = out;
  _buttons_remap.val = global_loaded_settings.remap_profile;
  _remap_load_remap();
}

void remap_listen_stop()
{
  _button_remap_listen = false;
}

void remap_listen_enable(mapcode_t mapcode)
{
  /* Set the button we are assigning TO
   * EX we want to press a button that will activate
   * button A so we first input mapcode of MAPCODE_B_A */
  _button_remap_code = mapcode;
  _button_remap_listen = true;
}

void remap_buttons_task()
{
  _buttons_out->buttons_all = 0x00;
  _buttons_out->buttons_system = 0x00;

  if (_button_remap_listen)
  {
    _remap_listener(_buttons_in->buttons_all);
  }

  if (!safe_mode_check())
  {
    _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_plus, _button_remap_arr[MAPCODE_B_PLUS]);
    _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_minus, _button_remap_arr[MAPCODE_B_MINUS]);

    _buttons_out->button_home     = _buttons_in->button_home;
    _buttons_out->button_capture  = _buttons_in->button_capture;
  }

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_up, _button_remap_arr[MAPCODE_DUP]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_down, _button_remap_arr[MAPCODE_DDOWN]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_left, _button_remap_arr[MAPCODE_DLEFT]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_right, _button_remap_arr[MAPCODE_DRIGHT]);

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_a, _button_remap_arr[MAPCODE_B_A]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_b, _button_remap_arr[MAPCODE_B_B]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_x, _button_remap_arr[MAPCODE_B_X]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_y, _button_remap_arr[MAPCODE_B_Y]);

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->trigger_l, _button_remap_arr[MAPCODE_T_L]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->trigger_r, _button_remap_arr[MAPCODE_T_R]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->trigger_zl, _button_remap_arr[MAPCODE_T_ZL]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->trigger_zr, _button_remap_arr[MAPCODE_T_ZR]);

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_stick_left, _button_remap_arr[MAPCODE_B_STICKL]);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_stick_right, _button_remap_arr[MAPCODE_B_STICKR]);
}
