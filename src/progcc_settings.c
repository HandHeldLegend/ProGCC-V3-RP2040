#include "progcc_settings.h"

progcc_settings_s global_loaded_settings = {0};

#define FLASH_TARGET_OFFSET (1536 * 1024)

void settings_load()
{
  static_assert(sizeof(progcc_settings_s) <= FLASH_SECTOR_SIZE);
      const uint8_t *target_read = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET + (FLASH_SECTOR_SIZE));
  memcpy(&global_loaded_settings, target_read, sizeof(progcc_settings_s));

  // Check that the version matches, otherwise reset to default and save.
  if(global_loaded_settings.settings_version != SETTINGS_VERSION)
  {
    printf("Settings version does not match. Resetting... \n");
    settings_reset_to_default();
    settings_save();
  }
}

void settings_reset_to_default()
{
  const progcc_settings_s set = {
    .settings_version = FIRMWARE_VERSION,
    .comms_mode = COMM_MODE_USB,
    .usb_mode = PUSB_MODE_XI,
    .performance_mode = false,

    .lx_center = 0,
    .ly_center = 0,
    .rx_center = 0,
    .ry_center = 0,
    .l_angles = {45.0f, 45.0f, 45.0f, 45.0f},
    .r_angles = {45.0f, 45.0f, 45.0f, 45.0f},
    .l_angle_distances = {
      572.0219f,
      553.6759f,
      603.0829f,
      685.2306f,
      704.0859f,
      808.2234f,
      677.0894f,
      678.8225f
    },
    .r_angle_distances = {
      637.0385f,
      620.8978f,
      598.0836f,
      620.1814f,
      608.0822f,
      666.1486f,
      580.0862f,
      683.8340f
    },
  };

  memcpy(&global_loaded_settings, &set, sizeof(progcc_settings_s));
}

void settings_save()
{
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
}
