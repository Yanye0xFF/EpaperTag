/*
 * utils.h
 * @brief
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include "stdint.h"
#include "co_utils.h"

typedef enum {
    OK = 0,
    FAIL,
    PENDING,
    BUSY,
    CANCEL,
    IDLE
} STATUS;

#define BIT_SET      1
#define BIT_CLEAR    0

// 价签量产完成
#define FLAG_ACTIVED           0
// 手机APP连接
#define FLAG_PHONE_CONNECTED   1

typedef struct _sys_info {
    uint16_t adv_interval;
    uint8_t ble_name[32];
    uint8_t rumtime_flag;
} sys_info_t;

void delay_ms(unsigned int timeout);

uint32_t calc_crc32(uint32_t crc, const void *buffer, size_t size);

unsigned char calc_crc8(unsigned char init, const unsigned char *data, unsigned int len);

void get_flash_id(uint8_t *buffer);

void set_runtime_default(void);

void set_adv_interval(uint16_t interval);

uint16_t get_adv_interval(void);

void set_ble_name(uint8_t *name);

uint8_t *get_ble_name(void);

uint8_t get_runtime_bit(uint8_t pos);

void set_runtime_bit(uint8_t pos, uint8_t val);

void mac_to_str(uint8_t *dest, const uint8_t *mac,  uint32_t offset);

void memswap(uint8_t *buffer, uint32_t size);

#endif /* EXAMPLES_BLE_EPD_SRC_UTILS_H_ */
