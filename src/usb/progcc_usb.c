/*
 * Copyright (c) [2023] [Mitch Cairns/Handheldlegend, LLC]
 * All rights reserved.
 *
 * This source code is licensed under the provisions of the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "progcc_usb.h"

usb_mode_t _usb_mode    = PUSB_MODE_XI;
bool _usb_busy = false;

// Default 8ms (8000us)
uint32_t _usb_rate = 7600;

typedef void (*usb_cb_t)(button_data_s *, a_data_s *);

usb_cb_t _usb_hid_cb = NULL;

void _pusb_set_interval(usb_rate_t rate)
{
  _usb_rate = rate;
}

bool pusb_start(usb_mode_t mode)
{

  imu_set_enabled(false);

  switch(mode)
  {
    case PUSB_MODE_MAX:
    default:
    case PUSB_MODE_DI:
      _pusb_set_interval(USBRATE_8);
      _usb_hid_cb = dinput_hid_report;
      break;

    case PUSB_MODE_SW:
      _pusb_set_interval(USBRATE_8);
      _usb_hid_cb = swpro_hid_report;
      break;

    case PUSB_MODE_NS:
      _pusb_set_interval(USBRATE_8);
      imu_set_enabled(true);
      _usb_hid_cb = nsinput_hid_report;
      break;

    case PUSB_MODE_GC:
      _pusb_set_interval(USBRATE_8);
      _usb_hid_cb = gcinput_hid_report;
      break;

    case PUSB_MODE_XI:
      _pusb_set_interval(USBRATE_8);
      _usb_hid_cb = xinput_hid_report;
      break;
  }

  _usb_mode = mode;
  return tusb_init();
}

// Returns a bool to indicate whether or not
// a comms frame should occur
bool _pusb_poll_ready(uint32_t timestamp)
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
    else
    {
        diff = this_time - last_time;
    }

    // We want a target rate according to our variable
    if (diff >= _usb_rate)
    {
        // Set the last time
        last_time = this_time;
        return true;
    }
    return false;
} 

uint8_t buf = 0;
bool _usbNullerror = false;

bool _pusb_ready()
{
  if (_usb_mode == PUSB_MODE_XI)
  {
    return tud_xinput_ready();
  }
  else if (_usb_mode == PUSB_MODE_WEB)
  {
    return tud_vendor_available();
  }
  else return tud_hid_ready();
}

void pusb_task(uint32_t timestamp, button_data_s *button_data, a_data_s *analog_data)
{
  if (_pusb_ready())
  {
    if (_pusb_poll_ready(timestamp))
    {
      // Call the registered function
      if (_usb_hid_cb != NULL)
      {
          _usb_hid_cb(button_data, analog_data);
      }
      else if (!_usbNullerror)
      {
        _usbNullerror = true;
        printf("_usb_hid_cb is NULL\n");
      }
    }
  }
}

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void) {
  switch(_usb_mode)
  {
    default:
    case PUSB_MODE_DI:
      return (uint8_t const*) &di_device_descriptor;
      break;

    case PUSB_MODE_SW:
      return (uint8_t const*) &swpro_device_descriptor;
      break;

    case PUSB_MODE_NS:
      return (uint8_t const*) &ns_device_descriptor;
      break;

    case PUSB_MODE_GC:
      return (uint8_t const*) &gc_device_descriptor;
      break;

    case PUSB_MODE_XI:
      return (uint8_t const*) &xid_device_descriptor;
      break;
  }
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;  // for multiple configurations
  switch(_usb_mode)
  {
    case PUSB_MODE_MAX:
    default:
    case PUSB_MODE_DI:
      return (uint8_t const*) &di_configuration_descriptor;
      break;

    case PUSB_MODE_SW:
      return (uint8_t const*) &swpro_configuration_descriptor;
      break;

    case PUSB_MODE_NS:
      return (uint8_t const*) &ns_configuration_descriptor;
      break;

    case PUSB_MODE_GC:
      return (uint8_t const*) &gc_configuration_descriptor;
      break;

    case PUSB_MODE_XI:
      return (uint8_t const*) &xid_configuration_descriptor;
      break;
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) reqlen;

  return 0;
}

// Invoked when report complete
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    switch (_usb_mode)
    {
        case PUSB_MODE_DI:
            if ((report[0] == 0x01) || (report[0] == 0x02))
            {
                //usb_process_data();
            }
            break;

        default:
        case PUSB_MODE_NS:
            if (len == NS_REPORT_LEN)
            {
                //usb_process_data();
            }
            break;
        case PUSB_MODE_XI:
            if ( (report[0] == 0x00) && (report[1] == XID_REPORT_LEN))
            {
                //usb_process_data();
            }

            break;

        case PUSB_MODE_GC:
            if (report[0] == 0x21)
            {
                //usb_process_data();
            }
            break;
    }

}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  switch (_usb_mode)
  {
    case PUSB_MODE_MAX:
    default:
    case PUSB_MODE_DI:
      if (!report_id && !report_type)
      {
          /*
          if (buffer[0] == CMD_USB_REPORTID)
          {
              //command_handler(buffer, bufsize);
          }*/
      }
      break;

    case PUSB_MODE_SW:
      if (!report_id && !report_type)
      {
        if (buffer[0] == SW_OUT_ID_RUMBLE)
        {
          rumble_translate(&buffer[2]);
        }
        else
        {
          switch_commands_future_handle(buffer[0], buffer, bufsize);
        }

      }
      break;
    case PUSB_MODE_NS:
      break;
    case PUSB_MODE_GC:
      if (!report_id && !report_type)
      {
        if (buffer[0] == 0x11)
        {
            //rx_vibrate = (buffer[1] > 0) ? true : false;
        }
        else if (buffer[0] == 0x13)
        {
            //ESP_LOGI("INIT", "Rx");
        }
      }
      break;
    case PUSB_MODE_XI:
      if (!report_id && !report_type)
      {
          if ((buffer[0] == 0x00) && (buffer[1] == 0x08))
          {
              _usb_busy = true;
              if ((buffer[3] > 0) || (buffer[4] > 0))
              {
                  cb_progcc_rumble_enable(true);
              }
              else
              {
                  cb_progcc_rumble_enable(false);
              }
              _usb_busy = false;
          }
      }
      break;
  }
}

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void) instance;
    switch (_usb_mode)
    {
        case PUSB_MODE_MAX:
        default:
            break;
        case PUSB_MODE_DI:
            return di_hid_report_descriptor;
            break;
        case PUSB_MODE_SW:
            return swpro_hid_report_descriptor;
            break;
        case PUSB_MODE_NS:
            return ns_hid_report_descriptor;
            break;
        case PUSB_MODE_GC:
            return gc_hid_report_descriptor;
            break;

    }
    return NULL;
}

// Set up custom TinyUSB XInput Driver
// Sets up custom TinyUSB Device Driver
usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    *driver_count += 1;
    return &tud_xinput_driver;
}

static uint16_t _desc_str[64];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if ( index == 0)
  {
    memcpy(&_desc_str[1], global_string_descriptor[0], 2);
    chr_count = 1;
  }else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    const char* str = global_string_descriptor[index];

    // Cap at max char... WHY?
    chr_count = strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);
  return _desc_str;
}

// Vendor Device Class CB for receiving data
void tud_vendor_rx_cb(uint8_t itf)
{
  printf("WebUSB Data Received.\n");
  uint8_t buffer[64] = {0};
  uint32_t size = 0;
  tud_vendor_n_read(itf, buffer, 64);
  webusb_command_processor(buffer);
}

/********* USB Data Handling Utilities ***************/
/**
 * @brief Takes in directions and outputs a byte output appropriate for
 * HID Hat usage.
 * @param hat_type hat_mode_t type - The type of controller you're converting for.
 * @param leftRight 0 through 2 (2 is right) to indicate the direction left/right
 * @param upDown 0 through 2 (2 is up) to indicate the direction up/down
*/
uint8_t dir_to_hat(hat_mode_t hat_type, uint8_t leftRight, uint8_t upDown)
{
    uint8_t ret = 0;
    switch(hat_type)
    {
        default:
        case HAT_MODE_NS:
            ret = NS_HAT_CENTER;

        if (leftRight == 2)
        {
            ret = NS_HAT_RIGHT;
            if (upDown == 2)
            {
                ret = NS_HAT_TOP_RIGHT;
            }
            else if (upDown == 0)
            {
                ret = NS_HAT_BOTTOM_RIGHT;
            }
        }
        else if (leftRight == 0)
        {
            ret = NS_HAT_LEFT;
            if (upDown == 2)
            {
                ret = NS_HAT_TOP_LEFT;
            }
            else if (upDown == 0)
            {
                ret = NS_HAT_BOTTOM_LEFT;
            }
        }

        else if (upDown == 2)
        {
            ret = NS_HAT_TOP;
        }
        else if (upDown == 0)
        {
            ret = NS_HAT_BOTTOM;
        }

        return ret;
        break;

        case HAT_MODE_XI:
                ret = XI_HAT_CENTER;

            if (leftRight == 2)
            {
                ret = XI_HAT_RIGHT;
                if (upDown == 2)
                {
                    ret = XI_HAT_TOP_RIGHT;
                }
                else if (upDown == 0)
                {
                    ret = XI_HAT_BOTTOM_RIGHT;
                }
            }
            else if (leftRight == 0)
            {
                ret = XI_HAT_LEFT;
                if (upDown == 2)
                {
                    ret = XI_HAT_TOP_LEFT;
                }
                else if (upDown == 0)
                {
                    ret = XI_HAT_BOTTOM_LEFT;
                }
            }

            else if (upDown == 2)
            {
                ret = XI_HAT_TOP;
            }
            else if (upDown == 0)
            {
                ret = XI_HAT_BOTTOM;
            }

            return ret;
            break;
    }
}
