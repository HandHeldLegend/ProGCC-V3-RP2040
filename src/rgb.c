#include "rgb.h"

// 21 steps is about 0.35 seconds
// Formula is time period us / 16666us (60hz)
#define RGB_FADE_STEPS 21

uint8_t _rgb_anim_steps = 0;
bool _rgb_out_dirty = false;

rgb_s _rgb_next[NUM_PIXELS]     = {0};
rgb_s _rgb_current[NUM_PIXELS]  = {0};
rgb_s _rgb_last[NUM_PIXELS]     = {0};

// Returns a bool to indicate whether or not
// an animation frame should occur
bool _rgb_update_ready(uint32_t timestamp)
{
    static uint32_t last_time   = 0;
    static uint32_t this_time   = 0;

    this_time = timestamp;

    // Clear variable
    uint32_t diff = 0;

    // Handle edge case where time has
    // looped around and is now less
    if (this_time <= last_time)
    {
        diff = (0xFFFFFFFF - last_time) + this_time;
    }
    else
    {
        diff = this_time - last_time;
    }

    // We want a target LED rate of ~60hz
    // 16666us
    if (diff >= 16666)
    {
        // Set the last time
        last_time = this_time;
        return true;
    }
    return false;
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void _rgb_update_all()
{
    for(uint8_t i = 0; i < NUM_PIXELS; i++)
    {
        pio_sm_put_blocking(RGB_PIO, RGB_SM, _rgb_current[i].color);
    }
}

uint32_t _rgb_blend(rgb_s *original, rgb_s *new, float blend)
{
    float or = (float) original->r;
    float og = (float) original->g;
    float ob = (float) original->b;
    float nr = (float) new->r;
    float ng = (float) new->g;
    float nb = (float) new->b;
    float outr = or + ((nr-or)*blend);
    float outg = og + ((ng-og)*blend);
    float outb = ob + ((nb-ob)*blend);
    rgb_s col = {
        .r = (uint8_t) outr,
        .g = (uint8_t) outg,
        .b = (uint8_t) outb
    };
    return col.color;
}

void _rgb_animate_step()
{
    static uint8_t steps = RGB_FADE_STEPS;
    const float blend_step = 1.0f/RGB_FADE_STEPS;
    bool done = true;

    if (_rgb_out_dirty)
    {
        memcpy(_rgb_last, _rgb_current, sizeof(_rgb_last));
        steps = 0;
        _rgb_out_dirty = false;
        done = false;
    }

    if (steps < RGB_FADE_STEPS)
    {
        float blender = blend_step * (float) steps;
        // Blend between old and next colors appropriately
        for(uint8_t i = 0; i < NUM_PIXELS; i++)
        {
            _rgb_current[i].color = _rgb_blend(&_rgb_last[i], &_rgb_next[i], blender);
        }
        steps++;
        _rgb_update_all();
    }
    else if (!done)
    {
        memcpy(_rgb_current, _rgb_next, sizeof(_rgb_next));
        _rgb_update_all();
        done = true;
    }
}

// Enable the RGB transition to the next color
void _rgb_set_dirty()
{
    _rgb_out_dirty = true;
}

// Set all RGBs to one color
void rgb_set_all(uint32_t color)
{
    for(uint8_t i = 0; i < NUM_PIXELS; i++)
    {
        _rgb_next[i].color = color;
    }
    _rgb_out_dirty = true;
}

void rgb_set_group(uint8_t group)
{

}

void rgb_init()
{
    uint offset = pio_add_program(RGB_PIO, &ws2812_program);
    ws2812_program_init(RGB_PIO, RGB_SM, offset, WS2812_PIN, IS_RGBW);
}

// One tick of RGB logic
// only performs actions if necessary
void rgb_tick(uint32_t timestamp)
{
    if(_rgb_update_ready(timestamp))
    {
        _rgb_animate_step();
    }
}
