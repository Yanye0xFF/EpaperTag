/*
 * share_obj.c
 * @brief
 * Created on: May 9, 2022
 * Author: Yanye
 */

#include "shared_obj.h"
#include "common.h"

__align(4) uint8_t shared_tx_buffer[256];

tlv_sector_t shared_tlv_sector;
