#include <stdio.h>
#include <string.h>
#include "ssd-1306-i2c.h"
#include "ssd-1306-i2c-defs.h"
#include "driver/i2c.h"
// #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "oled_buffer.h"

#define I2C_PORT CONFIG_SSD_1306_I2C_PORT
#define I2C_FREQ CONFIG_SSD_1306_I2C_FREQ

static const char* TAG = "SSD_1306_I2C";

static uint8_t i2c_addr;
static oled_buffer_handle oled_buffer;

static esp_err_t init_i2c() {
    esp_err_t ret = i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
    if(ret != ESP_OK) {
        return ret;
    }

    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_14,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = GPIO_NUM_15,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ,
    };

    return i2c_param_config(I2C_PORT, &cfg);
}

static i2c_cmd_handle_t init_i2c_cmd() {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr<<1), 1);

    return cmd;
}

static esp_err_t send_i2c_cmd(i2c_cmd_handle_t cmd) {
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 50/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if(ret == ESP_OK) {
        ESP_LOGI(TAG, "Success!");
    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "i2c timed out");
    } else {
        ESP_LOGW(TAG, "Write Failed!");
    }

    return ret;
}

static esp_err_t write_bytes(uint8_t* data, int len) {
    i2c_cmd_handle_t cmd = init_i2c_cmd();

    for(int i=0; i<len; i++) {
        i2c_master_write(cmd, data, len, 1);
    }

    return send_i2c_cmd(cmd);
}

static esp_err_t initialize_lcd_settings() {
    uint8_t cmds[ ] = {
        CTRL_BYTE_STREAM_CMD,

        CMD_SET_DISPLAY_OFF, 
        
        CMD_SET_MUX_RATIO,
        VAL_RESET_MUX_RATIO,
        
        CMD_SET_DISP_OFFSET,
        VAL_DISP_OFFSET_RESET,

        CMD_DISPLAY_START_LINE,

        CMD_SEGMENT_REMAP,

        CMD_COM_SCAN_MODE,
        
        CMD_COM_PIN_MAP,
        VAL_COM_PIN_ALTERNATE,

        CMD_SET_CONTRAST,
        VAL_RESET_CONTRAST,

        CMD_ENTIRE_DISP_OFF,

        CMD_SET_INVERSE_OFF,

        CMD_DISPLAY_CLK_DIV,
        VAL_CLK_DIV_DEFAULT,

        CMD_CHARGE_PUMP,
        VAL_CHARGE_PUMP_DEFAULT,

        CMD_SET_PRECHARGE,
        VAL_PRECHARGE_DEFAULT,

        CMD_SET_VCOMH_DESELECT,
        VAL_VCOMH_DEFAULT,
        
        CMD_SET_ADDRESSING_MODE, 
        VAL_ADDRESSING_HORIZONTAL,
        
        CMD_SET_DISPLAY_ON,
    }; 

    return write_bytes(cmds, 26);
}

static esp_err_t canvas_to_oled_buffer(canvas_grid_handle grid_handle, oled_buffer_handle buffer_handle) {
    bool* canvas_data = canvas_get_data(grid_handle);
    uint8_t* buffer_data = oled_buffer_get_data(buffer_handle);
    
    uint8_t byte_val;
    for(int x=0; x<CANVAS_WIDTH; x++) {
        for(int by=0; by<CANVAS_HEIGHT / 8; by++) {
            byte_val = 0;
            for(int y=0; y<8; y++) {
                if (canvas_data[x * CANVAS_HEIGHT + by*8 + y]) {
                    byte_val |= 1<<y;
                }
            }

            buffer_data[CANVAS_WIDTH * by + x] = byte_val;
        }
    }

    return ESP_OK;
}

static esp_err_t oled_buffer_to_canvas(oled_buffer_handle buffer_handle, canvas_grid_handle grid_handle) {
    bool* canvas_data = canvas_get_data(grid_handle);
    uint8_t* buffer_data = oled_buffer_get_data(buffer_handle);

    uint8_t byte_val;
    for(int x=0; x<CANVAS_WIDTH; x++) {
        for(int by=0; by<CANVAS_HEIGHT / 8; by++) {
            byte_val = buffer_data[CANVAS_WIDTH * by + x];

            for(int y=0; y<8; y++) {
                canvas_data[x * CANVAS_WIDTH + by*8 + y] = 1<<y & byte_val;
            }
        }
    }

    return ESP_OK;
}

esp_err_t draw_canvas_grid(canvas_grid_handle canvas) {
    if (oled_buffer == NULL) {
        oled_buffer = init_oled_buffer();
    }

    RETURN_IF_NOT_OK(canvas_to_oled_buffer(canvas, oled_buffer), "canvas to oled bytes");

    uint8_t cmds[ ] = {
        CTRL_BYTE_STREAM_CMD,
        CMD_SET_COL_ADDRESS,
        0x00,
        0x7f,
        CMD_SET_PAGE_RANGE,
        0x00,
        0x07
    };

    ESP_LOGI(TAG, "[2.1] set position");
    int cmds_len = 7;
    RETURN_IF_NOT_OK(write_bytes(cmds, cmds_len), "write command stream");

    ESP_LOGI(TAG, "[2.2] write data");
    i2c_cmd_handle_t cmd = init_i2c_cmd();
    i2c_master_write_byte(cmd, CTRL_BYTE_STREAM_DATA, 1);
    uint8_t* data = (uint8_t*)oled_buffer_get_data(oled_buffer);
    i2c_master_write(cmd, data, OLED_BYTE_COUNT, 1);
    RETURN_IF_NOT_OK(send_i2c_cmd(cmd), "send data stream");

    return ESP_OK;
}

esp_err_t dump_canvas_buffer(canvas_grid_handle canvas) {
    if (oled_buffer == NULL) {
        oled_buffer = init_oled_buffer();
    }

    uint8_t* data = oled_buffer_get_data(oled_buffer);

    RETURN_IF_NOT_OK(canvas_to_oled_buffer(canvas, oled_buffer), "canvas to oled bytes");

    printf("Dumping canvas buffer:\n");

    for(int y=0; y<CANVAS_HEIGHT; y++) {
        printf("%d: ", y);
        for(int x=0; x<CANVAS_WIDTH; x++) {
            int yByte = y/8;
            int yBit = y%8;
            uint8_t intVal = data[yByte * CANVAS_WIDTH + x];
            
            if (intVal & 1<<yBit) {
                printf("X");
            } else {
                printf("_");
            }
        }
        printf("\n");
    }


    return ESP_OK;
}

void set_lcd_i2c_addr(uint8_t addr) {
    i2c_addr = addr;
}

esp_err_t init_lcd_i2c() {
    i2c_addr = CONFIG_SSD_1306_I2C_ADDR;

    ESP_LOGI(TAG, "[1.1] init i2s");
    RETURN_IF_NOT_OK(init_i2c(), "init i2c");

    ESP_LOGI(TAG, "[1.2] initialize lcd settings");
    RETURN_IF_NOT_OK(initialize_lcd_settings(), "init lcd settings");

    return ESP_OK;
}

esp_err_t deinit_lcd_i2s() {
    deinit_oled_buffer(oled_buffer);

    return i2c_driver_delete(I2C_PORT);
}
