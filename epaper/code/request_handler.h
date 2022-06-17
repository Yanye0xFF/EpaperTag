#ifndef _REQUESET_HANDLER_H_
#define _REQUESET_HANDLER_H_

#include <stdint.h>

#define RESULT_OK              0x00

#define ACT_WR_RAM    0x00
#define WR_RAM_OFFSET_ERR    0x01
#define WR_RAM_LENGTH_ERR    0x02
#define WR_RAM_PACKET_ERR    0x03
#define WR_RAM_EPD_BUSY      0x04

#define ACT_FLUSH_SCREEN    0x01
#define EPD_BUSY_ERR           0x01
#define EPD_FLUSH_DONE         0x02
#define EPD_FLUSH_TASK_ERR     0x03

#define ACT_RD_RAM    0x02
#define RD_RAM_OFFSET_ERR    0x01
#define RD_RAM_LENGTH_ERR    0x02

#define ACT_RD_HW_INFO  0x03
#define HW_INFO_FLASH      0x00
#define HW_INFO_EFUSE      0x01
#define HW_INFO_VBAT       0x02

#define ACT_ERASE_SECTOR  0x04
#define ERASE_SECTOR_ADDRESS_ERR      0x01
#define ERASE_SECTOR_LENGTH_ERR       0x02
#define ERASE_SECTOR_TASK_ERR         0x03

#define ACT_WRITE_CONFIG   0x05
#define WRITE_CONFIG_LEN_ERR     0x01
#define WRITE_CONFIG_ERR         0x02
#define WRITE_CONFIG_TASK_ERR    0x03

#define ACT_READ_CONFIG    0x06
#define READ_CONFIG_LEN_ERR      0x01
#define CONFIG_TAG_NOT_EXIST     0x02
#define READ_TASK_FAIL           0x03

#define ACT_DELETE_CONFIG  0x07
#define DELETE_TAG_NOT_EXIST       0x01
#define DELETE_TASK_FAIL           0x02

#define ACT_ACTIVE_INFO    0x08
#define ACTIVE_INFO_LEN_ERR     0x03
#define ACTIVE_INFO_SIGN_ERR    0x04
#define ACTIVE_TASK_ERR         0x05
#define ACTIVE_RANDOM_ERR       0x06


typedef struct _erase_operation {
    uint32_t address;
    uint16_t pages;
} erase_opt_t;

uint8_t dispatch_write_request(uint16_t src_task_id, uint16_t pid, uint8_t *data, uint16_t len);

#endif
