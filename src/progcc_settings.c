#include "progcc_settings.h"

const button_remap_s default_user_map = {
    .dpad_up = MAPCODE_DUP,
    .dpad_down = MAPCODE_DDOWN,
    .dpad_left = MAPCODE_DLEFT,
    .dpad_right = MAPCODE_DRIGHT,

    .button_a = MAPCODE_B_A,
    .button_b = MAPCODE_B_B,
    .button_x = MAPCODE_B_X,
    .button_y = MAPCODE_B_Y,

    .trigger_l = MAPCODE_T_ZL,
    .trigger_r = MAPCODE_T_ZR,
    .trigger_zl = MAPCODE_T_L,
    .trigger_zr = MAPCODE_T_R,

    .button_plus = MAPCODE_B_PLUS,
    .button_minus = MAPCODE_B_MINUS,
    .button_stick_left = MAPCODE_B_STICKL,
    .button_stick_right = MAPCODE_B_STICKR,
};

progcc_settings_s global_loaded_settings = {0};

// Internal functions for command processing
void _generate_mac()
{
  printf("Generated MAC: ");
  for(uint8_t i = 0; i < 6; i++)
  {
    global_loaded_settings.switch_mac_address[i] = get_rand_32() & 0xFF;
    printf("%X : ", global_loaded_settings.switch_mac_address[i]);
  }
  printf("\n");
}
#define FLASH_TARGET_OFFSET (1200 * 1024)

// Returns true if loaded ok
// returns false if no settings and reset to default
bool settings_load()
{
  static_assert(sizeof(progcc_settings_s) <= FLASH_SECTOR_SIZE);
      const uint8_t *target_read = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET + (FLASH_SECTOR_SIZE));
  memcpy(&global_loaded_settings, target_read, sizeof(progcc_settings_s));

  // Check that the version matches, otherwise reset to default and save.
  if(global_loaded_settings.settings_version != SETTINGS_VERSION)
  {
    printf("Settings version does not match. Resetting... \n");
    settings_reset_to_default();
    settings_save(false);
    return false;
  }

  return true;
}

void settings_reset_to_default()
{
  const progcc_settings_s set = {
    .settings_version = SETTINGS_VERSION,
    .comms_mode = COMM_MODE_USB,
    .usb_mode = PUSB_MODE_XI,
    .performance_mode = false,
    .remap_profile = default_user_map.val,

    .lx_center = 2048,
    .ly_center = 2048,
    .rx_center = 2048,
    .ry_center = 2048,
    .l_angles = {0,0,0,0,0,0,0,0},
    .r_angles = {0,0,0,0,0,0,0,0},
    .l_angle_distances = {
      600, 600, 600, 600, 600, 600, 600, 600,
    },
    .r_angle_distances = {
      600, 600, 600, 600, 600, 600, 600, 600,
    },
    .lx_snapback = 0,
    .ly_snapback = 0,
    .rx_snapback = 0,
    .ry_snapback = 0,

    .rgb_colors = {
      COLOR_RED.color,
      COLOR_ORANGE.color,

      COLOR_YELLOW.color,

      COLOR_GREEN.color,
      COLOR_BLUE.color,
      COLOR_CYAN.color,
      COLOR_PURPLE.color,

      COLOR_RED.color,
      COLOR_GREEN.color,
      COLOR_BLUE.color,
      COLOR_YELLOW.color,
      COLOR_BLUE.color,
    },
  };
  memcpy(&global_loaded_settings, &set, sizeof(progcc_settings_s));
  for(uint16_t i = 0; i < 26; i++)
  {
    global_loaded_settings.imu_calibration[i] = 0xFF;
  }
  memset(&global_loaded_settings.imu_offsets[0], 0, 12);
  _generate_mac();
}

volatile bool _save_flag = false;
volatile bool _webusb_indicate = false;

void settings_core1_save_check()
{
  if (_save_flag)
  {
    multicore_lockout_start_blocking();
    // Check that we are less than our flash sector size
    static_assert(sizeof(progcc_settings_s) <= FLASH_SECTOR_SIZE);

    // Store interrupts status and disable
    uint32_t ints = save_and_disable_interrupts();

    // Calculate storage bank address via index
    uint32_t memoryAddress = FLASH_TARGET_OFFSET + (FLASH_SECTOR_SIZE);

    // Create blank page data
    uint8_t page[FLASH_SECTOR_SIZE] = {0x00};
    // Copy settings into our page buffer
    memcpy(page, &global_loaded_settings, sizeof(progcc_settings_s));

    // Erase the settings flash sector
    flash_range_erase(memoryAddress, FLASH_SECTOR_SIZE);

    // Program the flash sector with our page
    flash_range_program(memoryAddress, page, FLASH_SECTOR_SIZE);

    // Restore interrups
    restore_interrupts(ints);
    multicore_lockout_end_blocking();

    // Indicate change
    if (_webusb_indicate)
    {
      webusb_save_confirm();
      _webusb_indicate = false;
    }
    _save_flag = false;
  }
}

void settings_save_webindicate()
{
  _webusb_indicate = true;
}

void settings_save()
{
  _save_flag = true;
}

void settings_set_centers(int lx, int ly, int rx, int ry)
{
  global_loaded_settings.lx_center = lx;
  global_loaded_settings.ly_center = ly;
  global_loaded_settings.rx_center = rx;
  global_loaded_settings.ry_center = ry;
}

void settings_set_distances(float *l_angle_distances, float *r_angle_distances)
{
  memcpy(global_loaded_settings.l_angle_distances, l_angle_distances, sizeof(float)*8);
  memcpy(global_loaded_settings.r_angle_distances, r_angle_distances, sizeof(float)*8);
}

void settings_set_angles(float *l_angles, float *r_angles)
{
  memcpy(global_loaded_settings.l_angles, l_angles, sizeof(float)*8);
  memcpy(global_loaded_settings.r_angles, r_angles, sizeof(float)*8);
}

void settings_set_mode(uint8_t comms_mode, uint8_t usb_mode)
{
  global_loaded_settings.comms_mode   = comms_mode;
  global_loaded_settings.usb_mode     = usb_mode;
}

void settings_set_snapback(uint8_t axis, uint8_t level)
{
  if (level > 7) level = 7;
  switch(axis)
  {
    default:
    case 0:
      global_loaded_settings.lx_snapback = level;
      break;
    case 1:
      global_loaded_settings.ly_snapback = level;
      break;
    case 2:
      global_loaded_settings.rx_snapback = level;
      break;
    case 3:
      global_loaded_settings.ry_snapback = level;
      break;
  }
}