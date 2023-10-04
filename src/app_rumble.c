#include "hoja_includes.h"
#include "interval.h"
#include "main.h"

const uint32_t _rumble_interval = 8000;


int _rumble_cap = 0;

int _rumble_floor = 0;

int _rumble_current = 0;

#define RUMBLE_MAX 100
static uint8_t _rumble_max = RUMBLE_MAX;

static bool _declining = false;

/**
 * pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 0);
 * pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, _rumble_current);
*/

void app_rumble_task(uint32_t timestamp)
{
    if(interval_run(timestamp, _rumble_interval))
    {
        if (_rumble_current < _rumble_cap)
        {
            _rumble_current += 20;
        }
        else
        {
            _rumble_current -= 10;
            
            if (_rumble_current <= _rumble_floor)
            {
                _rumble_current = _rumble_floor; 
            }

            _rumble_cap = _rumble_current;
        }
        
        pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, (!_rumble_current) ? 255 : 0);
        pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, (_rumble_current > 0) ? _rumble_current : 0);
    }
}

void cb_hoja_rumble_enable(float intensity)
{
    if(intensity > 1.0f) intensity = 1.0f;

    float p = _rumble_max * intensity;
    uint16_t tmp = (uint16_t) p;

    _rumble_floor = tmp;
    if(tmp>_rumble_cap)
    {
        _rumble_cap = tmp;
    }

}

void cb_hoja_set_rumble_intensity(uint8_t floor, uint8_t intensity)
{   
    floor       = (floor > 50)      ? 50 : floor;
    intensity   = (intensity > 50)  ? 50 : intensity;

    if(!intensity)
    {
        _rumble_max = 0;
    }
    else
    {
        _rumble_max = floor + intensity;
    }
    
    cb_hoja_rumble_enable(1);
    sleep_ms(350);
    cb_hoja_rumble_enable(0);
}
