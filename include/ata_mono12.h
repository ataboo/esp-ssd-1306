#pragma once

#include <stdint.h>

// Generated by fontexport.py plugin for Gimp

// Each character is represented by 12 rows of integers representing the bit mask for each rows.
uint32_t ata_mono12_data[95][12];

#define CREATE_ATA_MONO12() {  \
    init_canvas_font("ata_mono12", 6, 12, ' ', '~', (uint32_t*)ata_mono12_data)  \
}