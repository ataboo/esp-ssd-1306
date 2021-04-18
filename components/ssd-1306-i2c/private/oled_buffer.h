#pragma once

#include "esp_system.h"

#define SCREEN_WIDTH CONFIG_SSD_1306_SCREEN_WIDTH
#define SCREEN_HEIGHT CONFIG_SSD_1306_SCREEN_HEIGHT

#define OLED_BYTE_COUNT SCREEN_WIDTH * SCREEN_HEIGHT / 8

typedef struct oled_buffer_impl *oled_buffer_handle;

oled_buffer_handle init_oled_buffer();

uint8_t* oled_buffer_get_data(oled_buffer_handle bytes_handle);

esp_err_t deinit_oled_buffer(oled_buffer_handle bytes_handle);
