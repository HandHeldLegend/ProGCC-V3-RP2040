#include "remap.h"

#define REMAP_SHIFT(button, shift) (button << shift)

button_data_s *_buttons_in;
button_data_s *_buttons_out;
button_remap_s _buttons_remap;

void remap_init(button_data_s *in, button_data_s *out, button_remap_s *remap)
{
  _buttons_in   = in;
  _buttons_out  = out;
  _buttons_remap.val = remap->val;
}

void remap_load_remap(button_remap_s *remap)
{
  _buttons_remap.val = remap->val;
}

void remap_buttons_task()
{
  _buttons_out->buttons_system = _buttons_in->buttons_system;
  _buttons_out->buttons_all = 0x00;

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_a, _buttons_remap.button_a);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_b, _buttons_remap.button_b);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_x, _buttons_remap.button_x);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_y, _buttons_remap.button_y);

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_up,    _buttons_remap.dpad_up);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_down,  _buttons_remap.dpad_down);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_left,  _buttons_remap.dpad_left);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->dpad_right, _buttons_remap.dpad_right);

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_plus,    _buttons_remap.button_plus);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_minus,   _buttons_remap.button_minus);

  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_stick_left,  _buttons_remap.button_stick_left);
  _buttons_out->buttons_all |= REMAP_SHIFT(_buttons_in->button_stick_right, _buttons_remap.button_stick_right);
}
