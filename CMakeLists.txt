cmake_minimum_required(VERSION 3.12)

set(NAME puzzle-detector)
set(PICO_SDK_FETCH_FROM_GIT ON)
include(FetchContent)
include(pico_sdk_import.cmake)

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${NAME}
        main.cpp
)
target_include_directories(${NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${NAME} pico_stdlib hardware_pwm pico_multicore)
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

