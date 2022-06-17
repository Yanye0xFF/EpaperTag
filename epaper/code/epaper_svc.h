#ifndef __PROJ_EPAPER_H__
#define __PROJ_EPAPER_H__

// 625us/LSB
// msg.intv_min不起作用，但必须 >= 0x20 && <= msg.intv_max。
#define GAP_ADV_INT_MIN    0x20    // 20ms
// msg.intv_max参数表示37,38,39一个循环结束后，到下个循环结束之间的时间间隔。中间的时间可以休眠。
// 37,38,39每个channel停留1.5ms。3个channel切换完毕则需要等待msg.intv_max参数规定的间隔延迟。
// 该宏当前无效，使用get_adv_interval()获取用户配置的广播间隔
#define GAP_ADV_INT_MAX    0x640   // 1000ms

#define EPD_SERVICE_UUID   0xD0FF

// 测试模式: 1
// 正式固件：0
#define DEBUG_MODE    0

void epaper_svc_init(void);

#endif




