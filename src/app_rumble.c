#include "hoja_includes.h"
#include "interval.h"
#include "main.h"

const uint32_t _rumble_interval = 8000;
int _rumble_intensity = 0;
int _rumble_current = 0;

#define RUMBLE_MAX 100
static uint8_t _rumble_max = RUMBLE_MAX;

static bool _rumble = false;
static bool _declining = false;

void app_rumble_task(uint32_t timestamp)
{
    static int lvl = 0;

    if(interval_run(timestamp, _rumble_interval))
    {
        if ( (lvl < _rumble_intensity) && !_declining)
        {
            lvl += 10;
            if (lvl >= _rumble_intensity)
            {
                lvl = _rumble_intensity;
                _declining = true;
            }
            pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 0);
            pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, lvl);
        }
        else if (_declining && (lvl > _rumble_current))
        {
            lvl -= 20;
            if (lvl <= 0)
            {
                lvl = 0;
                pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, 0);
                pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 255);
            }
            else
            {
                pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, lvl);
            }
            _rumble_intensity = lvl;
        }
    }
}

void cb_hoja_rumble_enable(float intensity)
{
    if(intensity > 1.0f) intensity = 1.0f;
    
    float p = _rumble_max * intensity;
    uint16_t tmp = (uint16_t) p;
    if(tmp>_rumble_intensity)
    {
        _rumble_intensity = tmp;
        _declining = false;
    }

    if(!intensity) 
    {
        _rumble_current = 0;
    }
    else _rumble_current = tmp;

}

void cb_hoja_set_rumble_intensity(uint8_t intensity)
{   
    _rumble_max = intensity;
    cb_hoja_rumble_enable(1);
    sleep_ms(350);
    cb_hoja_rumble_enable(0);
}
