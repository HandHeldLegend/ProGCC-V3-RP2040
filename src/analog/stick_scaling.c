#include "stick_scaling.h"
#include <math.h>

#define SCALER_V (float) 128.0f
#define SCALER_10_BIT (float) 510.0f
#define SCALE_10_TO_8 (float) 0.25f
#define STICK_INTERNAL_CENTER 512
#define STICK_OUT_CENTER 128
#define DEAD_ZONE 20

#define ANGLE_TOLERANCE 1

#define CLAMP_0_255(value) ((value) < 0 ? 0 : ((value) > 255 ? 255 : (value)))

// Stores internal
// calibration data
a_calib_center_s c = {
    .lx_center = 128,
    .ly_center = 128,
    .rx_center = 128,
    .ly_center = 128,
};

// Stores processed XY Scaled
// as a high precision float before
// processing angular modifications
af_data_s pa = {0};

// Temporary data to store high/low values
a_calib_hilo_s chl = {0};

// XY Scalers
float ls_x_scaler_lo = 0;
float ls_x_scaler_hi = 0;

float rs_x_scaler_lo = 0;
float rs_x_scaler_hi = 0;

float ls_y_scaler_lo = 0;
float ls_y_scaler_hi = 0;

float rs_y_scaler_lo = 0;
float rs_y_scaler_hi = 0;

// Init scalers
// Pairs 0/1, 2/3, 4/5, 6/7
float l_scalers[8] = {0};
float r_scalers[8] = {0};

float l_angle_scalers[8] = {0};
float r_angle_scalers[8] = {0};

// Diagonal cardinal dividers
// IE where the crossover point is for the 45 degree mark
float l_angles[4] = {45.0f, 45.0f, 45.0f, 75.0f};
float r_angles[4] = {45.0f, 45.0f, 45.0f, 45.0f};

int angles[4] = {0, 90, 180, 270};

// INTERNAL PROCESSING

// Function to scale an input by value according
// to float scalers
float scale_by_hilo(int input, int center, float s_high, float s_low)
{
  float out = STICK_INTERNAL_CENTER;
  if (input > center)
  {
    float newx = ((float) input - (float) center) * s_high;
    out += newx;
  }
  else if (input < center)
  {
    float newy = ((float) center - (float) input) * s_low;
    out -= newy;
  }

  return out;
}

// Processes incoming data and stores it in our temporary
// buffer
void process_xy_scale(a_data_s *in)
{
  pa.lx = scale_by_hilo(in->lx, c.lx_center, ls_x_scaler_hi, ls_x_scaler_lo);
  pa.ly = scale_by_hilo(in->ly, c.ly_center, ls_y_scaler_hi, ls_y_scaler_lo);

  pa.rx = scale_by_hilo(in->rx, c.rx_center, rs_x_scaler_hi, rs_x_scaler_lo);
  pa.ry = scale_by_hilo(in->ry, c.ry_center, rs_y_scaler_hi, rs_y_scaler_lo);
}

// Takes in 4 diagonal angles, outputs 8 appropriate scalers
void calculate_angle_scalers(float *angles_in, float *scalers_out)
{
  uint8_t t = 0;
  for(uint8_t i = 0; i < 4; i++)
  {
    // Calculate percentage of first scaler
    // based on the set modified angle
    float s1 = 45.0f/angles_in[i];
    // Get second scaler from first
    float s2 = 45.0f/(90.0f-angles_in[i]);

    scalers_out[t]    = s1;
    scalers_out[t+1]  = s2;
    t+=2;
  }
}

// Setup function to calculate our XY Scalers
void calculate_xy_scalers()
{
  ls_x_scaler_hi = SCALER_10_BIT / ( (float) chl.lx_hi - (float) c.lx_center );
  ls_x_scaler_lo = SCALER_10_BIT / ( (float) c.lx_center - (float) chl.lx_lo );

  rs_x_scaler_hi = SCALER_10_BIT / ( (float) chl.ry_hi - (float) c.ry_center );
  rs_x_scaler_lo = SCALER_10_BIT / ( (float)c.ry_center - (float) chl.ry_lo );

  ls_y_scaler_hi = SCALER_10_BIT / ( (float) chl.ly_hi - (float) c.ly_center );
  ls_y_scaler_lo = SCALER_10_BIT / ( (float) c.ly_center - (float) chl.ly_lo );

  rs_y_scaler_hi = SCALER_10_BIT / ( (float) chl.ry_hi - (float) c.ry_center );
  rs_y_scaler_lo = SCALER_10_BIT / ( (float) c.ry_center - (float) chl.ry_lo );
}

// Returns the octant to let us know which
// diagonal angle we are adjusting
int get_angle_adjust_index(float a)
{
    int o = (int) a/90 ;
    return o;
}

// Produces a normalized vector at a given angle
void normalized_vector(float angle, float *x, float *y)
{
  float rad = angle * (M_PI / 180.0);
  *x = cos(rad);
  *y = sin(rad);
}

// Returns the float angle given the XY coordinate pair and the
// calibrated center point
float get_angle(float x, float y) {
    float angle = atan2f( (y - SCALER_10_BIT), (x - SCALER_10_BIT) ) * 180.0f / M_PI;
    if (angle < 0) {
        angle += 360.0f;
    }
    return angle;
}

// Returns float distance between float coordinate pair
// and 512,512 coordinate center point
float get_distance(float x, float y) {
    float dx = x - SCALER_10_BIT;
    float dy = y - SCALER_10_BIT;
    return sqrtf(dx * dx + dy * dy);
}

// -- END PRIVATE -- //

// PUBLIC FUNCTIONS

// Process analog data according to scaler
void stick_scaling_process_data(a_data_s *in, a_data_s *out)
{
  // Scale our raw values to 10 bit
  process_xy_scale(in);

  // Calculate the distance of the stick
  float ld = get_distance(pa.lx, pa.ly);
  float rd = get_distance(pa.rx, pa.ry);

  // Process LEFT stick
  if (ld >= DEAD_ZONE)
  {
    // Scale our distance to 8 bit range
    ld *= SCALE_10_TO_8;

    // Capture our angles from 10 bit vals
    float la = get_angle(pa.lx, pa.ly);

    // Generate modified angles SECTION
    // First we get the index based on
    // our angle (0-3)
    int la_idx = get_angle_adjust_index(la);
    int ls_idx = la_idx*2;

    float la_new = 0;
    float la_mod = 0;
    float l_diff = 0;

    // Get our base angle to modify
    la_mod = fmod(la, 90);

    // Now we check if we are in the first or second segment
    // so we know which scaler we should use
    if (la_mod > l_angles[la_idx])
    {
      // Get our remaining angle travel
      l_diff = la_mod - l_angles[la_idx];

      // Increase index to next
      ls_idx += 1;
      la_new = angles[la_idx] + 45.0f + (l_diff * l_angle_scalers[ls_idx]);
    }
    else
    {
      la_new = angles[la_idx] + (la_mod * l_angle_scalers[ls_idx]);
    }

    // Generate normalized vector data
    // based on our new adjusted angles
    float nlx = 0;
    float nly = 0;
    normalized_vector(la_new, &nlx, &nly);

    // Finally, scale our values up accordingly
    nlx *= ld;
    nly *= ld;

    out->lx = CLAMP_0_255((int) roundf(nlx + 128));
    out->ly = CLAMP_0_255((int) roundf(nly + 128));
  }
  else
  {
    out->lx = STICK_OUT_CENTER;
  }
}

// Captures the center point or sets it with input data
void stick_scaling_capture_center(a_data_s *input)
{
  c.lx_center = input->lx;
  c.ly_center = input->ly;

  c.rx_center = input->rx;
  c.ry_center = input->ry;

  // Reset hi/lo vals
  chl.lx_lo = input->lx;
  chl.lx_hi = input->lx;
  chl.ly_lo = input->ly;
  chl.ly_hi = input->ly;
  chl.rx_lo = input->rx;
  chl.rx_hi = input->rx;
  chl.ry_lo = input->ry;
  chl.ry_hi = input->ry;
}

// Captures the high and low values and sets accordingly
void stick_scaling_capture_hilo(a_data_s *input)
{
  // Left X
  if (input->lx < chl.lx_lo)
  {
    chl.lx_lo = input->lx;
  }
  else if (input->lx > chl.lx_hi)
  {
    chl.lx_hi = input->lx;
  }

  // Left Y
  if (input->ly < chl.ly_lo)
  {
    chl.ly_lo = input->ly;
  }
  else if (input->ly > chl.ly_hi)
  {
    chl.ly_hi = input->ly;
  }

  // Right X
  if (input->rx < chl.rx_lo)
  {
    chl.rx_lo = input->rx;
  }
  else if (input->rx > chl.rx_hi)
  {
    chl.rx_hi = input->rx;
  }

  // Right Y
  if (input->ry < chl.ry_lo)
  {
    chl.ry_lo = input->ry;
  }
  else if (input->ry > chl.ry_hi)
  {
    chl.ry_hi = input->ry;
  }
}

void stick_scaling_init()
{
  calculate_angle_scalers(l_angles, l_angle_scalers);
  calculate_angle_scalers(r_angles, r_angle_scalers);
}

void stick_scaling_finalize()
{
  calculate_xy_scalers();
}
