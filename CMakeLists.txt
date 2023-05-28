cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(PROGCC_RP2040 CXX C ASM)
# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include_directories(
	include include/devices include/usb
	include/analog
	)

add_executable(${PROJECT_NAME}
src/main.c

src/progcc_settings.c
src/progcc_defines.c
src/progcc_utils.c

src/devices/dinput.c
src/devices/gcinput.c
src/devices/nsinput.c
src/devices/xinput.c
src/devices/swpro.c

src/usb/progcc_usb.c
src/usb/xinput_device.c

src/analog/stick_scaling.c

src/rgb.c
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
	PICO_DEFAULT_UART_TX_PIN=12
	PICO_DEFAULT_UART_RX_PIN=13
	#PICO_DEFAULT_BOOT_STAGE2=boot2_generic_03h
	#PICO_FLASH_SPI_CLKDIV=8
	)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
#pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/joybus.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(${PROJECT_NAME}
pico_stdlib
pico_bootrom
pico_multicore
pico_platform

hardware_gpio
hardware_adc
hardware_flash
hardware_pio
hardware_dma
hardware_pwm
hardware_spi
hardware_sync
hardware_timer

tinyusb_device
tinyusb_board
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})
