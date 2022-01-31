# micropython-camera-driver

This repository adds camera (OV2640) support to MicroPython for the ESP32 family.

I follow the advice of [#32](https://github.com/lemariva/micropython-camera-driver/issues/32) and modify the repository to fit to those requirements. However a file `main.c` needs a modification, and I got some problems compiling with the `berkeley-db` library (`DBTree`) support enable and I cannot switch it off using the `CMake` flags. Thus, some other files need modification (check below). If you are enable to solve those problems, send me a pull request and I will evaluate that and merge the files. Thanks! 

For more information about installing MicroPython visit this tutorial: https://lemariva.com/blog/2022/01/micropython-upgraded-support-cameras-m5camera-esp32-cam-etc

The MicroPython example codes are included here:
* [Webserver](https://github.com/lemariva/upyCam)
* [Timelapse Camera](https://github.com/lemariva/upyCam/tree/timelapse-camera)


## Example
```python
import camera

# ESP32-CAM (default configuration) - https://bit.ly/2Ndn8tN
camera.init(0, format=camera.JPEG)  

# M5Camera (Version B) - https://bit.ly/317Xb74
camera.init(0, d0=32, d1=35, d2=34, d3=5, d4=39, d5=18, d6=36, d7=19,
            format=camera.JPEG, framesize=camera.FRAME_VGA, xclk_freq=camera.XCLK_10MHz,
            href=26, vsync=25, reset=15, sioc=23, siod=22, xclk=27, pclk=21)   #M5CAMERA

# T-Camera Mini (green PCB) - https://bit.ly/31H1aaF
import axp202 # source https://github.com/lewisxhe/AXP202_PythonLibrary
# USB current limit must be disabled (otherwise init fails)
axp=axp202.PMU( scl=22, sda=21, address=axp202.AXP192_SLAVE_ADDRESS  )
limiting=axp.read_byte( axp202.AXP202_IPS_SET )
limiting &= 0xfc
axp.write_byte( axp202.AXP202_IPS_SET, limiting )

camera.init(0, d0=5, d1=14, d2=4, d3=15, d4=18, d5=23, d6=36, d7=39,
            format=camera.JPEG, framesize=camera.FRAME_VGA, 
            xclk_freq=camera.XCLK_20MHz,
            href=25, vsync=27, reset=-1, pwdn=-1,
            sioc=12, siod=13, xclk=32, pclk=19)

# The parameters: format=camera.JPEG, xclk_freq=camera.XCLK_10MHz are standard for all cameras.
# You can try using a faster xclk (20MHz), this also worked with the esp32-cam and m5camera
# but the image was pixelated and somehow green.

## Other settings:
# flip up side down
camera.flip(1)
# left / right
camera.mirror(1)

# framesize
camera.framesize(camera.FRAME_240x240)
# The options are the following:
# FRAME_96X96 FRAME_QQVGA FRAME_QCIF FRAME_HQVGA FRAME_240X240
# FRAME_QVGA FRAME_CIF FRAME_HVGA FRAME_VGA FRAME_SVGA
# FRAME_XGA FRAME_HD FRAME_SXGA FRAME_UXGA FRAME_FHD
# FRAME_P_HD FRAME_P_3MP FRAME_QXGA FRAME_QHD FRAME_WQXGA
# FRAME_P_FHD FRAME_QSXGA
# Check this link for more information: https://bit.ly/2YOzizz

# special effects
camera.speffect(camera.EFFECT_NONE)
# The options are the following:
# EFFECT_NONE (default) EFFECT_NEG EFFECT_BW EFFECT_RED EFFECT_GREEN EFFECT_BLUE EFFECT_RETRO

# white balance
camera.whitebalance(camera.WB_NONE)
# The options are the following:
# WB_NONE (default) WB_SUNNY WB_CLOUDY WB_OFFICE WB_HOME

# saturation
camera.saturation(0)
# -2,2 (default 0). -2 grayscale 

# brightness
camera.brightness(0)
# -2,2 (default 0). 2 brightness

# contrast
camera.contrast(0)
#-2,2 (default 0). 2 highcontrast

# quality
camera.quality(10)
# 10-63 lower number means higher quality

buf = camera.capture()

```

### Important
* The ESP32-CAM module has a 4MB PSRAM. However, this RAM is not used by the camera while running MicroPython. Inside the `main.c` file between lines 100-140 MicroPython takes over the entire SPIRAM region and uses it for the GC heap. Thus, it is not possible to use any RTOS functions to allocate memory there. If you want that the esp32-camera code allocates memory in the SPIRAM then the logic in `mp_task()` that allocates the MicroPython GC heap will need to be changed. I tried that, but without any success.
* Except when using CIF or lower resolution with JPEG, the driver requires PSRAM to be installed and activated. This is activated, but it is limited due that MicroPython needs RAM.
* Using YUV or RGB puts a lot of strain on the chip because writing to PSRAM is not particularly fast. The result is that image data might be missing. This is particularly true if WiFi is enabled. If you need RGB data, it is recommended that JPEG is captured and then turned into RGB using `fmt2rgb888 or fmt2bmp/frame2bmp`. The conversion is not supported. The formats are included, but I got almost every time out of memory, trying to capture an image in a different format than JPEG.
* The firmware was compiled without BLE support. Otherwise I got `region 'iram0_0_seg' overflowed by 468 bytes`.

## Firmware

I've included a compiled MicroPython firmware with camera (check the `firmware` folder). The firmware was compiled using esp-idf v4.3.2 ([8bf14a9](https://github.com/espressif/esp-idf/commit/8bf14a9238329954c7c5062eeeda569529aedf75))


To flash it to the board, you need to type the following:
```sh
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 micropython_cmake_60e05ae_esp32_idf4_4_camera.bin
```
More information is available in this [tutorial](https://lemariva.com/blog/2022/01/micropython-upgraded-support-cameras-m5camera-esp32-cam-etc).

If you want to compile your driver from scratch follow the next section:

## DIY

Read this section if you want to include the camera support to MicroPython from scratch. To do that follow these steps:
  
- Note 1: if you get stuck, those pages are a good help:
  - https://github.com/micropython/micropython/tree/master/ports/esp32
  - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation-step-by-step

- Note 2: Up to micropython version 1.14, the build tool for the esp32 port is Make. Starting with this [PR](https://github.com/micropython/micropython/pull/6892), it is CMAKE. You can find more discussion in this [micropython forum blog post](https://forum.micropython.org/viewtopic.php?f=18&t=9820)


1. Clone the MicroPython repository:
    ```
    git clone --recursive https://github.com/micropython/micropython.git
    ```
    Note: The MicroPython repo changes a lot, I've done this using the version with the hash [`60e05ae84`](https://github.com/micropython/micropython/commit/60e05ae84ec08c06e3f9d9051339811641479a94) (release=`v1.18-63-g60e05ae84-dirty`).

    :warning: If you want to directly replace the original files with the provided in this repository, be sure that you've taken the same commit hash. MicroPython changes a lot, and you'll compiling issues if you ignore this warning.

2. Make the following modification on following file(s):
    * If you have the following error by compiling: 
        ```
        micropython/lib/berkeley-db-1.xx/PORT/include/db.h:1:1: error: expected identifier or '(' before '.' token ../../include/db.h
        ```
        Modify the following files inside the folder `micropython/lib/berkeley-db-1.xx/PORT/include`:
        * `bsd-queue.h`: `queue.h > #include "queue.h"`
        * `db.h`: `../../include/db.h > #include "../../include/db.h"`
        * `filevtable.h`: `../../include/filevtable.h -> #include "../../include/filevtable.h"`
        * `mpool.h`: `../../include/mpool.h -> #include "../../include/mpool.h"`
  
3. Copy the files and folders inside the `boards` folder into `micropython/ports/esp32/boards`
4. Clone the `https://github.com/espressif/esp32-camera` repository inside the `~/esp/esp-idf/components` folder.
    ```sh
        cd ~/esp/esp-idf/components
        git clone https://github.com/espressif/esp32-camera
        # it was compiled with this commit 093688e0b3521ac982bc3d38bbf92059d97e3613
        git checkout 093688e0b3521ac982bc3d38bbf92059d97e3613
    ```
5. Compile the firmware by typing following commands:
    ```
    cd micropython/ports/esp32
    make USER_C_MODULES=../../../../micropython-camera-driver/src/micropython.cmake BOARD=GENERIC_CAM all
    ```
    Note that the folder `micropython-camera-driver` should be in the same folder level as the `micropython`. Otherwise, you'll need to change the path (`../../../../micropython-camera-driver/src/`) to the `micropython.cmake` file.
6. Deploy the firmware into the ESP32 by typing:
    ```
    cd micropython/ports/esp32
    esptool.py --port /dev/ttyUSB0 erase_flash
    esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash -z 0x1000 build-GENERIC_CAM/firmware.bin
    ```
