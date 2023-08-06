#include "snapback.h"
#include <math.h>

#define SB_0 0
#define SB_1 0.2
#define SB_2 0.4
#define SB_3 0.6
#define SB_4 0.78
#define SB_5 0.88
#define SB_6 0.98
#define SB_7 1

#define SNAPBACK_COUNT 20
#define VEL_COUNT 16

float _snapback_lut[8] = {SB_0, SB_1, SB_2, SB_3, SB_4, SB_5, SB_6, SB_7};

uint32_t _timestamp_delta(uint32_t new, uint32_t old)
{
    if(new>=old)
    {
        return new-old;
    }
    else if (new<old)
    {
        uint32_t d = 0xFFFFFFFF - old;
        return d + new;
    }
    
    return 0;
}

#define ARC_MAX_WIDTH 25
#define ARC_MIN_WIDTH 6
#define ARC_MAX_LOOP ARC_MAX_WIDTH-1
#define CENTERVAL 2048
#define MAXVAL 4095
#define ARC_MAX_HEIGHT 355

typedef struct
{
    int points[ARC_MAX_WIDTH];
    uint8_t width;
    int height;
    uint8_t timer;
    int direction;
    uint8_t start_idx;
} arc_s;

typedef struct
{
    int buffer[ARC_MAX_WIDTH];
    int last_pos;
    uint8_t idx;
    arc_s arc;
    bool full;
    bool crossed;
} axis_s;

inline bool _is_between(int A, int B, int C)
{
    if (B < C) 
    {
        return (B < A) && (A <= C);
    } 
    else 
    {
        return (C < A) && (A <= B);
    }

    return false;
}

/**
 * Adds value to axis_s and returns the latest value
*/
int _add_axis(int v, axis_s *a)
{
    int ret = CENTERVAL;

    if (a->full)
    {
        ret = a->buffer[a->idx];
    }

    a->buffer[a->idx] = v;

    if (a->arc.timer < ARC_MAX_WIDTH)
    {
        a->arc.points[a->arc.timer] = v;
    }

    // First, check if we crossed once
    if(_is_between(CENTERVAL, v, a->last_pos))
    {

        if(!a->crossed)
        {
            a->crossed = true;
        }
        // If we get here
        // our arc is complete and within the width(time) limit
        else if(a->arc.timer >= ARC_MIN_WIDTH)
        {
            
            // Determine max height along the arc
            int height = 0;
            for(uint8_t h = 0; h <= a->arc.timer; h++)
            {   
                int hc = abs(a->arc.points[h] - CENTERVAL);
                if( hc > height) height = hc;
            }

            if (height >= ARC_MAX_HEIGHT)
            {
                //printf("ARC: %i\n", height);
                // Calculate scaler for this arc
                float height_scale = ARC_MAX_HEIGHT/(float) height;
                //printf("SCALE: %f\n", height_scale);
                //printf("TIME: %d\n", a->arc.timer);

                // Scale all the values directly into the output buffer
                // This essentially overwrites what would be values that are
                // snapping back.
                //printf("OWI: %d\n", a->arc.start_idx);
                uint8_t si = a->arc.start_idx;
                for(uint8_t i = 0; i < a->arc.timer; i++)
                {
                    int new_h = abs(a->arc.points[i] - CENTERVAL);
                    float new_val = ((float) new_h * height_scale * (int) a->arc.direction);
                    //printf("nf: %d, ", (int) new_val);

                    a->buffer[si] = (int) new_val + CENTERVAL;
                    //printf("nb %d, ", (int) a->buffer[si]); 
 
                    si = (si + 1) % ARC_MAX_WIDTH; // Add and wrap if needed
                }
                //printf("\n");
            }
        }
        
        // Now that we have written all of the values,
        // we should reset our arc
        a->arc.start_idx = a->idx;
        a->arc.points[0] = v;
        a->arc.timer = 0;
        if(v < a->last_pos)
        {
            a->arc.direction = -1;
        }
        else 
        {
            a->arc.direction = 1;
        }
    }
    
    a->arc.timer++;
    if (a->arc.timer >= ARC_MAX_WIDTH)
    {
        a->crossed = false;
        a->arc.timer = 0;
    }

    a->idx = (a->idx + 1) % ARC_MAX_WIDTH;
    if(a->idx == 0) a->full = true;

    // Update last pos
    a->last_pos = v;
    // Return the first in value
    return ret;
}

void snapback_process(uint32_t timestamp, a_data_s *input, a_data_s *output)
{
    static axis_s lx = {0};
    static axis_s ly = {0};
    static axis_s rx = {0};
    static axis_s ry = {0};

    output->lx = _add_axis(input->lx, &lx);
    output->ly = _add_axis(input->ly, &ly);
    output->rx = _add_axis(input->rx, &rx);
    output->ry = _add_axis(input->ry, &ry);
}

uint8_t _snapback_report[64] = {0};

#define CLAMP_0_255(value) ((value) < 0 ? 0 : ((value) > 255 ? 255 : (value)))

bool _snapback_add_value(int val)
{
    static uint8_t _idx = 0;
    int t = val >> 4;
    _snapback_report[_idx + 2] = (uint8_t)t;
    _idx += 1;
    if (_idx > 61)
    {
        _idx = 0;
        return true;
    }
    return false;
}

#define CAP_OFFSET 200
#define UPPER_CAP 4095 - CAP_OFFSET
#define LOWER_CAP 0 + CAP_OFFSET
#define CAP_INTERVAL 1000

void snapback_webcapture_task(uint32_t timestamp, a_data_s *data)
{
    if (!tud_vendor_n_mounted(0)) return;

    if (interval_run(timestamp, CAP_INTERVAL))
    {

        static bool _capturing = false;
        static int *selection = NULL;
        static bool _got_selection = false;
        static uint8_t _selection_idx = 0;

        if (_capturing)
        {
            if (_snapback_add_value(*selection))
            {
                // Send packet
                _snapback_report[0] = WEBUSB_CMD_ANALYZE_STOP;
                _snapback_report[1] = _selection_idx;
                tud_vendor_n_write(0, _snapback_report, 64);
                tud_vendor_n_flush(0);
                _capturing = false;
                _got_selection = false;
            }
        }
        else if (!_got_selection)
        {
            if (data->lx >= UPPER_CAP || data->lx <= LOWER_CAP)
            {
                selection = &(data->lx);
                _got_selection = true;
                _selection_idx = 0;
            }
            else if (data->ly >= UPPER_CAP || data->ly <= LOWER_CAP)
            {
                selection = &(data->ly);
                _got_selection = true;
                _selection_idx = 1;
            }
            else if (data->rx >= UPPER_CAP || data->rx <= LOWER_CAP)
            {
                selection = &(data->rx);
                _got_selection = true;
                _selection_idx = 2;
            }
            else if (data->ry >= UPPER_CAP || data->ry <= LOWER_CAP)
            {
                selection = &(data->ry);
                _got_selection = true;
                _selection_idx = 3;
            }
        }
        else if (_got_selection)
        {
            if (*selection<UPPER_CAP && *selection>LOWER_CAP)
            {
                _capturing = true;
            }
        }
    }
}
