#include <stdio.h>
#include <string.h>
#include "ssd-1306-i2c.h"
#include "ssd-1306-i2c-defs.h"
#include "ata_monospace.h"
#include "driver/i2c.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#define I2C_PORT I2C_NUM_0
#define I2C_FREQ 100000
#define LCD_ADDR 0x3C

static const char* TAG = "SSD_1306_I2C";

static esp_err_t deinit_i2s() {
    return i2c_driver_delete(I2C_PORT);
}

static esp_err_t init_i2s() {
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
    i2c_master_write_byte(cmd, (LCD_ADDR<<1), 1);

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

static esp_err_t set_pixel_test() {
    uint8_t cmds[ ] = {
        CTRL_BYTE_STREAM_CMD,
        CMD_SET_COL_ADDRESS,
        0x00,
        0x7f,
        CMD_SET_PAGE_RANGE,
        0x00,
        0x07
    };
    esp_err_t ret = write_bytes(cmds, 7);
    if(ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set col address");
        return ret;
    }

    i2c_cmd_handle_t cmd;
    uint8_t buffer[1024] = {};

    cmd = init_i2c_cmd();
    i2c_master_write_byte(cmd, CTRL_BYTE_STREAM_DATA, 1);
    
    i2c_master_write(cmd, buffer, 128, 1);
    ret = send_i2c_cmd(cmd);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to send the data!");
    }

    ret = write_bytes(cmds, 7);
    if(ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set col address");
        return ret;
    }

    memcpy(buffer, ata_monospace[0], 5);
    buffer[5] = 0;
    memcpy(buffer+6, ata_monospace[1], 5);
    buffer[11] = 0;


    cmd = init_i2c_cmd();
    i2c_master_write_byte(cmd, CTRL_BYTE_STREAM_DATA, 1);
    i2c_master_write(cmd, buffer, 12, 1);
    ret = send_i2c_cmd(cmd);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to send the data!");
    }

    // cmd = init_i2c_cmd();
    // i2c_master_write_byte(cmd, CTRL_BYTE_SINGLE_CMD, 1);
    // i2c_master_write_byte(cmd, CMD_SET_DISPLAY_ON, 1);
    // send_i2c_cmd(cmd);

    return ret;
}

void hello_world() {
    ESP_ERROR_CHECK(init_i2s());

    ESP_ERROR_CHECK(initialize_lcd_settings());

    vTaskDelay(1000/portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(set_pixel_test());




    ESP_ERROR_CHECK_WITHOUT_ABORT(deinit_i2s());

    printf("Hello world!");
}