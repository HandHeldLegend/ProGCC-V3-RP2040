#include "hoja_includes.h"
#include "interval.h"
#include "main.h"

const uint32_t _rumble_interval = 8000;
int _rumble_intensity = 0;

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
        if(_declining)
        {
            _rumble_current -= 20;
            if (_rumble_current < 0)
            {
                _rumble_current = 0;
            }
            _rumble_intensity = _rumble_current;
        }
        else
        {
            if (_rumble_current < _rumble_intensity)
            {
                _rumble_current += 10;

                if (_rumble_current >= _rumble_intensity)
                {
                    _rumble_current = _rumble_intensity;
                }
            }
            else _declining = true;

        }
        
        pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, (!_rumble_current) ? 255 : 0);
        pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, (_rumble_current > 0) ? _rumble_current : 0);

    }
}

void cb_hoja_rumble_enable(float intensity)
{
    if(intensity > 1.0f) intensity = 1.0f;

    if(!intensity)
    {
        // Do nothing
    }
    else 
    {
        float p = _rumble_max * intensity;
        uint16_t tmp = (uint16_t) p;
        if(tmp>_rumble_intensity)
        {
            _rumble_intensity = tmp;
        }
        _declining = false;
    }
    
    
}

void cb_hoja_set_rumble_intensity(uint8_t intensity)
{   
    if(!intensity)
    {
        _rumble_max = 0;
    }
    else
    {
        _rumble_max = 50 + (intensity/2);
    }
    
    cb_hoja_rumble_enable(1);
    sleep_ms(350);
    cb_hoja_rumble_enable(0);
}
