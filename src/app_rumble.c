#include "hoja_includes.h"
#include "interval.h"

bool rumble_on = false;
bool ramp_up = false;
bool ramp_down = false;
const uint32_t _rumble_interval = 8000;

void app_rumble_task(uint32_t timestamp)
{
    static int lvl = 0;

    if(interval_run(timestamp, _rumble_interval))
    {
        if (rumble_on)
        {
            lvl += 10;
            if (lvl >= 110)
            {
                lvl = 110;
            }
            pwm_set_gpio_level(PGPIO_RUMBLE_BRAKE, 0);
            pwm_set_gpio_level(PGPIO_RUMBLE_MAIN, lvl);
        }
        else
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
        }
    }
}

void cb_hoja_rumble_enable(bool enable)
{
    rumble_on = enable;
}
