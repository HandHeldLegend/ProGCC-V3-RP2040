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
bool _analog_centered   = false;

bool _remap_enabled = false;

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
    pusb_task(_progcc_timestamp, &_progcc_buttons_remapped, &_progcc_analog_scaled);
    tud_task();
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
    stick_scaling_process_data(_progcc_analog, &_progcc_analog_scaled);
  }

}

void _progcc_task_1()
{
  for(;;)
  {
    _progcc_analog_tick(_progcc_timestamp);
  }


  // Read Gyro
  // NOT IMPLEMENTED
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
  uint8_t sub_mode = PUSB_MODE_XI;
  if (button_memory->button_a)
  {
      sub_mode = PUSB_MODE_SW;
  }

  uint8_t comms_mode = COMM_MODE_USB;

  settings_load();
  //settings_reset_to_default(); //debug

  sleep_ms(200);

  // Initialize analog stick scaling stuff
  // This is the preset angles that can be modified
  stick_scaling_init();

  // TEMP BECAUSE WE DO NOT HAVE
  // DEFAULT CENTERS YET
  cb_progcc_read_analog();
  stick_scaling_capture_center(_progcc_analog);
  stick_scaling_finalize();

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
