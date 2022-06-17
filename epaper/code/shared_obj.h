/*
 * share_obj.h
 * @brief
 * Created on: May 9, 2022
 * Author: Yanye
 */

#ifndef CODE_SHARED_OBJ_H_
#define CODE_SHARED_OBJ_H_

#include <stdint.h>
#include "flash_tlv.h"

extern __align(4) uint8_t shared_tx_buffer[256];

extern tlv_sector_t shared_tlv_sector;

#endif /* CODE_SHARED_OBJ_H_ */
