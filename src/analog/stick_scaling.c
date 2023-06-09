#include "stick_scaling.h"
#include <math.h>

#define STICK_INTERNAL_CENTER 2048
#define STICK_MAX 4095
#define STICK_DEAD_ZONE 24
#define STICK_SCALE_DISTANCE STICK_INTERNAL_CENTER


#define ANGLE_TOLERANCE 1

#define CLAMP_0_MAX(value) ((value) < 0 ? 0 : ((value) > STICK_MAX ? STICK_MAX : (value)))

// Stores internal
// calibration data
a_calib_center_s c = {
    .lx_center = STICK_INTERNAL_CENTER,
    .ly_center = STICK_INTERNAL_CENTER,
    .rx_center = STICK_INTERNAL_CENTER,
    .ly_center = STICK_INTERNAL_CENTER,
};

// Init scalers
// These scalers are responsible for
// the distance scalers for each angle group.
// Starting straight East and going clockwise.
float l_distance_scalers[8] = {0};
float r_distance_scalers[8] = {0};

// These scalers are responsible for adjustments of
// our 45 degree angles.
float l_angle_scalers[8] = {0};
float r_angle_scalers[8] = {0};

// Diagonal cardinal dividers
// IE where the crossover point is for the 45 degree mark.
float l_angles[4] = {45.0f, 45.0f, 45.0f, 45.0f};
float r_angles[4] = {45.0f, 45.0f, 45.0f, 45.0f};

// Store distances for each angle
// for scaler calculation.

float l_angle_distances[8] = {0};
float r_angle_distances[8] = {0};

int distance_angles[8] = {0, 45, 90, 135, 180, 225, 270, 315};
int diagonal_angles[4] = {0, 90, 180, 270};

// INTERNAL PROCESSING

// Takes in 4 diagonal angles, outputs 8 appropriate scalers
void calculate_diagonal_scalers(float *angles_in, float *scalers_out)
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

// Calculate the angular scalers used to determine the distance scaler
void calculate_distance_scalers(float *distances_in, float *scalers_out)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    scalers_out[i] = (STICK_SCALE_DISTANCE) / (distances_in[i]-STICK_DEAD_ZONE);
  }
}

// Returns the quadrant to let us know which
// diagonal angle we are adjusting
int get_angle_adjust_index(float a)
{
    int o = (int) a/90 ;
    return o;
}

// Returns the appropriate distance index to tell
// us where the angle falls in the 8 sections
int get_distance_adjust_index(float a)
{
  if (a > 337.5f)
  {
    return 0;
  }

  if (a < 0)
  {
    return 0;
  }

  float a_n = a+22.5f;

  int o = (int) a_n/45;
  return o;
}

// Return the appripriate distance scaler
float get_distance_scaler(float angle, float *scalers)
{
  int idx_a = get_distance_adjust_index(angle);
  int idx_b = 0;

  float high_a_percent = 0;
  float low_a_percent = 0;

  // Calculate distance along angle, plus 22.5
  float a_d = fmod(angle, 45);

  // This is if we are below the direct cardinal
  if (a_d > 22.5)
  {
    if (!idx_a)
    {
      idx_b = 7;
    }
    else
    {
      idx_b = idx_a - 1;
    }

    high_a_percent = a_d/45;
    low_a_percent = 1.0f-high_a_percent;
  }
  // We are above the direct cardinal
  else
  {
    if (idx_a < 7)
    {
      idx_b = idx_a + 1;
    }

    low_a_percent = a_d/45;
    high_a_percent = 1.0f-low_a_percent;
  }

  // Return the combined scaler value
  return (scalers[idx_a] * high_a_percent) + (scalers[idx_b] * low_a_percent);
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
float get_angle(int x, int y, int center_x, int center_y) {
    float angle = atan2f( (y - center_y), (x - center_x) ) * 180.0f / M_PI;
    if (angle < 0) {
        angle += 360.0f;
    }
    return angle;
}

// Returns float distance between float coordinate pair
// and 512,512 coordinate center point
float get_distance(int x, int y, int center_x, int center_y) {
    float dx = (float) x - (float) center_x;
    float dy = (float) y - (float) center_y;
    return sqrtf(dx * dx + dy * dy);
}

// Takes in x/y coordinate, x/y center, and spits out
// the appropriately scaled distance.
float get_scaled_distance(float angle, int x, int y, int center_x, int center_y, float *scalers)
{
  // Get distance for angle
  float d = get_distance(x, y, center_x, center_y);
  if (d < STICK_DEAD_ZONE)
  {
    d = 0;
  }
  else
  {
    d -= STICK_DEAD_ZONE;
  }

  // Scale accordingly
  float s = get_distance_scaler(angle, scalers);

  return d * s;
}

float get_scaled_angle(float angle, float *angles, float *angle_scalers)
{
  // Generate modified angles SECTION
  // First we get the index based on
  // our angle (0-3)
  int a_idx = get_angle_adjust_index(angle);
  int s_idx = a_idx*2;

  float a_new = 0;
  float a_mod = 0;
  float a_diff = 0;

  // Get our base angle to modify
  a_mod = fmod(angle, 90);

  // Now we check if we are in the first or second segment
  // so we know which scaler we should use
  if (a_mod > angles[a_idx])
  {
    // Get our remaining angle travel
    a_diff = a_mod - angles[a_idx];

    // Increase index to next
    s_idx += 1;
    a_new = diagonal_angles[a_idx] + 45.0f + (a_diff * angle_scalers[s_idx]);
  }
  else
  {
    a_new = diagonal_angles[a_idx] + (a_mod * angle_scalers[s_idx]);
  }

  return a_new;
}

// -- END PRIVATE -- //

float last_l_angle;
float last_r_angle;

// PUBLIC FUNCTIONS
void stick_scaling_get_last_angles(float *la_out, float *ra_out)
{
  *la_out = last_l_angle;
  *ra_out = last_r_angle;
}

// Process analog data according to scaler
void stick_scaling_process_data(a_data_s *in, a_data_s *out)
{
  // Get angles of input
  float la = get_angle(in->lx, in->ly, c.lx_center, c.ly_center);
  float ra = get_angle(in->rx, in->ry, c.rx_center, c.ry_center);
  last_l_angle = la;
  last_r_angle = ra;

  // Calculate the distance of the stick
  // upcoming operations
  float ld = get_scaled_distance(la, in->lx, in->ly, c.lx_center, c.ly_center, l_distance_scalers);
  float rd = get_scaled_distance(ra, in->rx, in->ry, c.rx_center, c.ry_center, r_distance_scalers);

  // Process LEFT stick
  if (ld > 0)
  {
    float la_new = get_scaled_angle(la, l_angles, l_angle_scalers);

    // Generate normalized vector data
    // based on our new adjusted angles
    float nlx = 0;
    float nly = 0;
    normalized_vector(la_new, &nlx, &nly);

    // Finally, scale our values up accordingly
    nlx *= ld;
    nly *= ld;

    out->lx = CLAMP_0_MAX((int) roundf(nlx + STICK_INTERNAL_CENTER));
    out->ly = CLAMP_0_MAX((int) roundf(nly + STICK_INTERNAL_CENTER));
  }
  else
  {
    out->lx = STICK_INTERNAL_CENTER;
  }

  // Process RIGHT stick
  if (rd > 0)
  {

    float ra_new = get_scaled_angle(ra, r_angles, r_angle_scalers);

    // Generate normalized vector data
    // based on our new adjusted angles
    float nrx = 0;
    float nry = 0;
    normalized_vector(ra_new, &nrx, &nry);

    // Finally, scale our values up accordingly
    nrx *= rd;
    nry *= rd;

    out->rx = CLAMP_0_MAX((int) roundf(nrx + STICK_INTERNAL_CENTER));
    out->ry = CLAMP_0_MAX((int) roundf(nry + STICK_INTERNAL_CENTER));
  }
  else
  {
    out->rx = STICK_INTERNAL_CENTER;
  }
}

uint16_t _stick_distances_tracker = 0x00;

void stick_scaling_reset_distances()
{
  _stick_distances_tracker = 0x00;
  memset(l_angle_distances, 0, sizeof(float)*8);
  memset(r_angle_distances, 0, sizeof(float)*8);
}

// Captures stick distance for 8 angles on a loop
// Returns true when all 8 angles are accounted for
// on both sticks
bool stick_scaling_capture_distances(a_data_s *input)
{
  // Get angle for left stick
  float l_a = get_angle(input->lx, input->ly, c.lx_center, c.ly_center);
  float l_a_d = fmod(l_a, 45);
  // If we're close to a direct 45 degree angle
  if ((l_a_d < 1) || (l_a_d > 44))
  {
    // Get index
    int idx = get_distance_adjust_index(l_a);

    // Get distance for left stick
    float l_d = get_distance(input->lx, input->ly, c.lx_center, c.ly_center);

    if (l_d > l_angle_distances[idx])
    {
      l_angle_distances[idx] = l_d;
      _stick_distances_tracker |= (1<<idx);
    }
  }

  // Get angle for right stick
  float r_a = get_angle(input->rx, input->ry, c.rx_center, c.ry_center);
  float r_a_d = fmod(r_a, 45);
  // If we're close to a direct 45 degree angle
  if ((r_a_d < 1) || (r_a_d > 44))
  {
    // Get index
    int idx = get_distance_adjust_index(r_a);

    // Get distance for right stick
    float r_d = get_distance(input->rx, input->ry, c.rx_center, c.ry_center);

    if (r_d > r_angle_distances[idx])
    {
      r_angle_distances[idx] = r_d;
      _stick_distances_tracker |= (1<<(idx+8));
    }
  }

  return (_stick_distances_tracker == 0xFFFF);
}

// Captures the center point or sets it with input data
void stick_scaling_capture_center(a_data_s *input)
{
  c.lx_center = input->lx;
  c.ly_center = input->ly;

  c.rx_center = input->rx;
  c.ry_center = input->ry;
}

void stick_scaling_save_all()
{
  settings_set_centers(c.lx_center, c.ly_center, c.rx_center, c.ry_center);
  settings_set_distances(l_angle_distances, r_angle_distances);
  settings_set_angles(l_angles, r_angles);
  settings_save();
}

// Loads settings from memory (Settings need to be loaded first!)
void stick_scaling_init()
{
  memcpy(l_angles, &global_loaded_settings.l_angles, 4*sizeof(float));
  memcpy(r_angles, &global_loaded_settings.r_angles, 4*sizeof(float));

  memcpy(l_angle_distances, &global_loaded_settings.l_angle_distances, 8*sizeof(float));
  memcpy(r_angle_distances, &global_loaded_settings.r_angle_distances, 8*sizeof(float));

  c.lx_center = global_loaded_settings.lx_center;
  c.ly_center = global_loaded_settings.ly_center;
  c.rx_center = global_loaded_settings.rx_center;
  c.ry_center = global_loaded_settings.ry_center;

  calculate_diagonal_scalers(l_angles, l_angle_scalers);
  calculate_diagonal_scalers(r_angles, r_angle_scalers);
}

void stick_scaling_finalize()
{
  calculate_distance_scalers(l_angle_distances, l_distance_scalers);
  calculate_distance_scalers(r_angle_distances, r_distance_scalers);
}
