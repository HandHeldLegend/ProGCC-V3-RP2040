#include "remap.h"

#define REMAP_SHIFT(button, shift) (button << shift)

void remap_buttons(button_data_s *in, button_data_s *out, button_remap_s *r)
{
  out->buttons_system = in->buttons_system;
  out->buttons_all = 0x00;

  out->buttons_all |= REMAP_SHIFT(in->button_a, r->button_a);
  out->buttons_all |= REMAP_SHIFT(in->button_b, r->button_b);
  out->buttons_all |= REMAP_SHIFT(in->button_x, r->button_x);
  out->buttons_all |= REMAP_SHIFT(in->button_y, r->button_y);

  out->buttons_all |= REMAP_SHIFT(in->dpad_up,    r->dpad_up);
  out->buttons_all |= REMAP_SHIFT(in->dpad_down,  r->dpad_down);
  out->buttons_all |= REMAP_SHIFT(in->dpad_left,  r->dpad_left);
  out->buttons_all |= REMAP_SHIFT(in->dpad_right, r->dpad_right);

  out->buttons_all |= REMAP_SHIFT(in->button_plus,    r->button_plus);
  out->buttons_all |= REMAP_SHIFT(in->button_minus,   r->button_minus);

  out->buttons_all |= REMAP_SHIFT(in->button_stick_left,  r->button_stick_left);
  out->buttons_all |= REMAP_SHIFT(in->button_stick_right, r->button_stick_right);
}
