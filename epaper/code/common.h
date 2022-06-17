#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

// unit:ms
#define SLEEP_DELAY_AFTER_BOOT    5000

enum {
    EPAPER_IDX_SVC = 0,
    
    EPAPER_IDX_RX_CHAR,
    EPAPER_IDX_RX_VALUE,
    
    EPAPER_IDX_NTF_CHAR,
    EPAPER_IDX_NTF_VALUE,
    EPAPER_IDX_NTF_CFG,
    
    EPAPER_IDX_NB,
};

#define EPAPER_DEFAULT_PREFIX    "Epaper"

#define BLE_TX_POWER_LEVEL    0x10

#define BLE_TX_OFFSET    5

// 20ms
#define ADV_INTERVAL_MIN        32
// 10.24s
#define ADV_INTERVAL_MAX     16384

#define FLASH_TLV_MAJOR    0x28000
#define FLASH_TLV_MINOR    0x29000

#define TAG_BLE_MAC     0x0001
#define TAG_MAX_SIZE    6

#define TAG_ADV_INT     0x0002
#define TAG_ADV_SIZE    2

#define TAG_BLE_NAME    0x0003
// 可变长


#endif
