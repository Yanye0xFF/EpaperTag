/*
 * systick.c
 * @brief
 * Created on: May 7, 2022
 * Author: Yanye
 */

#include "systick.h"

uint32_t sysTick_config(uint32_t ticks) {
    if ((ticks - 1) > 0xFFFFFF) {
        return 1;
    }
    _SYSTICK_LOAD = (ticks - 1);
    _SYSTICK_VAL  = 0;
    // 0B111  CLKSOURCE:AHB, TICKINT:0, ENABLE:1
    _SYSTICK_CTRL = 0x07;
    return 0;
}
