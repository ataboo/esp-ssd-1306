#pragma once

// Followed by value 0-0xFF
#define CMD_SET_CONTRAST          0x81
#define VAL_RESET_CONTRAST        0x7F

// Update display from RAM values.
#define CMD_ENTIRE_DISP_OFF       0xA4
// Turn all pixels on and ignore ram values
#define CMD_ENTIRE_DISP_ON        0xA5	

#define CMD_SET_INVERSE_OFF       0xA6
#define CMD_SET_INVERSE_ON        0xA7

#define CMD_SET_MUX_RATIO         0xA8
#define VAL_RESET_MUX_RATIO       0x3F

#define CMD_SET_DISPLAY_OFF       0xAE
#define CMD_SET_DISPLAY_ON        0xAF

#define CMD_SET_ADDRESSING_MODE   0x20
#define VAL_ADDRESSING_HORIZONTAL 0x00
#define VAL_ADDRESSING_VERTICAL   0x01
#define VAL_ADDRESSING_PAGE       0x02

#define CMD_DISPLAY_START_LINE	  0x40
#define CMD_SEGMENT_REMAP         0xA1

#define CMD_COM_PIN_MAP           0xDA
#define VAL_COM_PIN_ALTERNATE     0x12

#define CMD_COM_SCAN_MODE         0xC8

// Followed by start byte then end byte
#define CMD_SET_COL_ADDRESS       0x21

#define CMD_SET_PAGE_RANGE        0x22

// Followed by byte 0-63
#define CMD_SET_DISP_OFFSET       0xD3
#define VAL_DISP_OFFSET_RESET     0x00

#define CMD_DISPLAY_CLK_DIV       0xD5
#define VAL_CLK_DIV_DEFAULT       0x80

#define CMD_CHARGE_PUMP           0x8D
#define VAL_CHARGE_PUMP_DEFAULT   0x14

#define CMD_SET_PRECHARGE         0xD9
#define VAL_PRECHARGE_DEFAULT     0x22

#define CMD_SET_VCOMH_DESELECT    0xD8
#define VAL_VCOMH_DEFAULT         0x30

// 0b00000000
#define CTRL_BYTE_STREAM_CMD      0x00
// 0b01000000
#define CTRL_BYTE_STREAM_DATA     0x40
// 0b10000000
#define CTRL_BYTE_SINGLE_CMD      0x80
// 0b11000000
#define CRTL_BYTE_SINGLE_DATA     0xC0

