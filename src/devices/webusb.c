#include "webusb.h"

uint8_t _webusb_out_buffer[64] = {0x00};

void webusb_save_confirm()
{
    printf("Sending Save receipt...\n");
    memset(_webusb_out_buffer, 0, 64);
    _webusb_out_buffer[0] = 0xF1;
    tud_vendor_n_write(0, _webusb_out_buffer, 64);
    tud_vendor_n_flush(0);
}

void webusb_command_processor(uint8_t *data)
{
    switch(data[0])
    {
        default:

        break;

        // Set RGB Group
        case WEBUSB_CMD_RGB:
            {
                printf("WebUSB: Got RGB command.\n");
                rgb_s col = 
                {
                    .r = data[2],
                    .g = data[3],
                    .b = data[4],
                };
                rgb_set_group(data[1], col.color);
            }
            break;

        case WEBUSB_CMD_SNAPBACK:
            {
                printf("WebUSB: Got Snapback SET command.\n");
                settings_set_snapback(data[1], data[2]);
            }
            break;

        case WEBUSB_CMD_SNAPBACK_GET:
            {
                printf("WebUSB: Got Snapback GET command.\n");
                _webusb_out_buffer[0] = WEBUSB_CMD_SNAPBACK_GET;
                _webusb_out_buffer[1] = global_loaded_settings.lx_snapback;
                _webusb_out_buffer[2] = global_loaded_settings.ly_snapback;
                _webusb_out_buffer[3] = global_loaded_settings.rx_snapback;
                _webusb_out_buffer[4] = global_loaded_settings.ry_snapback;
                tud_vendor_n_write(0, _webusb_out_buffer, 64);
                tud_vendor_n_flush(0);
            }
            break;

        case WEBUSB_CMD_SAVEALL:
            {
                printf("WebUSB: Got SAVE command.\n");
                settings_save(true);
            }
            break;
    }
}