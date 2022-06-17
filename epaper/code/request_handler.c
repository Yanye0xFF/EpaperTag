#include "request_handler.h"
#include "rwble_hl_error.h"
#include <string.h>
#include <stdbool.h>
#include "user_profile.h"
#include "gattc_task.h"
#include "ke_msg.h"
#include "app_api.h"
#include "flash.h"
#include "driver_efuse.h"
#include "user_msg_q.h"
#include "ke_msg.h"
#include "ke_mem.h"
#include "user_task.h"
#include "co_bt.h"
#include "adc.h"

#include "common.h"
#include "ssd1680.h"
#include "utils.h"
#include "shared_obj.h"
#include "flash_tlv.h"

#define WRITE_DISP_RAM_HEADER_SIZE    6
#define FILL_DISP_RAM_HEADER_SIZE     2
#define READ_DISP_RAM_MAX             242
#define USER_SPACE_PAGE_START         0x28000
#define USER_SPACE_PAGE_END           0x3F000
#define USER_SPACE_PAGE_LIMIT         0x40000
#define USER_SPACE_PAGE_MAX           24

static uint8_t async_task_id;

typedef void (*action_handler_t)(uint8_t *data, uint16_t len);

static void write_display_ram(uint8_t *data, uint16_t len);

#define FLUSH_DISPLAY_EVENT     0x3F16
static void flush_screen(uint8_t *data, uint16_t len);

static void read_display_ram(uint8_t *data, uint16_t len);
static void read_hardware_info(uint8_t *data, uint16_t len);

#define ERASE_FLASH_EVENT     0x3F17
static void erase_flash_sector(uint8_t *data, uint16_t len);

#define WRITE_CONFIG_EVENT     0x3F18
static void write_config(uint8_t *data, uint16_t len);

#define READ_CONFIG_EVENT     0x3F19
static void read_config(uint8_t *data, uint16_t len);

#define DELETE_CONFIG_EVENT     0x3F1A
static void delete_config(uint8_t *data, uint16_t len);

int async_task(os_event_t *msg);

static void ble_notify_app(uint8_t act_id, uint8_t ret_code, uint8_t *ext_data, uint8_t len);
static void epd_flush_done(uint8_t reason);

static uint16_t task_id;
static uint16_t prf_id;

static const action_handler_t handler[] = {
    write_display_ram,
    flush_screen,
    read_display_ram,
    read_hardware_info,
    erase_flash_sector,
    write_config,
    read_config,
    delete_config,
};

// BLE指令分发
uint8_t dispatch_write_request(uint16_t src_task_id, uint16_t pid, uint8_t *data, uint16_t len) {
    const uint32_t limit = sizeof(handler) / (sizeof(handler[0]));
    uint32_t opcode = data[0];
    task_id = src_task_id;
    prf_id = pid;
    if(opcode < limit) {
        handler[opcode](data, len);
    }
    return 0;
}

// 写入显存数据
static void write_display_ram(uint8_t *data, uint16_t len) {
    uint32_t offset;
    uint8_t length, result;
    if(len < WRITE_DISP_RAM_HEADER_SIZE) {
        ble_notify_app(ACT_WR_RAM, WR_RAM_PACKET_ERR, NULL, 0);
        return;
    }
    if(epd_is_idle()) {
        memcpy(&offset, (data + 1), sizeof(uint32_t));
        memcpy(&length, (data + 5), sizeof(uint8_t));
        result = epd_write(offset, (data + 6), length);
        ble_notify_app(ACT_WR_RAM, result, NULL, 0);
    }else {
        ble_notify_app(ACT_WR_RAM, WR_RAM_EPD_BUSY, NULL, 0);
    }
}

static void epd_flush_done(uint8_t reason) {
    if(get_runtime_bit(FLAG_PHONE_CONNECTED)) {
        ble_notify_app(ACT_FLUSH_SCREEN, EPD_FLUSH_DONE, NULL, 0);
    }
}

// 刷新价签屏幕
static void flush_screen(uint8_t *data, uint16_t len) {
    uint8_t res;
    os_event_t evt;
    if(epd_is_idle()) {
        res = os_task_create(async_task, &async_task_id);
        if(res == KE_TASK_OK) {
            evt.event_id = FLUSH_DISPLAY_EVENT;
            evt.param = NULL;
            evt.param_len = 0;
            evt.src_task_id = ACT_FLUSH_SCREEN;
            os_msg_post(async_task_id, &evt);
        }else {
            ble_notify_app(ACT_FLUSH_SCREEN, EPD_FLUSH_TASK_ERR, NULL, 0);
        }
    }else {
        ble_notify_app(ACT_FLUSH_SCREEN, EPD_BUSY_ERR, NULL, 0);
    }
}

// 读取显存
static void read_display_ram(uint8_t *data, uint16_t len) {
    uint32_t offset;
    uint8_t length;
    
    memcpy(&offset, (data + 1), sizeof(uint32_t));
    memcpy(&length, (data + 5), sizeof(uint8_t));
    
    if(length > READ_DISP_RAM_MAX) {
        ble_notify_app(ACT_RD_RAM, RD_RAM_LENGTH_ERR, NULL, 0);
        return;
    }
    if(((offset + length) > EPD_RAM_SIZE) || (offset >= EPD_RAM_SIZE)) {
        ble_notify_app(ACT_RD_RAM, RD_RAM_LENGTH_ERR, NULL, 0);
        return;
    }
    uint8_t *fb = epd_get_framebuffer();
    ble_notify_app(ACT_RD_RAM, RESULT_OK, (fb + offset), length);
}

// 获取硬件信息
static void read_hardware_info(uint8_t *data, uint16_t len) {
    uint32_t efuse_id[3];
    uint8_t payload_len = 0, tag; 
    uint8_t *buffer = (shared_tx_buffer + BLE_TX_OFFSET + 2);
    const uint8_t length_lut[] = {8, 12, 2};
        
    for(uint8_t i = 0; i < data[1]; i++) {
        tag = data[i + 2];
        if(tag == HW_INFO_FLASH) {
            get_flash_id((uint8_t *)efuse_id);
        }else if(tag == HW_INFO_EFUSE) {
            efuse_read(efuse_id);
        }else if(tag == HW_INFO_VBAT) {
            efuse_id[0] = get_vbat_value();
        }
        *buffer = tag;
        buffer++;
        payload_len++;
        memcpy(buffer, efuse_id, length_lut[tag]);
        buffer += length_lut[tag];
        payload_len += length_lut[tag];
    }
    
    ble_notify_app(ACT_RD_HW_INFO, RESULT_OK, NULL, payload_len);
}

// 擦除指定扇区
static void erase_flash_sector(uint8_t *data, uint16_t len) {
    uint32_t address, avail;
    uint16_t pages;
    uint8_t res;
    os_event_t evt;
    erase_opt_t opt;
    
    memcpy(&address, (data + 1), sizeof(uint32_t));
    memcpy(&pages, (data + 5), sizeof(uint16_t));
    if((address < USER_SPACE_PAGE_START) || (address > USER_SPACE_PAGE_END)) {
        ble_notify_app(ACT_ERASE_SECTOR, ERASE_SECTOR_ADDRESS_ERR, NULL, 0);
        return;
    }
    // aligned with 4096 bytes.
    address &= 0xFFFFF000;
    avail = (USER_SPACE_PAGE_LIMIT - address) >> 12; 
    if(pages > avail) {
        ble_notify_app(ACT_ERASE_SECTOR, ERASE_SECTOR_LENGTH_ERR, NULL, 0);
        return;
    }
    res = os_task_create(async_task, &async_task_id);
    if(res == KE_TASK_OK) {
        opt.address = address;
        opt.pages = pages;
        evt.event_id = ERASE_FLASH_EVENT;
        evt.param = (void *)&opt;
        evt.param_len = sizeof(erase_opt_t);
        evt.src_task_id = ACT_ERASE_SECTOR;
        os_msg_post(async_task_id, &evt);
    }else {
        ble_notify_app(ACT_ERASE_SECTOR, ERASE_SECTOR_TASK_ERR, NULL, 0);
    }
}

#define WRITE_CONFIG_PACKET_MIN    6

static void write_config(uint8_t *data, uint16_t len) {
    uint8_t res;
    os_event_t evt;
    uint16_t payload_len;
    // 1 header + 2 tag + 2 length + 最少1字节的数据
    if(len < WRITE_CONFIG_PACKET_MIN) {
        ble_notify_app(ACT_WRITE_CONFIG, WRITE_CONFIG_LEN_ERR, NULL, 0);
    }

    memcpy(&payload_len, (data + 3), sizeof(uint16_t));
    
    res = os_task_create(async_task, &async_task_id);
    if(res == KE_TASK_OK) {
        evt.event_id = WRITE_CONFIG_EVENT;
        evt.param = (void *)(data + 1);
        // 2 tag + 2 length + payload length
        evt.param_len = (4 + payload_len);
        evt.src_task_id = ACT_WRITE_CONFIG;
        os_msg_post(async_task_id, &evt);
    }else {
        ble_notify_app(ACT_WRITE_CONFIG, WRITE_CONFIG_TASK_ERR, NULL, 0);
    }
}

#define READ_CONFIG_REQUEST_LEN    3

static void read_config(uint8_t *data, uint16_t len) {
    uint16_t tag;
    uint8_t res;
    os_event_t evt;

    uint8_t *buffer = (shared_tx_buffer + BLE_TX_OFFSET + 2);

    if(len < READ_CONFIG_REQUEST_LEN) {
        ble_notify_app(ACT_READ_CONFIG, READ_CONFIG_LEN_ERR, NULL, 0);
        return;
    }

    memcpy(&tag, (data + 1), sizeof(uint16_t));

    if((tag & 0xFF00) == 0xFF00) {
        if(tag == 0xFF01) {
            buffer[0] = 0x01; buffer[1] = 0xFF;
            buffer[2] = 0x01; buffer[3] = 0x00;
            buffer[4] = get_runtime_bit(FLAG_ACTIVED);
            ble_notify_app(ACT_READ_CONFIG, RESULT_OK, NULL, 5);
        }else {
            ble_notify_app(ACT_READ_CONFIG, CONFIG_TAG_NOT_EXIST, NULL, 0);
        }
    }else {
        res = os_task_create(async_task, &async_task_id);
        if(res == KE_TASK_OK) {
            evt.event_id = READ_CONFIG_EVENT;
            evt.param = (void *)(data + 1);
            // 2 bytes tag
            evt.param_len = 2;
            evt.src_task_id = ACT_READ_CONFIG;
            os_msg_post(async_task_id, &evt);
        }else {
            ble_notify_app(ACT_READ_CONFIG, READ_TASK_FAIL, NULL, 0);
        }
    }
}

static void delete_config(uint8_t *data, uint16_t len) {
    uint8_t res;
    os_event_t evt;

    res = os_task_create(async_task, &async_task_id);
    if(res == KE_TASK_OK) {
        evt.event_id = DELETE_CONFIG_EVENT;
        evt.param = (void *)(data + 1);
        // 2 bytes tag
        evt.param_len = 2;
        evt.src_task_id = ACT_DELETE_CONFIG;
        os_msg_post(async_task_id, &evt);
    }else {
        ble_notify_app(ACT_DELETE_CONFIG, DELETE_TASK_FAIL, NULL, 0);
    }
}

int async_task(os_event_t *msg) {
    uint8_t result = RESULT_OK;
    uint8_t *extra = NULL;
    uint8_t extra_length = 0;

    // 'msg->param' will free automatically
    if(msg->event_id == FLUSH_DISPLAY_EVENT) {
        epd_init();
        epd_open();
        epd_set_flush_done_callback(epd_flush_done);
        epd_flush();

    }else if(msg->event_id == ERASE_FLASH_EVENT) {
        erase_opt_t *opt = (erase_opt_t *)msg->param;
        for(uint32_t i = 0; i < opt->pages; i++) {
            flash_erase(opt->address, FLASH_PAGE_SIZE);
            opt->address += FLASH_PAGE_SIZE;
        }

    }else if(msg->event_id == WRITE_CONFIG_EVENT) {
        uint16_t tag, length;
        uint8_t *value = (uint8_t *)(msg->param);

        memcpy(&tag, value, sizeof(uint16_t));
        memcpy(&length, (value + 2), sizeof(uint16_t));

        if(!flash_tlv_append(&shared_tlv_sector, tag, (value + 4), length)) {
            result = WRITE_CONFIG_ERR;
        }
        
    }else if(msg->event_id == READ_CONFIG_EVENT) {
        uint16_t tag;
        tlv_block_t blk;
        uint8_t *buffer = (shared_tx_buffer + BLE_TX_OFFSET + 2);

        memcpy(&tag, msg->param, sizeof(uint16_t));

        if(flash_tlv_query(&shared_tlv_sector, tag, &blk)) {
            memcpy(buffer, &tag, sizeof(uint16_t));
            memcpy((buffer + 2), &(blk.length), sizeof(uint16_t));
            flash_tlv_read(&blk, (buffer + 4), 0, blk.length);
            extra = buffer;
            extra_length = (uint8_t)(blk.length + 4);
        }else {
            result = CONFIG_TAG_NOT_EXIST;
        }
    }else if(msg->event_id == DELETE_CONFIG_EVENT) {
        uint16_t tag;
        bool ret;

        memcpy(&tag, msg->param, sizeof(uint16_t));

        ret = flash_tlv_delete(&shared_tlv_sector, tag);
        if(!ret) {
            result = DELETE_TAG_NOT_EXIST;
        }

    }
   
    ble_notify_app((uint8_t)msg->src_task_id, result, extra, extra_length);
    os_task_delete(async_task_id);
    return KE_MSG_CONSUMED;
}

static void ble_notify_app(uint8_t act_id, uint8_t ret_code, uint8_t *ext_data, uint8_t len) {
    struct user_prf_pkt pkt_param;
    uint8_t *buffer = (shared_tx_buffer + BLE_TX_OFFSET);
    
    buffer[0] = act_id;
    buffer[1] = ret_code;
    if(ext_data) {
        memcpy((buffer + 2), ext_data, len);
    }
    
    pkt_param.att_idx = EPAPER_IDX_NTF_VALUE;
    pkt_param.op_type = GATTC_NOTIFY;
    pkt_param.prf_id = prf_id;
    pkt_param.packet = (shared_tx_buffer + BLE_TX_OFFSET);
    pkt_param.packet_size = (len + 2);
    
    user_profile_send_ntf(KE_IDX_GET(task_id), &pkt_param);
}
