#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define IS_RGBW false
#define NUM_PIXELS 8

#define WS2812_PIN 15

// GPIO definitions

#define PGPIO_NS_SERIAL     7
#define PGPIO_NS_CLOCK      8
#define PGPIO_NS_LATCH      9

// Enable if we are using a keypad setup
#define PROGCC_USE_KEYPAD 1

// Whether or not we should use SPI based ADC for the analog sticks
// If this is zero, we are using Gulikit's UART solution.
#define PROGCC_USE_SPI_ADC 1

#define PROGCC_USE_ANALOG_TRIGGERS 0

#define PGPIO_BUTTON_RS   10
#define PGPIO_BUTTON_LS   11
#define PGPIO_BUTTON_MODE 14

#if PROGCC_USE_KEYPAD
  #define PGPIO_SCAN_A    23
  #define PGPIO_SCAN_B    20
  #define PGPIO_SCAN_C    17
  #define PGPIO_SCAN_D    19

  #define PGPIO_PUSH_A    16
  #define PGPIO_PUSH_B    18
  #define PGPIO_PUSH_C    21
  #define PGPIO_PUSH_D    22
#else
  #define PGPIO_BUTTON_A       17
  #define PGPIO_BUTTON_B       16
  #define PGPIO_BUTTON_X       18
  #define PGPIO_BUTTON_Y       19

  #define PGPIO_BUTTON_DUP     9
  #define PGPIO_BUTTON_DDOWN   10
  #define PGPIO_BUTTON_DLEFT   8
  #define PGPIO_BUTTON_DRIGHT  11

  #define PGPIO_BUTTON_L       22
  #define PGPIO_BUTTON_R       21
  #define PGPIO_BUTTON_Z       20
  #define PGPIO_BUTTON_START   5

  #define PGPIO_INPUT_ARRAY ((uint8_t[]) {PGPIO_BUTTON_A, PGPIO_BUTTON_B, PGPIO_BUTTON_X, PGPIO_BUTTON_Y, PGPIO_BUTTON_DUP, PGPIO_BUTTON_DDOWN, PGPIO_BUTTON_DLEFT, PGPIO_BUTTON_DRIGHT, PGPIO_BUTTON_L, PGPIO_BUTTON_R, PGPIO_BUTTON_Z, PGPIO_BUTTON_START})
#endif

// Analog L Trigger ADC
#define PADC_LT 1
// Analog R Trigger ADC
#define PADC_RT 0

// Analog L Trigger GPIO
#define PGPIO_LT 27
// Analog R Trigger GPIO
#define PGPIO_RT 26

#define PGPIO_RUMBLE_MAIN   25
#define PGPIO_RUMBLE_BRAKE  24

#if PROGCC_USE_SPI_ADC
  // SPI ADC CLK pin
  #define PGPIO_SPI_CLK 2
  // SPI ADC TX pin
  #define PGPIO_SPI_TX  3
  // SPI ADC RX pin
  #define PGPIO_SPI_RX  0

  // Left stick ADC Chip Select
  #define PGPIO_LS_CS   1
  // Right stick ADC Chip Select
  #define PGPIO_RS_CS   4
#endif

#endif
