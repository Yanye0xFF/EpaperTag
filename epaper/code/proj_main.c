#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "app.h"
#include "user_sys.h"
#include "app_api.h"
#include "flash.h"
#include "co_bt.h"
#include "driver_efuse.h"
#include "user_timer.h"

#include "epaper_svc.h"
#include "ssd1680.h"
#include "common.h"
#include "utils.h"
#include "flash_tlv.h"
#include "shared_obj.h"

static os_timer_t slp_timer;
static void slp_timer_callback(void *parg);

static const uint8_t default_mac[BD_ADDR_LEN] = {0x00, 0x00, 0x00, 0x09, 0x17, 0x20};

void user_proj_main_before_ble_ini(void) {
    tlv_block_t tlv_blk;
    bool result;
    uint8_t buffer[32];
    uint16_t adv_int;

    rf_set_lna_gain(0x0C);  
    rf_set_tx_power(BLE_TX_POWER_LEVEL);

    set_runtime_default();
    flash_tlv_init(&shared_tlv_sector, FLASH_TLV_MAJOR, FLASH_TLV_MINOR, 4096);
    // MAC address
    result = flash_tlv_query(&shared_tlv_sector, TAG_BLE_MAC, &tlv_blk);
    if(result) {
        flash_tlv_read(&tlv_blk, buffer, 0, BD_ADDR_LEN);
        memswap(buffer, BD_ADDR_LEN);
        ble_set_addr((uint8_t *)buffer);
        set_runtime_bit(FLAG_ACTIVED, BIT_SET);
    }else {
        ble_set_addr((uint8_t *)default_mac);
    }
    // advertise interval
    result = flash_tlv_query(&shared_tlv_sector, TAG_ADV_INT, &tlv_blk);
    if(result) {
        flash_tlv_read(&tlv_blk, buffer, 0, sizeof(uint16_t));
        memcpy(&adv_int, buffer, sizeof(uint16_t));
        set_adv_interval(adv_int);
    }
    // ble name
    result = flash_tlv_query(&shared_tlv_sector, TAG_BLE_NAME, &tlv_blk);
    if(result) {
        memset(buffer, 0x00, sizeof buffer);
        flash_tlv_read(&tlv_blk, buffer, 0, tlv_blk.length);
        set_ble_name(buffer);
    }
}

#define CPU_CLK_12M    2
#define CPU_CLK_24M    1
#define CPU_CLK_48M    0

void user_proj_main(void) {
#if DEBUG_MODE
    uint32_t i;
    i = app_boot_get_firmwave_version();
    printf("epaper start: %d\n", i);
#endif

    user_set_cpu_clk(2);

    epd_init();
    epd_open();
    epd_sleep();
    epd_close();

    #if DEBUG_MODE
    printf("run debug mode\n");
    #else
    os_timer_setfn(&slp_timer, slp_timer_callback, NULL);
    os_timer_arm(&slp_timer, SLEEP_DELAY_AFTER_BOOT, false);
    #endif

    epaper_svc_init();
}

static void slp_timer_callback(void *parg) {
    appm_sleep_start();
}
