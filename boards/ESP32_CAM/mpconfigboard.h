#define MICROPY_HW_BOARD_NAME "ESP32-cam module (i2s)"
#define MICROPY_HW_MCU_NAME "ESP32"


#define MICROPY_PY_BLUETOOTH                (0)
#define MODULE_CAMERA_ENABLED               (1)

// The offset only has an effect if a board has psram
// it allows the start of the range allocated to 
#define MICROPY_ALLOCATE_HEAP_USING_MALLOC (1)
#define MICROPY_HEAP_SIZE_REDUCTION (512 * 1024)