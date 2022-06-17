#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

#define STACK_SIZE                  (0x800 >> 1)
#define PATCH_MAP_BASE_ADDR         (0x20001000 + 0x800)  //32

#define CFG_CPU_CLK_SEL  (2)
#define CFG_CPU_CORTEX_M3
#define CFG_EMB
#define CFG_HOST
#define CFG_H4TL
#define CFG_BLE
#define CFG_RF_FREQ
#define CFG_ALLROLES
#define CFG_CON             6
#define CFG_MAX_CON         2
#define CFG_SEC_CON
#define CFG_SLEEP
#define CFG_WLAN_COEX
#define CFG_CHNL_ASSESS
#define CFG_APP
#define CFG_GTL
#define CFG_ATTC
#define CFG_ATTS
#define RP_HWSIM_BYPASS
#define CFG_PRF
#define CFG_NB_PRF          10
#define CFG_DBG_PRINTF
#define CFG_BUTTON
#define CFG_DBG_STACK_PROF

// 如果需要用到加密绑定，宏CFG_APP_SEC不能屏蔽
//#define CFG_APP_SEC

#define CFG_RTC_REPLACE_STANDBY
#define FR8019_0_RF_PARAM
//#define FR8019_1_RF_PARAM

#define LOG_SWO (0)

#define HID_DBG FR_DBG_OFF
#define SEC_DBG FR_DBG_OFF
#define APP_DBG FR_DBG_ON

//For user to enable mem uasge stastic or not.
#define USER_MEM_API_ENABLE       (0)
// 内部默认定义为1，启用
//#define USER_PROFILE_API_ENABLE       (1)
//#define USER_TASK_API_ENABLE      (1)
//#define USER_TIMER_API_ENABLE     (1)

//#define MEM_TEST_ENABLE (1)
//#define PROFILE_TEST_ENABLE (1)
//#define TASK_TEST_ENABLE (1)
//#define TIME_TEST_ENABLE (1)

//#define CFG_DEC_SBC
#define CFG_DEC_ADPCM_MS

// 使用自带的OTA功能
// 会增加0xfe00的service和0xff00~0xff03的characteristic
#if 1
#define CFG_APP_OTA
#define CFG_PRF_OTAS
#endif

#if 0
#define CFG_APP_RTCT
#define CFG_PRF_RTCTS
#endif

#if 0
#define CFG_APP_SPSS
#define CFG_PRF_SPSS
#endif

#if 0
#define CFG_APP_SPSC
#define CFG_PRF_SPSC
#endif


#endif  //_USER_CONFIG_H

