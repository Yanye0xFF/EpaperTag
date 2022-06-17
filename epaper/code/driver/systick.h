/*
 * systick.h
 * @brief
 * Created on: May 7, 2022
 * Author: Yanye
 */

#ifndef CODE_DRIVER_SYSTICK_H_
#define CODE_DRIVER_SYSTICK_H_

#include "stdint.h"

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(volatile uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(volatile uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(volatile uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(volatile uint32_t *)(_SCB_BASE + 0xC))

uint32_t sysTick_config(uint32_t ticks);

#endif /* CODE_DRIVER_SYSTICK_H_ */
