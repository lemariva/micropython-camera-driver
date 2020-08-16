#include "esp_camera.h"
#include "esp_log.h"

#include <string.h>
#include "py/nlr.h"
#include "py/obj.h"
#include "modcamera.h"
#include "py/runtime.h"
#include "py/binary.h"

#include "esp_system.h"
#include "esp_spi_flash.h"

typedef struct _camera_obj_t {
    int8_t                 id;
    camera_config_t        config;
    bool                   used;
} camera_obj_t;

STATIC camera_obj_t camera_obj;


STATIC bool camera_init_helper(camera_obj_t *camera, size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
      enum {
        ARG_format,
        ARG_quality,
        ARG_d0,
        ARG_d1,
        ARG_d2,
        ARG_d3,
        ARG_d4,
        ARG_d5,
        ARG_d6,
        ARG_d7,
        ARG_VSYNC,
        ARG_HREF,
        ARG_PCLK,
        ARG_PWDN,
        ARG_RESET,
        ARG_XCLK,
        ARG_SIOD,
        ARG_SIOC,
        ARG_FREQ,
    };

    //{ MP_QSTR_d0,              MP_ARG_KW_ONLY                   | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_format,          MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = PIXFORMAT_JPEG} },
        { MP_QSTR_quality,         MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = 12} },
        { MP_QSTR_d0,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D0} },
        { MP_QSTR_d1,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D1} },
        { MP_QSTR_d2,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D2} },
        { MP_QSTR_d3,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D3} },
        { MP_QSTR_d4,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D4} },
        { MP_QSTR_d5,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D5} },
        { MP_QSTR_d6,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D6} },
        { MP_QSTR_d7,              MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_D7} },
        { MP_QSTR_vsync,           MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_VSYNC} },
        { MP_QSTR_href,            MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_HREF} },
        { MP_QSTR_pclk,            MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_PCLK} },
        { MP_QSTR_pwdn,            MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_PWDN} },
        { MP_QSTR_reset,           MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_RESET} },
        { MP_QSTR_xclk,            MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_XCLK} },
        { MP_QSTR_siod,            MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_SIOD} },
        { MP_QSTR_sioc,            MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = CAM_PIN_SIOC} },
        { MP_QSTR_xclk_freq,       MP_ARG_KW_ONLY                   | MP_ARG_INT,   {.u_int = XCLK_FREQ_10MHz} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_pos_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // TODO:---- Check validity of arguments ----
    int8_t format = args[ARG_format].u_int;
    if ((format != PIXFORMAT_JPEG) &&
        (format != PIXFORMAT_YUV422) &&
        (format != PIXFORMAT_GRAYSCALE) &&
        (format != PIXFORMAT_RGB565)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Image format is not valid"));
    }

    int32_t xclk_freq = args[ARG_FREQ].u_int;
    if ((xclk_freq != XCLK_FREQ_10MHz) &&
        (xclk_freq != XCLK_FREQ_20MHz)) {
        mp_raise_ValueError(MP_ERROR_TEXT("xclk frequency is not valid"));
    }

    // configuring camera
    camera->config.pin_d0 = args[ARG_d0].u_int;
    camera->config.pin_d1 = args[ARG_d1].u_int;
    camera->config.pin_d2 = args[ARG_d2].u_int;
    camera->config.pin_d3 = args[ARG_d3].u_int;
    camera->config.pin_d4 = args[ARG_d4].u_int;
    camera->config.pin_d5 = args[ARG_d5].u_int;
    camera->config.pin_d6 = args[ARG_d6].u_int;
    camera->config.pin_d7 = args[ARG_d7].u_int;
    camera->config.pin_vsync = args[ARG_VSYNC].u_int;
    camera->config.pin_href = args[ARG_HREF].u_int;
    camera->config.pin_pclk = args[ARG_PCLK].u_int;
    camera->config.pin_pwdn = args[ARG_PWDN].u_int;
    camera->config.pin_reset = args[ARG_RESET].u_int;
    camera->config.pin_xclk = args[ARG_XCLK].u_int;
    camera->config.pin_sscb_sda = args[ARG_SIOD].u_int;
    camera->config.pin_sscb_scl = args[ARG_SIOC].u_int;
    camera->config.pixel_format = args[ARG_format].u_int;   //YUV422,GRAYSCALE,RGB565,JPEG
    camera->config.jpeg_quality = args[ARG_quality].u_int;  //0-63 lower number means higher quality

    // defaul parameters
    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    camera->config.xclk_freq_hz = args[ARG_FREQ].u_int;
    camera->config.ledc_timer = LEDC_TIMER_0;
    camera->config.ledc_channel = LEDC_CHANNEL_0;
    camera->config.frame_size = FRAMESIZE_UXGA;//QQVGA-QXGA Do not use sizes above QVGA when not JPEG
    camera->config.fb_count = 1; //if more than one, i2s runs in continuous mode. Use only with JPEG

    esp_err_t err = esp_camera_init(&camera->config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Camera Init Failed"));

        return false;
    }

    return true;
}


STATIC mp_obj_t camera_init(mp_uint_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    bool camera = camera_init_helper(&camera_obj, n_pos_args - 1, pos_args + 1, kw_args);

    if (camera) {
        return mp_const_true;
    }
    else
    {
        return mp_const_false;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(camera_init_obj, 1, camera_init);


STATIC mp_obj_t camera_deinit(){
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera deinit Failed");
        return mp_const_false;
    }

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(camera_deinit_obj, camera_deinit);


STATIC mp_obj_t camera_capture(){
    //acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return mp_const_false;
    }

    mp_obj_t image = mp_obj_new_bytes(fb->buf, fb->len);

    //return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    return image;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(camera_capture_obj, camera_capture);

STATIC mp_obj_t camera_flip(mp_obj_t direction){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Flipping Failed");
        return mp_const_false;
      }
    int dir = mp_obj_get_int(direction);
    s->set_vflip(s, dir);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_flip_obj, camera_flip);

STATIC mp_obj_t camera_mirror(mp_obj_t direction){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Mirroring Failed");
        return mp_const_false;
      }
    int dir = mp_obj_get_int(direction);
    s->set_hmirror(s, dir);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_mirror_obj, camera_mirror);

STATIC mp_obj_t camera_framesize(mp_obj_t what){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Framesize Failed");
        return mp_const_false;
      }
    int size = mp_obj_get_int(what);
    /* same as in screen.h */
    if (size == 0) {
      s->set_framesize(s, FRAMESIZE_96X96); // 96x96
    } else if (size == 1) {
      s->set_framesize(s, FRAMESIZE_QQVGA); // 160x120
    } else if (size == 2) {
      s->set_framesize(s, FRAMESIZE_QCIF); // 176x144
    } else if (size == 3) {
      s->set_framesize(s, FRAMESIZE_HQVGA); // 240x176
    } else if (size == 4) {
      s->set_framesize(s, FRAMESIZE_240X240); // 240x240
    } else if (size == 5) {
      s->set_framesize(s, FRAMESIZE_QVGA); // 320x240
    } else if (size == 6) {
      s->set_framesize(s, FRAMESIZE_CIF); // 400x296
    } else if (size == 7) {
      s->set_framesize(s, FRAMESIZE_HVGA); // 480x320
    } else if (size == 8) {
      s->set_framesize(s, FRAMESIZE_VGA); // 640x480
    } else if (size == 9) {
      s->set_framesize(s, FRAMESIZE_SVGA); // 800x600
    } else if (size == 10) {
      s->set_framesize(s, FRAMESIZE_XGA); // 1024x768  (default)
    } else if (size == 11) {
      s->set_framesize(s, FRAMESIZE_HD); // 1280x720
    } else if (size == 12) {
      s->set_framesize(s, FRAMESIZE_SXGA); // 1280x1024
    } else if (size == 13) {
      s->set_framesize(s, FRAMESIZE_UXGA); // 1600x1200
    } else if (size == 14) {
      s->set_framesize(s, FRAMESIZE_FHD); // 1920x1080
    } else if (size == 15) {
      s->set_framesize(s, FRAMESIZE_P_HD); // 720x1280
    } else if (size == 16) {
      s->set_framesize(s, FRAMESIZE_P_3MP); // 864x1536
    } else if (size == 17) {
      s->set_framesize(s, FRAMESIZE_QXGA); // 2048x1536
    } else if (size == 18) {
      s->set_framesize(s, FRAMESIZE_QHD); // 2560x1440
    } else if (size == 19) {
      s->set_framesize(s, FRAMESIZE_WQXGA); // 2560x1600
    } else if (size == 20) {
      s->set_framesize(s, FRAMESIZE_P_FHD); // 1080x1920
    } else if (size == 21) {
      s->set_framesize(s, FRAMESIZE_QSXGA); // 2560x1920
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_framesize_obj, camera_framesize);

STATIC mp_obj_t camera_quality(mp_obj_t what){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Quality Failed");
        return mp_const_false;
      }
    int val = mp_obj_get_int(what); // 10-63 lower number means higher quality
    s->set_quality(s, val);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_quality_obj, camera_quality);

STATIC mp_obj_t camera_contrast(mp_obj_t what){
    //acquire a frame
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Contrast Failed");
        return mp_const_false;
      }
    int val = mp_obj_get_int(what); // -2,2 (default 0). 2 highcontrast
    s->set_contrast(s, val);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_contrast_obj, camera_contrast);

STATIC mp_obj_t camera_saturation(mp_obj_t what){
    //acquire a frame
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Saturation Failed");
        return mp_const_false;
      }
    int val = mp_obj_get_int(what);
    s->set_saturation(s, val); // -2,2 (default 0). -2 grayscale
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_saturation_obj, camera_saturation);

STATIC mp_obj_t camera_brightness(mp_obj_t what){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Brightness Failed");
        return mp_const_false;
      }
    int val = mp_obj_get_int(what);
    s->set_brightness(s, val); // -2,2 (default 0). 2 brightest
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_brightness_obj, camera_brightness);

STATIC mp_obj_t camera_speffect(mp_obj_t what){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Special Effect Failed");
        return mp_const_false;
      }
    int val = mp_obj_get_int(what);
    s->set_special_effect(s, val); // 0-6 (default 0).
                                   // 0 - no effect
				   // 1 - negative
				   // 2 - black and white
				   // 3 - reddish
				   // 4 - greenish
				   // 5 - blue
				   // 6 - retro
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_speffect_obj, camera_speffect);

STATIC mp_obj_t camera_whitebalance(mp_obj_t what){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "White Balance Failed");
        return mp_const_false;
      }
    int val = mp_obj_get_int(what);
    s->set_wb_mode(s, val); // 0-4 (default 0).
                                   // 0 - no effect
                                   // 1 - sunny
                                   // 2 - cloudy
                                   // 3 - office
                                   // 4 - home
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_whitebalance_obj, camera_whitebalance);

STATIC const mp_rom_map_elem_t camera_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_camera) },

    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&camera_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&camera_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_capture), MP_ROM_PTR(&camera_capture_obj) },
    { MP_ROM_QSTR(MP_QSTR_flip), MP_ROM_PTR(&camera_flip_obj) },
    { MP_ROM_QSTR(MP_QSTR_mirror), MP_ROM_PTR(&camera_mirror_obj) },
    { MP_ROM_QSTR(MP_QSTR_framesize), MP_ROM_PTR(&camera_framesize_obj) },
    { MP_ROM_QSTR(MP_QSTR_quality), MP_ROM_PTR(&camera_quality_obj) },
    { MP_ROM_QSTR(MP_QSTR_contrast), MP_ROM_PTR(&camera_contrast_obj) },
    { MP_ROM_QSTR(MP_QSTR_saturation), MP_ROM_PTR(&camera_saturation_obj) },
    { MP_ROM_QSTR(MP_QSTR_brightness), MP_ROM_PTR(&camera_brightness_obj) },
    { MP_ROM_QSTR(MP_QSTR_speffect), MP_ROM_PTR(&camera_speffect_obj) },
    { MP_ROM_QSTR(MP_QSTR_whitebalance), MP_ROM_PTR(&camera_whitebalance_obj) },

    // Constants
    { MP_ROM_QSTR(MP_QSTR_JPEG),            MP_ROM_INT(PIXFORMAT_JPEG) },
    { MP_ROM_QSTR(MP_QSTR_YUV422),          MP_ROM_INT(PIXFORMAT_YUV422) },
    { MP_ROM_QSTR(MP_QSTR_GRAYSCALE),       MP_ROM_INT(PIXFORMAT_GRAYSCALE) },
    { MP_ROM_QSTR(MP_QSTR_RGB565),          MP_ROM_INT(PIXFORMAT_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_XCLK_10MHz),      MP_ROM_INT(XCLK_FREQ_10MHz) },
    { MP_ROM_QSTR(MP_QSTR_XCLK_20MHz),      MP_ROM_INT(XCLK_FREQ_20MHz) },

};

STATIC MP_DEFINE_CONST_DICT(camera_module_globals, camera_module_globals_table);

const mp_obj_module_t mp_module_camera = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&camera_module_globals,
};
