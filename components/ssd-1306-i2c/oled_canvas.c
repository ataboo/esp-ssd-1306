#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "oled_buffer.h"
// #define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "ssd-1306-i2c.h"

static const char* TAG = "OLED_CANVAS";

typedef struct {
    bool values[SCREEN_WIDTH][SCREEN_HEIGHT];
} canvas_grid_impl;


typedef struct {
    const char* name;
    int width;
    int height;
    uint8_t start_char;
    uint8_t end_char;
    uint32_t* data;
} canvas_font_impl;

static void plot_line_dx(canvas_grid_impl* canvas, canvas_point_t p1, canvas_point_t p2) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int y_step = 1;

    if (dy < 0) {
        y_step = -1;
        dy = -dy;
    }

    float err = (2*dy) - dx;
    int y = p1.y;

    for(int x=p1.x; x<=p2.x; x++) {
        canvas->values[x][y] = 1;
        if(err > 0) {
            y += y_step;
            err += 2*(dy - dx);
        } else {
            err += 2 * dy;
        }
    }
}

static void plot_line_dy(canvas_grid_impl* canvas, canvas_point_t p1, canvas_point_t p2) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int x_step = 1;

    if (dx < 0) {
        x_step = -1;
        dx = -dx;
    }

    float err = (2*dx) - dy;
    int x = p1.x;

    for(int y=p1.y; y<=p2.y; y++) {
        canvas->values[x][y] = 1;
        if(err > 0) {
            x += x_step;
            err += 2*(dx - dy);
        } else {
            err += 2 * dx;
        }
    }
}

esp_err_t canvas_draw_line(canvas_grid_handle canvas, canvas_point_t p1, canvas_point_t p2) {
    if (!canvas) {
        return ESP_FAIL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;

    if(abs(p2.y - p1.y) < abs(p2.x - p1.x)) {
        if (p1.x > p2.x) {
            plot_line_dx(canvas_impl, p2, p1);
        } else {
            plot_line_dx(canvas_impl, p1, p2);
        }
    } else {
        if (p1.y > p2.y) {
            plot_line_dy(canvas_impl, p2, p1);
        } else {
            plot_line_dy(canvas_impl, p1, p2);
        }
    }

    return ESP_OK;
}

esp_err_t dump_canvas(canvas_grid_handle canvas) {
    if (!canvas) {
        return ESP_FAIL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;
    char log_buffer[CANVAS_WIDTH+2];

    printf("Dumping Canvas Grid:\n");
    for(canvas_int_t y=0; y<CANVAS_HEIGHT; y++) {
        for(canvas_int_t x=0; x<CANVAS_WIDTH; x++) {
            log_buffer[x] = canvas_impl->values[x][y] ? 'x' : '_';
        }
        log_buffer[CANVAS_WIDTH] = '\n';
        log_buffer[CANVAS_WIDTH+1] = '\0';
        printf("%2d:", y);
        printf(log_buffer);
    }

    return ESP_OK;
}

esp_err_t clear_canvas_grid(canvas_grid_handle canvas) {
    if(canvas == NULL) {
        return ESP_FAIL;
    }

    printf("bool is this big: %d\n", sizeof(bool));

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;
    memset(canvas_impl->values, 0, sizeof(bool) * CANVAS_WIDTH * CANVAS_HEIGHT);

    return ESP_OK;
}

canvas_grid_handle init_canvas_grid() {
    canvas_grid_impl* canvas = (canvas_grid_impl*)malloc(sizeof(canvas_grid_impl));
    clear_canvas_grid((canvas_grid_handle)canvas);

    return (canvas_grid_handle)canvas;
}

esp_err_t canvas_draw_circle(canvas_grid_handle canvas, canvas_point_t point, float radius, bool filled) {
    if (!canvas) {
        return ESP_FAIL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;
    double minAngle = acos(1 - 1/radius);

    int cx, cy;

    for(double angle = 0; angle <= 180; angle += minAngle)
    {
        cx = round(radius * cos(angle));
        cy = round(radius * sin(angle));

        if (filled) {
            canvas_draw_line(canvas, (canvas_point_t){point.x + cx, point.y + cy}, (canvas_point_t){point.x - cx, point.y - cy});
        } else {
            canvas_impl->values[point.x + cx][point.y + cy] = 1;
            canvas_impl->values[point.x - cx][point.y - cy] = 1;
        }
    }

    return ESP_OK;
}


esp_err_t canvas_draw_rect(canvas_grid_handle canvas, canvas_point_t point1, canvas_point_t point2, bool filled) {
    if (!canvas) {
        return ESP_FAIL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;

    if (filled) {
        int minX = point1.x < point2.x ? point1.x : point2.x;
        int maxX = point1.x < point2.x ? point2.x : point1.x;
        int minY = point1.y < point2.y ? point1.y : point2.y;
        int maxY = point1.y < point2.y ? point2.y : point1.y;
        for(int x=minX; x<maxX; x++) {
            for(int y=minY; y<maxY; y++) {
                canvas_impl->values[x][y] = 1;
            }
        }
    } else {
        canvas_draw_line(canvas, point1, (canvas_point_t){point2.x, point1.y});
        canvas_draw_line(canvas, point1, (canvas_point_t){point1.x, point2.y});
        canvas_draw_line(canvas, point2, (canvas_point_t){point2.x, point1.y});
        canvas_draw_line(canvas, point2, (canvas_point_t){point1.x, point2.y});
    }

    return ESP_OK;
}

canvas_font_handle init_canvas_font(const char* name, int width, int height, uint8_t start_char, uint8_t end_char, uint32_t* data) {
    canvas_font_impl* font = malloc(sizeof(canvas_font_impl));

    font->name = name;
    font->width = width;
    font->height = height;
    font->start_char = start_char;
    font->end_char = end_char;
    font->data = data;

    return (canvas_font_handle)font;
}

esp_err_t canvas_draw_text(canvas_grid_handle canvas, const char* text, canvas_point_t point, canvas_font_handle font) {
    if(!canvas || !font) {
        return ESP_FAIL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;
    canvas_font_impl* font_impl = (canvas_font_impl*)font;
    int text_len = strlen(text);

    //todo validate sizes.

    for(int i=0; i<text_len; i++) {
        int char_idx = text[i] - font_impl->start_char;
        uint32_t* pattern = font_impl->data + char_idx * font_impl->height;

        for(int y=0; y<font_impl->height; y++) {
            for(int x=0; x<font_impl->width; x++) {
                if(pattern[y] & 1<<x) {
                    canvas_impl->values[x + point.x + i*font_impl->width][y + point.y] = true;
                }
            }
        }
    }

    return ESP_OK;
}

esp_err_t deinit_canvas(canvas_grid_handle canvas) {
    if (!canvas) {
        return ESP_FAIL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;
    free(canvas_impl);

    return ESP_OK;
}

bool* canvas_get_data(canvas_grid_handle canvas) {
    if(!canvas) {
        return NULL;
    }

    canvas_grid_impl* canvas_impl = (canvas_grid_impl*)canvas;

    return (bool*)canvas_impl->values;
}
