set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    boards/sdkconfig.spiram
    boards/ESP32_CAM/sdkconfig.esp32cam
)

list(APPEND MICROPY_DEF_BOARD
    MICROPY_HW_BOARD_NAME="ESP32 module with Camera"
)
