cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(PROGCC_RP2040 CXX C ASM)
# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include_directories(
	include include/devices include/usb
	)

add_executable(${PROJECT_NAME}
src/main.c
src/progcc_settings.c
src/progcc_defines.c
src/devices/dinput.c
src/devices/gcinput.c
src/devices/nsinput.c
src/devices/xinput.c
src/usb/progcc_usb.c
src/usb/xinput_device.c
)

#pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/joybus.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(${PROJECT_NAME}
pico_stdlib
hardware_pio
hardware_dma
tinyusb_device
tinyusb_board
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
