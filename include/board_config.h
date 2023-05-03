#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// GPIO definitions

#define PGPIO_NS_SERIAL     0
#define PGPIO_NS_CLOCK      0
#define PGPIO_NS_LATCH      0

#define PROGCC_USE_KEYPAD 0

#if PROGCC_USE_KEYPAD
  #define PGPIO_SCAN_A    0
  #define PGPIO_SCAN_B    1
  #define PGPIO_SCAN_C    2
  #define PGPIO_SCAN_D    3

  #define PGPIO_PUSH_A    0
  #define PGPIO_PUSH_B    0
  #define PGPIO_PUSH_C    0
  #define PGPIO_PUSH_D    0
#else
  #define GPIO_BUTTON_A       0
  #define GPIO_BUTTON_B       0
  #define GPIO_BUTTON_X       0
  #define GPIO_BUTTON_Y       0

  #define GPIO_BUTTON_DUP     0
  #define GPIO_BUTTON_DDOWN   0
  #define GPIO_BUTTON_DLEFT   0
  #define GPIO_BUTTON_DRIGHT  0

  #define GPIO_BUTTON_L       0
  #define GPIO_BUTTON_R       0
  #define GPIO_BUTTON_Z       0
  #define GPIO_BUTTON_START   0
#endif

#endif
