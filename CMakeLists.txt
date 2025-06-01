# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

set(FREERTOS_KERNEL_PATH "E:/Documentos/EMBARCATECH/fase2/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(conn-iot-mqtt C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Bibliotecas externas
add_subdirectory(lib/dht)
add_subdirectory(lib/ssd1306)

# Executável principal
add_executable(conn-iot-mqtt conn-iot-mqtt.c)

pico_set_program_name(conn-iot-mqtt "conn-iot-mqtt")
pico_set_program_version(conn-iot-mqtt "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(conn-iot-mqtt 0)
pico_enable_stdio_usb(conn-iot-mqtt 1)

# Includes adicionais
target_include_directories(conn-iot-mqtt PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/mqtt
)

# Add any user requested libraries
target_link_libraries(conn-iot-mqtt
    pico_stdlib
    hardware_gpio
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_mbedtls
    pico_lwip_mbedtls
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4
    dht
    ssd1306
)

pico_add_extra_outputs(conn-iot-mqtt)

