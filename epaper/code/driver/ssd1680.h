/*
 * ssd1680.h
 * @brief 250*122 1bit B/W/R
 * @note SPI write mode 20MHz Max
 */

#ifndef _SSD1680_H_
#define _SSD1680_H_

#include "gpio.h"
#include "system.h"

#include "stdbool.h"
#include "stdint.h"

//#define EPD128X296  

#define EPD_COMM_PORT     GPIO_PORT_C

// PC5 BUSY
#define EPD_BUSY_PORT     GPIO_PORT_C
#define EPD_BUSY_PIN      GPIO_BIT_5

// PC4 DC
#define EPD_DC_PORT       GPIO_PORT_C
#define EPD_DC_PIN        GPIO_BIT_4

// PA7 RESET
#define EPD_RESET_PORT    GPIO_PORT_A
#define EPD_RESET_PIN     GPIO_BIT_7

// 硬件spi(FR801x平台称为ssp)不太稳定的样子
#define EPD_SPI_SPEED     1000000
// 由于没有硬件SPI的更多资料，改用软件模拟SPI，这可以保证通信逻辑正确
#define USE_SOFT_SPI    0

#define EPD_WIDTH             122
#define EPD_HEIGHT            250
#define EPD_RAM_WIDTH         16
#define EPD_RAM_SIZE          8000
#define EPD_PART_RAM_SIZE     4000

// full refresh cost about 25s
#define EPD_REFRESH_TIMEOUT   1000

#define SW_RESET              0x12
#define DRIVER_OUTPUT_CTRL    0x01
#define DEEP_SLP_MODE         0x10
#define DATA_ENTRY_MODE       0x11
#define SET_RAM_X_ADDR        0x44
#define SET_RAM_Y_ADDR        0x45
#define BORDER_WAVEFORM_CTRL  0x3C
#define TEMP_SENSOR_CTRL      0x18
#define DISP_UPDATE_CTRL      0x21
#define SET_RAM_X_COUNTER     0x4E
#define SET_RAM_Y_COUNTER     0x4F
#define NOP                   0x7F

#define DEEP_SLEEP_CTRL       0x10
#define NORMAL_MODE           0x00
#define SLEEP_MODE1           0x01
#define SLEEP_MODE2           0x03

// For Black pixel: Content of Write RAM(BW) = 0 otherwise 1
#define WRITE_RAM_BW          0x24
// For Red pixel: Content of Write RAM(RED) = 1 otherwise 0
#define WRITE_RAM_RED         0x26

#define DISPLAY_UPDATE_CTRL2  0x22
#define MASTER_ACTIVATION     0x20

#define COLOR_MASK_RED        0x02
#define COLOR_MASK_BW         0x01
#define COLOR_FILL_WHITE      0x55

#define EPD_OK            0
#define EPD_OFFSET_ERR    1
#define EPD_LENGTH_ERR    2

enum {
    COLOR_BLACK = 0,
    COLOR_WHITE = 1,
    COLOR_RED = 3,
};

typedef struct _epd_cmd {
    uint8_t cmd;
    uint8_t len;
    const char *data;
} epd_cmd_t;

#define FLUSH_DONE       0
#define FLUSH_TIMEOUT    1

typedef void (* flush_done_callback_t)(uint8_t reason);

void epd_set_flush_done_callback(flush_done_callback_t cbk);

bool epd_is_idle(void);

uint8_t *epd_get_framebuffer(void);

void epd_init(void);

void epd_open(void);

void epd_clear(void);

uint8_t epd_write(uint32_t offset, uint8_t *data, uint32_t length);

bool epd_flush(void);

void epd_sleep(void);

void epd_close(void);

#endif
