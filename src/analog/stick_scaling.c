#include "stick_scaling.h"
#include <math.h>

#define SCALER_V (float) 128.0f
#define ANGLE_TOLERANCE 1
#define DEAD_ZONE 25
#define CLAMP_0_255(value) ((value) < 0 ? 0 : ((value) > 255 ? 255 : (value)))

a_calibration_s c = {
    .lx_center = 128,
    .ly_center = 128,
    .rx_center = 128,
    .ly_center = 128,
};

// Init scalers
float l_scalers[8] = {0};
float r_scalers[8] = {0};
int angles[8] = {0, 45, 90, 135, 180, 225, 270, 315};
float dl[8] = {0};
float dr[8] = {0};

// Returns a float angle given the XY coordinate pair and the
// calibrated center point
float get_angle(int x, int y, int center_x, int center_y) {
    float angle = atan2f((float) (y - center_y), (float) (x - center_x)) * 180.0f / M_PI;
    if (angle < 0) {
        angle += 360.0f;
    }
    return angle;
}

// Returns float distance between two int coordinate pairs
float get_distance(int x, int y, int center_x, int center_y) {
    float dx = (float) (x - center_x);
    float dy = (float) (y - center_y);
    return sqrtf(dx * dx + dy * dy);
}

// Takes in XY Coordinate, XY calibrated center, scaling factor, and outputs an adjusted XY coordinate
void adjust_coordinates(a_data_s *in, a_calibration_s *c, float l_scaling_factor, float r_scaling_factor, a_data_s *out) {
    float ldx = (float) (in->lx - c->lx_center);
    float ldy = (float) (in->ly - c->ly_center);

    float rdx = (float) (in->rx - c->rx_center);
    float rdy = (float) (in->ry - c->ry_center);

    float ldistance = sqrtf(ldx * ldx + ldy * ldy);
    float rdistance = sqrtf(rdx * rdx + rdy * rdy);

    if (ldistance >= DEAD_ZONE)
    {
      ldistance-=DEAD_ZONE;
      float nlx = ldx / ldistance;
      float nly = ldy / ldistance;

      float ladj = (ldistance * l_scaling_factor);

      nlx *= ladj;
      nly *= ladj;

      if(nlx > 300) return;
      if(nly > 300) return;

      out->lx = CLAMP_0_255((int) roundf(nlx + 128));
      out->ly = CLAMP_0_255((int) roundf(nly + 128));


    }
    else
    {
      out->lx = 128;
      out->ly = 128;
    }

    if (rdistance >= DEAD_ZONE)
    {
      rdistance-=DEAD_ZONE;
      float nrx = rdx / rdistance;
      float nry = rdy / rdistance;

      float radj = (rdistance * r_scaling_factor);

      nrx *= radj;
      nry *= radj;

      if(nrx > 300) return;
      if(nry > 300) return;

      out->rx = CLAMP_0_255((int) roundf(nrx + 128));
      out->ry = CLAMP_0_255((int) roundf(nry + 128));
    }
    else
    {
      out->rx = 128;
      out->ry = 128;
    }

}

// Returns an index based on the angle
int get_angle_index(float a)
{
  // Add half of 45 degree increment.
  float s = a+22.5f;
  // As an example, we want to ensure that we increment
  // up to index 1 when we are at 22.5 degrees since
  // we are chopping each area in half.

  // Process the angle when it's above 337.5f (upper range of idx 7),
    if (a > 337.5f)
    {
      return 0;
    }

    int o = (int) s/45 ;
    return o;
}

// Return a float scaler based on the angle
float get_scaler(float a, float *s)
{
    int i_a = get_angle_index(a-22.5f);

    int i_b = i_a+1;

    if (i_a >= 7)
    {
        i_a = 7;
        i_b = 0;
    }

    // Determine angle we can
    // comp against 45 degrees
    // This gives us a ratio to apply
    // the scalers
    float o = (a-angles[i_a])/45.0f;
    float o2 = 1.0f-o;

    float out = (o2*s[i_a]) + (o*s[i_b]);
    return out;
}

// PUBLIC FUNCTIONS

// Takes in analog data and constantly updates
// the analog scalers appropriately
void stick_scaling_create_scalers(a_data_s *analog_input)
{
  // Calculate angles
  float la = get_angle(analog_input->lx, analog_input->ly, c.lx_center, c.ly_center);
  float ra = get_angle(analog_input->rx, analog_input->ry, c.rx_center, c.ry_center);

  if (!((int) la % 45))
  {
    // First, let's calculate the distance and angle index
    float ld = get_distance(analog_input->lx, analog_input->ly, c.lx_center, c.ly_center);

    // Get angle index for left and right sticks
    int li = get_angle_index(la);

    // Calculate the scaler
    float ls = SCALER_V/(ld-DEAD_ZONE);

    // If the distance is further than what we have so far
    // store the scaler and distance
    if (ld > dl[li])
    {
      dl[li] = ld;
      l_scalers[li] = ls;
    }
  }

  if (!((int) ra % 45))
  {
    float rd = get_distance(analog_input->rx, analog_input->ry, c.rx_center, c.ry_center);
    // Get angle index for left and right sticks
    int ri = get_angle_index(get_angle(analog_input->rx, analog_input->ry, c.rx_center, c.ry_center));

    // Calculate the scaler
    float rs = SCALER_V/(rd-DEAD_ZONE);

    // If the distance is further than what we have so far
    // store the scaler and distance
    if (rd > dr[ri])
    {
      dr[ri] = rd;
      r_scalers[ri] = rs;
    }
  }
}

void stick_scaling_print_scalers()
{
  printf("\n\nL STICK SCALERS:\n\n");
  printf("S0: %f\n", l_scalers[0]);
  printf("S1: %f\n", l_scalers[1]);
  printf("S2: %f\n", l_scalers[2]);
  printf("S3: %f\n", l_scalers[3]);
  printf("S4: %f\n", l_scalers[4]);
  printf("S5: %f\n", l_scalers[5]);
  printf("S6: %f\n", l_scalers[6]);
  printf("S7: %f\n", l_scalers[7]);

  printf("\n\nR STICK SCALERS:\n\n");
  printf("S0: %f\n", r_scalers[0]);
  printf("S1: %f\n", r_scalers[1]);
  printf("S2: %f\n", r_scalers[2]);
  printf("S3: %f\n", r_scalers[3]);
  printf("S4: %f\n", r_scalers[4]);
  printf("S5: %f\n", r_scalers[5]);
  printf("S6: %f\n", r_scalers[6]);
  printf("S7: %f\n", r_scalers[7]);
}

void stick_scaling_print_centers()
{
  printf("\n\nSTICK CENTERS:\n\n");
  printf("LX C: %i\n", c.lx_center);
  printf("LY C: %i\n", c.ly_center);
  printf("RX C: %i\n", c.rx_center);
  printf("RY C: %i\n", c.ry_center);
}

// Process analog data according to scaler
void stick_scaling_process_data(a_data_s *in, a_data_s *out)
{
  float ls = get_scaler(get_angle(in->lx, in->ly, c.lx_center, c.ly_center), l_scalers);
  float rs = get_scaler(get_angle(in->rx, in->ry, c.rx_center, c.ry_center), r_scalers);

  adjust_coordinates(in, &c, ls, rs, out);
}

// Captures the center point or sets it with input data
void stick_scaling_capture_center(a_data_s *analog_input)
{
  c.lx_center = analog_input->lx;
  c.ly_center = analog_input->ly;

  c.rx_center = analog_input->rx;
  c.ry_center = analog_input->ry;
}
