cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
##include(pico_extras_import_optional.cmake)

##project(myproject C CXX ASM)
project(myproject)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

##if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
##    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
##endif()

##set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures you don't lose any debug output while USB is set up
##if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
##    set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
##endif()

# Initialize the SDK
pico_sdk_init()

add_subdirectory(lib/LCD)

#include_directories(./lib/LCD)

add_subdirectory(src)

