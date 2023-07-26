#include "progcc.h"

bool _progcc_usb_task_enable = false;

button_data_s *_progcc_buttons = NULL;
button_data_s _progcc_buttons_remapped = {0};

a_data_s      *_progcc_analog = NULL;
a_data_s      _progcc_analog_scaled = {
  .lx = 128,
  .ly = 128,
  .rx = 128,
  .ry = 128,
};

button_remap_s *_progcc_remap = NULL;
volatile uint32_t _progcc_timestamp = 0;

bool _analog_calibrate  = false;
bool _analog_calibrate_octagon = false;
bool _analog_centered   = false;
bool _analog_all_angles_got    = false;

bool _remap_enabled = false;

void _progcc_calibrate_analog_start()
{
  rgb_s red = {
    .r = 225,
    .g = 0,
    .b = 0,
  };
  rgb_set_all(red.color);
  // Reset scaling distances
  stick_scaling_reset_distances();
  // Capture center value
  cb_progcc_read_analog();
  stick_scaling_capture_center(_progcc_analog);
  _analog_all_angles_got = false;
  _analog_centered  = true;
  _analog_calibrate = true;
}

void _progcc_calibrate_analog_save()
{
  rgb_s green = {
    .r = 0,
    .g = 0,
    .b = 200,
  };
  rgb_set_all(green.color);
  _analog_calibrate = false;
  _analog_calibrate_octagon = false;
  cb_progcc_rumble_enable(true);
  sleep_ms(200);
  cb_progcc_rumble_enable(false);
  stick_scaling_save_all();
  sleep_ms(200);
  stick_scaling_init();
}


void _progcc_task_0()
{
  _progcc_timestamp = time_us_32();
  // Read buttons
  cb_progcc_read_buttons();

  if (_remap_enabled)
  {
    remap_buttons(_progcc_buttons, &_progcc_buttons_remapped, _progcc_remap);
  }
  else
  {
    memcpy(&_progcc_buttons_remapped, _progcc_buttons, sizeof(_progcc_buttons_remapped));
  }

  if (_progcc_usb_task_enable)
  {
    // Process USB if needed
    tud_task();
    pusb_task(&_progcc_buttons_remapped, &_progcc_analog_scaled);
  }

  // Do callback for userland code insertion
  cb_progcc_task_0_hook(_progcc_timestamp);
}

// Returns a bool to indicate whether or not
// a comms frame should occur
bool _progcc_analog_ready(uint32_t timestamp)
{
    static uint32_t last_time   = 0;
    static uint32_t this_time   = 0;

    this_time = timestamp;

    // Clear variable
    uint32_t diff = 0;

    // Handle edge case where time has
    // looped around and is now less
    if (this_time < last_time)
    {
        diff = (0xFFFFFFFF - last_time) + this_time;
    }
    else if (this_time > last_time)
    {
        diff = this_time - last_time;
    }
    else return false;

    // We want a target rate according to our variable
    if (diff >= PROGCC_ANALOG_RATE)
    {
        // Set the last time
        last_time = this_time;
        return true;
    }
    return false;
}

// It will auto unblock until the configured
// analog input rate is met
void _progcc_analog_tick(uint32_t timestamp)
{
  if (_progcc_analog_ready(timestamp))
  {
    // Read analog sticks
    cb_progcc_read_analog();
    if (_analog_calibrate)
    {
      // Capture analog data
      if (stick_scaling_capture_distances(_progcc_analog) && !_analog_all_angles_got && !_analog_calibrate_octagon)
      {
        _analog_all_angles_got = true;
        rgb_s red = {
            .r = 0,
            .g = 128,
            .b = 128,
        };
        rgb_set_all(red.color);
      }
      else if(_progcc_buttons->button_a && _analog_calibrate_octagon)
      {
        if (stick_scaling_capture_angle(_progcc_analog))
        {
          rgb_s c1 = {
            .r = 0,
            .g = 128,
            .b = 0,
          };
          rgb_set_all(c1.color);
          sleep_ms(200);
          rgb_s c2 = {
              .r = 128,
              .g = 128,
              .b = 0,
          };
          rgb_set_all(c2.color);
        }
      }

      if (_progcc_buttons->button_capture)
      {
        _progcc_calibrate_analog_save();
      }
    }
    else
    {
      stick_scaling_process_data(_progcc_analog, &_progcc_analog_scaled);
    }
  }

}

// Core 1 task loop entrypoint
void _progcc_task_1()
{
  for(;;)
  {
    // Check if we need to save
    settings_core1_save_check();
    imu_read_test(_progcc_timestamp);
    // Do analog stuff :)
    _progcc_analog_tick(_progcc_timestamp);
  }
}



void progcc_remapping_enable(bool enable)
{
  _remap_enabled = enable;
}

void progcc_init(button_data_s *button_memory, a_data_s *analog_memory, button_remap_s *remap_profile)
{
  _progcc_buttons = button_memory;
  _progcc_analog  = analog_memory;

  // Set up hardware first
  cb_progcc_hardware_setup();

  // Read buttons
  cb_progcc_read_buttons();
  cb_progcc_read_buttons();
  cb_progcc_read_buttons();

  if (!settings_load())
  {
    settings_reset_to_default();
    sleep_ms(200);
    stick_scaling_init();
    // If we saved a default settings, initiate calibration
    _progcc_calibrate_analog_start();
  }

  if (button_memory->button_minus && button_memory->button_plus)
  {
    _progcc_calibrate_analog_start();
  }
  else if (button_memory->button_minus && button_memory->button_a)
  {
    _analog_calibrate_octagon = true;
    _progcc_calibrate_analog_start();
  }
  else
  {
    sleep_ms(200);
    // Initialize analog stick scaling stuff
    // This is the preset angles that can be modified
    stick_scaling_init();
  }

  // For switch Pro stuff
  switch_analog_calibration_init();

  uint8_t sub_mode = PUSB_MODE_SW; //PUSB_MODE_SW;
  uint8_t comms_mode = COMM_MODE_USB;
  if (button_memory->button_x)
  {
    sub_mode = PUSB_MODE_XI;
  }

  // Determine launch mode
  switch(comms_mode)
  {
    default:
    case COMM_MODE_USB:
    {
      bool did_usb_boot_ok = pusb_start(sub_mode, false);
      if (!did_usb_boot_ok)
      {

        // If USB mode fails, boot to bootloader.
        reset_usb_boot(0, 0);
        return;
      }
      _progcc_usb_task_enable = true;
    }
    break;

    // OTHER MODES NOT IMPLEMENTED FOR NOW
  }

  // Enable lockout victimhood :,)
  multicore_lockout_victim_init();

  // Launch second core
  multicore_launch_core1(_progcc_task_1);
  for(;;)
  {
    _progcc_task_0();
  }
}

void progcc_setup_gpio_scan(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_OUT);
  gpio_put(gpio, true);
}

void progcc_setup_gpio_push(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  //printf("Set up GPIO: %d", (uint8_t) gpio);
}

void progcc_setup_gpio_button(uint8_t gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  //printf("Set up GPIO: %d", (uint8_t) gpio);
}
