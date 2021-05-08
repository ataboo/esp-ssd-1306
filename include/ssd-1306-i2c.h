#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#define CANVAS_HEIGHT CONFIG_SSD_1306_SCREEN_HEIGHT
#define CANVAS_WIDTH CONFIG_SSD_1306_SCREEN_WIDTH

#define RETURN_IF_NOT_OK(action, message) ({ \
    esp_err_t ret = action;                  \
    if(ret != ESP_OK) {                      \
        ESP_LOGE(TAG, message);              \
        return ret;                          \
    }                                        \
})

typedef unsigned char canvas_int_t;

typedef struct canvas_grid_impl *canvas_grid_handle;

typedef struct {
    canvas_int_t x;
    canvas_int_t y;
} canvas_point_t;

typedef struct canvas_font_impl *canvas_font_handle;

canvas_grid_handle init_canvas_grid();

canvas_font_handle init_canvas_font(const char* name, int width, int height, uint8_t start_char, uint8_t end_char, uint32_t* data);

esp_err_t canvas_draw_line(canvas_grid_handle canvas, canvas_point_t p1, canvas_point_t p2);

esp_err_t clear_canvas_grid(canvas_grid_handle canvas);

esp_err_t dump_canvas(canvas_grid_handle canvas);

esp_err_t dump_canvas_buffer(canvas_grid_handle canvas);

esp_err_t canvas_draw_circle(canvas_grid_handle canvas, canvas_point_t point, float radius, bool filled);

esp_err_t canvas_draw_rect(canvas_grid_handle canvas, canvas_point_t point1, canvas_point_t point2, bool filled);

esp_err_t canvas_draw_text(canvas_grid_handle canvas, const char* text, canvas_point_t position, canvas_font_handle font);

esp_err_t canvas_draw_eq_tri(canvas_grid_handle canvas, canvas_point_t p1, int width, int height, bool inverted);

esp_err_t deinit_canvas(canvas_grid_handle canvas);

bool* canvas_get_data(canvas_grid_handle canvas);

esp_err_t init_lcd_i2c();

void set_lcd_i2c_addr(uint8_t addr);

esp_err_t draw_canvas_grid(canvas_grid_handle canvas_grid);

esp_err_t dispose_i2c_lcd();
