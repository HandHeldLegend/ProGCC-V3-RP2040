#include "snapback.h"

float _snapback_lut_a[8] = {0, 0.14f, 0.28f, 0.42f, 0.56f, 0.70f, 0.84f, 1.0f};
float _snapback_lut_b[8] = {1.0f, 0.86f, 0.72f, 0.58f, 0.44f, 0.30f, 0.16f, 0};

void snapback_process(a_data_s *input, a_data_s *output)
{
    static int lx_t = 0;
    static int ly_t = 0;
    static int rx_t = 0;
    static int ry_t = 0;

    static float lx_avg = 0;
    static float ly_avg = 0;
    static float rx_avg = 0;
    static float ry_avg = 0;

    static int lx[SNAPBACK_COUNT] = {0};
    static int ly[SNAPBACK_COUNT] = {0};
    static int rx[SNAPBACK_COUNT] = {0};
    static int ry[SNAPBACK_COUNT] = {0};

    // Store incoming data as float
    float lx_current = input->lx;
    float ly_current = input->ly;
    float rx_current = input->rx;
    float ry_current = input->ry;

    static uint8_t idx = 0;
    static uint8_t sample_count = 0;

    // Add to our sample count 
    if (sample_count<SNAPBACK_COUNT)
    {
        sample_count += 1;
    }
    // If we are at our sample count, subtract the last item
    {
        lx_t -= lx[idx];
        ly_t -= ly[idx];
        rx_t -= rx[idx];
        ry_t -= ry[idx];
    }

    // First, add the new value to our array
    lx[idx] = input->lx;
    ly[idx] = input->ly;
    rx[idx] = input->rx;
    ry[idx] = input->ry;

    // Next, add the new value to our running total
    lx_t += input->lx;
    ly_t += input->ly;
    rx_t += input->rx;
    ry_t += input->ry;

    // Perform our average operation
    lx_avg = (float) lx_t/SNAPBACK_COUNT;
    ly_avg = (float) ly_t/SNAPBACK_COUNT;
    rx_avg = (float) rx_t/SNAPBACK_COUNT;
    ry_avg = (float) ry_t/SNAPBACK_COUNT;

    // Perform our blend operation
    lx_current *= _snapback_lut_b[global_loaded_settings.lx_snapback];
    ly_current *= _snapback_lut_b[global_loaded_settings.ly_snapback];
    rx_current *= _snapback_lut_b[global_loaded_settings.rx_snapback];
    ry_current *= _snapback_lut_b[global_loaded_settings.ry_snapback];
    
    // Add to the final portion
    lx_current += lx_avg * _snapback_lut_a[global_loaded_settings.lx_snapback];
    ly_current += ly_avg * _snapback_lut_a[global_loaded_settings.ly_snapback];
    rx_current += rx_avg * _snapback_lut_a[global_loaded_settings.rx_snapback];
    ry_current += ry_avg * _snapback_lut_a[global_loaded_settings.ry_snapback];

    // Set outputs
    output->lx = (int) lx_current;
    output->ly = (int) ly_current;
    output->rx = (int) rx_current;
    output->ry = (int) ry_current;

    // Add one to our index
    idx += 1;
    if (idx == SNAPBACK_COUNT) idx = 0;
}