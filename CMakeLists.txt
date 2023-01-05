# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Application version
set(FIRMWARE_VERSION_MAJOR 0)
set(FIRMWARE_VERSION_MINOR 0)
set(FIRMWARE_VERSION_PATCH 1)
set(FIRMWARE_NAME "cappa-oleari")

add_definitions(-DAPP_CONFIG_FIRMWARE_VERSION_MAJOR=${FIRMWARE_VERSION_MAJOR}u)
add_definitions(-DAPP_CONFIG_FIRMWARE_VERSION_MINOR=${FIRMWARE_VERSION_MINOR}u)
add_definitions(-DAPP_CONFIG_FIRMWARE_VERSION_PATCH=${FIRMWARE_VERSION_PATCH}u)
add_definitions(-DAPP_CONFIG_FIRMWARE_NAME="${FIRMWARE_NAME}")

add_definitions(-DLV_HOR_RES_MAX=480)
add_definitions(-DLV_VER_RES_MAX=320)
add_definitions(-DSPI_HOST_MAX=SPI3_HOST)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${FIRMWARE_NAME})
target_add_binary_data(cappa-oleari.elf "webapp/index.html.gzip" TEXT)