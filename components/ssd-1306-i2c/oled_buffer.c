#include "oled_buffer.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

typedef struct {
    uint8_t values[OLED_BYTE_COUNT]; 
} oled_buffer_impl;

oled_buffer_handle init_oled_buffer() {
    oled_buffer_impl* bytes = malloc(sizeof(oled_buffer_impl));

    return (oled_buffer_handle)bytes;
}

uint8_t* oled_buffer_get_data(oled_buffer_handle bytes_handle) {
    if (!bytes_handle) {
        return NULL;
    }

    oled_buffer_impl* bytes = (oled_buffer_impl*)bytes_handle;
    
    return (void*)bytes->values;
}

esp_err_t deinit_oled_buffer(oled_buffer_handle bytes_handle) {
    if (!bytes_handle) {
        return ESP_FAIL;
    }

    oled_buffer_impl* bytes = (oled_buffer_impl*)bytes_handle;

    free(bytes);

    return ESP_OK;
}
