cmake_minimum_required(VERSION 3.28.0)

include(external/pico-sdk/pico_sdk_init.cmake)

project(ENTER)

pico_sdk_init()

add_executable(ENTER
    "src/main.cpp"
    "src/usb_descriptors.cpp"
)

target_include_directories(ENTER
    PRIVATE
        "include"
)

target_link_libraries(ENTER
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(ENTER 1)
pico_enable_stdio_uart(ENTER 0)
