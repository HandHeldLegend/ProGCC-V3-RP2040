#include "progcc.h"

button_remap_s default_remap = {
  .dpad_up    = MAPCODE_DUP,
  .dpad_down  = MAPCODE_DDOWN,
  .dpad_left  = MAPCODE_DLEFT,
  .dpad_right = MAPCODE_DRIGHT,

  .button_up  = MAPCODE_B_UP,
  .button_down = MAPCODE_B_DOWN,
  .button_left = MAPCODE_B_LEFT,
  .button_right = MAPCODE_B_RIGHT,

  .trigger_l = MAPCODE_T_L,
  .trigger_r = MAPCODE_T_R,
  .trigger_zl = MAPCODE_T_ZL,
  .trigger_zr = MAPCODE_T_ZR,

  .button_start = MAPCODE_B_START,
  .button_select = MAPCODE_B_SELECT,
  .button_stick_left = MAPCODE_B_STICKL,
  .button_stick_right = MAPCODE_B_STICKR,
};

button_data_s *_progcc_buttons;
a_data_s      *_progcc_analog;
a_data_s      _progcc_analog_scaled = {0};

bool _analog_calibrate  = false;
bool _analog_centered   = false;

void _progcc_remap_process()
{

}

void _progcc_task_0(uint32_t timestamp)
{
  // Read buttons

  // Process COMMS
  comms_tick();
}

void _progcc_task_1(uint32_t timestamp)
{
  // Read Sticks
  if (_analog_calibrate)
  {
    stick_scaling_capture_distances(_progcc_analog);
  }
  else
  {

  }
  // Read Gyro
}

void progcc_calibrate_analog_set(calibrate_set_t set)
{
  switch(set)
  {
    default:
    case CALIBRATE_START:
    {
      // Capture center value
      stick_scaling_capture_center(_progcc_analog);
      _analog_centered  = true;
      _analog_calibrate = true;
    }
    break;

    case CALIBRATE_CANCEL:
    {
      _analog_calibrate = false;
    }
    break;

    case CALIBRATE_SAVE:
    {
      stick_scaling_finalize();
      _analog_calibrate = false;
      cb_progcc_rumble_enable(true);
      sleep_ms(200);
      cb_progcc_rumble_enable(false);
    }
    break;
  }
}

void progcc_calibrate_analog_save()
{

}

void progcc

void progcc_init(uint8_t comms_mode, uint8_t sub_mode, button_data_s *button_memory, a_data_s *analog_memory)
{

  // Set up hardware first
  cb_progcc_hardware_setup();

  // Initialize analog stick scaling stuff
  // This is the preset angles that can be modified
  stick_scaling_init();

  // Determine launch mode

  // Launch second core
  multicore_launch_core1(_progcc_task_1);
}
