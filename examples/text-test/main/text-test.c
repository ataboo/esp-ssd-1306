#include "ssd-1306-i2c.h"
#include "esp_log.h"
#include "ata_mono16.h"
#include "ata_mono12.h"

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_DEBUG);

    canvas_grid_handle canvas_grid = init_canvas_grid();
    canvas_font_handle mono_16 = CREATE_ATA_MONO16();
    canvas_font_handle mono_12 = CREATE_ATA_MONO12();

    canvas_draw_line(canvas_grid, (canvas_point_t){6, 60}, (canvas_point_t){24, 32});
    canvas_draw_line(canvas_grid, (canvas_point_t){24, 32}, (canvas_point_t){42, 60});
    canvas_draw_line(canvas_grid, (canvas_point_t){6, 60}, (canvas_point_t){42, 60});

    canvas_draw_circle(canvas_grid, (canvas_point_t){64, 46}, 14, false);

    canvas_draw_rect(canvas_grid, (canvas_point_t){92, 32}, (canvas_point_t){122, 60}, true);

    canvas_draw_text(canvas_grid, "Hello World!", (canvas_point_t){8, 0}, mono_16);
    canvas_draw_text(canvas_grid, "Hello World!", (canvas_point_t){8, 16}, mono_12);

    dump_canvas(canvas_grid);
    dump_canvas_buffer(canvas_grid);

    ESP_ERROR_CHECK(init_lcd_i2c());
    set_lcd_i2c_addr(0x3c);
    ESP_ERROR_CHECK(draw_canvas_grid(canvas_grid));
    ESP_ERROR_CHECK(deinit_canvas(canvas_grid));
}
