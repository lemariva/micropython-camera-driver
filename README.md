# micropython-camera-driver

This repository adds camera (OV2640) support to MicroPython for the ESP32 family. 

I could have forked the micropython repository and include the camera driver. However, I chose to include in this repository only the files that are required, so that you can always use the last version of MicroPython and add these files to add camera support.

Furthermore, I include a compiled version of the firmware, so that you can flash it and start playing with the camera.

## Example
```python
import camera

# ESP32-CAM (default configuration) - https://bit.ly/2Ndn8tN
camera.init(0, format=camera.JPEG)  

# M5Camera (Version B) - https://bit.ly/317Xb74
camera.init(0, format=camera.JPEG, d0=32, d1=35, d2=34, d3=5, d4=39, d5=18, d6=36, d7=19, 
            href=26, vsync=25, reset=15, sioc=23, siod=22, xclk=27, pclk=21)   #M5CAMERA

buf = camera.capture()
```
### Important
* Except when using CIF or lower resolution with JPEG, the driver requires PSRAM to be installed and activated. This is activated, but it is limited due that MicroPython needs RAM.
* Using YUV or RGB puts a lot of strain on the chip because writing to PSRAM is not particularly fast. The result is that image data might be missing. This is particularly true if WiFi is enabled. If you need RGB data, it is recommended that JPEG is captured and then turned into RGB using `fmt2rgb888 or fmt2bmp/frame2bmp`. The conversion is not supported. The formats are included, but I got almost every time out of memory, trying to capture an image in a different format than JPEG.

## DIY
To include the camera support to MicroPython, you need to compile the firmware from scratch. To do that follow these steps:
1. Clone the MicroPython repository:
    ```
    git clone --recursive https://github.com/micropython/micropython.git
    ```
    Note: The MicroPython repo changes a lot, I've done this using the version with the hash 3a9d948032e27f690e1fb09084c36bd47b1a75a0.
2. Copy the files of this repository inside the folder `ports/esp32`. If you don't want to replace the files `mpconfigport.h`, `main.h` and `Makefile`, make the following modifications to the originals:
    * `mpconfigport.c`
        1. add the line
        ```
           extern const struct _mp_obj_module_t mp_module_camera;
        ```
        in the code block below `// extra built in modules to add to the list of known ones` (approx. line 184)

        2. add the line:
        ```
            { MP_OBJ_NEW_QSTR(MP_QSTR_camera), (mp_obj_t)&mp_module_camera }, \
        ```
        in the code block below `#define MICROPY_PORT_BUILTIN_MODULES \` (approx. line 194/195)

    * `main.c`: modify the lines inside the `#if CONFIG_ESP32_SPIRAM_SUPPORT || CONFIG_SPIRAM_SUPPORT`, they should look like:
        ```
            mp_task_heap_size = 2 * 1024 * 1024;
            void *mp_task_heap = malloc(mp_task_heap_size);
            ESP_LOGI("main", "Allocated %dK for micropython heap at %p", mp_task_heap_size/1024, mp_task_heap);
        ```

    * `Makefile`:
        1. add the lines:
        ```
            INC_ESPCOMP += -I$(ESPCOMP)/esp32-camera/driver/include
            INC_ESPCOMP += -I$(ESPCOMP)/esp32-camera/driver/private_include
            INC_ESPCOMP += -I$(ESPCOMP)/esp32-camera/conversions/include
            INC_ESPCOMP += -I$(ESPCOMP)/esp32-camera/conversions/private_include
            INC_ESPCOMP += -I$(ESPCOMP)/esp32-camera/sensors/private_include
        ```
        inside under the `# Compiler and linker flags`  (approx. line 135)

        2. add the line:
        ```
            modcamera.c \
        ```
        inside the `SRC_C = \` block (approx. line 329 - after the above insertion)

        3. add the lines:
        ```
            ESP32_CAM_O = $(patsubst %.c,%.o,\
            $(wildcard $(ESPCOMP)/esp32-camera/driver/*.c) \
            $(wildcard $(ESPCOMP)/esp32-camera/sensors/*.c) \
            $(wildcard $(ESPCOMP)/esp32-camera/conversions/*.c) \
            )
        ```
        above the line `OBJ_ESPIDF =` (approx. line 615 - after the above insertions)

        4. add the line:
        ```
            $(eval $(call gen_espidf_lib_rule,esp32_cam,$(ESP32_CAM_O)))
        ```
        in the block above `ifeq ($(ESPIDF_CURHASH),$(ESPIDF_SUPHASH_V4))` (approx. line 655 - after the above insertions)

3. Clone the `https://github.com/lemariva/esp32-camera` repository inside the `~/esp/esp-idf/components` folder.
    ```sh
        cd ~/esp/esp-idf/components
        git clone https://github.com/lemariva/esp32-camera.git
    ```

4. Compile and deploy MicroPython following the instructions from this [tutorial](https://lemariva.com/blog/2020/03/tutorial-getting-started-micropython-v20). But, use the following compiling options:
    ```sh
    make BOARD=GENERIC_CAM PYTHON=python3 MICROPY_PY_BTREE=0 -j
    make BOARD=GENERIC_CAM PYTHON=python3 MICROPY_PY_BTREE=0 -j deploy
    ```

## Firmware
I've included a compiled MicroPython firmware with camera and BLE support (check the `firmware` folder). The firmware was compiled using esp-idf 4.x (hash 4c81978a3).

To deploy it, you need to type the following:
```sh
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 micropython_3a9d948_esp32_idf4.x_ble_camera.bin
```
More information is available in this [tutorial](https://lemariva.com/blog/2020/03/tutorial-getting-started-micropython-v20).
