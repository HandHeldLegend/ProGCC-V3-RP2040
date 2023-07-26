#include "webusb.h"

void webusb_command_processor(uint8_t *data)
{
    switch(data[0])
    {
        default:

        break;

        // Set RGB Group
        case WEBUSB_CMD_RGB:
            {

                rgb_s col = 
                {
                    .r = data[2],
                    .g = data[3],
                    .b = data[4],
                };
                rgb_set_group(data[1], col.color);
            }
            break;
    }
}