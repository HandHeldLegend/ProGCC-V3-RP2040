#include "hoja_comms.h"

comms_cb_t _comms_cb = NULL;

void hoja_comms_task(uint32_t timestamp, button_data_s * buttons, a_data_s * analog)
{
    if(_comms_cb != NULL)
    {
        _comms_cb(buttons, analog);
    }
}

void hoja_comms_init(comm_mode_t comms_mode)
{
    switch(comms_mode)
    {
        case COMM_MODE_GC:
            _comms_cb = gamecube_comms_task;
            gamecube_init();
        break;
    }
}