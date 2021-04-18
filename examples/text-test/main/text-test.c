#include "ssd-1306-i2c.h"
#include "esp_log.h"

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_DEBUG);

    canvas_grid_handle canvas_grid = init_canvas_grid();

    canvas_draw_line(canvas_grid, (canvas_point_t){6, 60}, (canvas_point_t){24, 32});
    canvas_draw_line(canvas_grid, (canvas_point_t){24, 32}, (canvas_point_t){42, 60});
    canvas_draw_line(canvas_grid, (canvas_point_t){6, 60}, (canvas_point_t){42, 60});

    canvas_draw_circle(canvas_grid, (canvas_point_t){64, 46}, 14, false);

    canvas_draw_rect(canvas_grid, (canvas_point_t){92, 32}, (canvas_point_t){122, 60}, true);


    dump_canvas(canvas_grid);

    // draw_canvas_grid(canvas_grid);

    // ESP_ERROR_CHECK(init_lcd_i2c());
    // set_lcd_i2c_addr(0x3c);
    // ESP_ERROR_CHECK(draw_canvas_grid(canvas_grid));
    // ESP_ERROR_CHECK(deinit_canvas(canvas_grid));
}
