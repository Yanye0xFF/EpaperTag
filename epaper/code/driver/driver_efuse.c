#include <stdint.h>

#include "driver_efuse.h"
#include "plf.h"
#include "apb2spi.h"
#include "co_math.h"
#include "co_utils.h"

struct efuse_reg_t {    
	uint32_t ctrl;    
	uint32_t d0;    
	uint32_t d1;    
	uint32_t d2;    
	uint32_t len_reg;
};

void efuse_write(uint32_t *data)
{
    volatile struct efuse_reg_t * const efuse_reg = (volatile struct efuse_reg_t *)(EFUSE_BASE);
    
     //ool_write(0x40,0x40);
    efuse_reg->len_reg = 0x2814;     
    efuse_reg->d0 = data[0];
    efuse_reg->d1 = data[1];
    efuse_reg->d2 = data[2];   
	
	efuse_reg->ctrl = 0x05;            
	while(((efuse_reg->ctrl)&CO_BIT(2)));            
	while(!((efuse_reg->ctrl)&CO_BIT(0)));    
  
}

void efuse_read(uint32_t *data)
{
    volatile struct efuse_reg_t * const efuse_reg = (volatile struct efuse_reg_t *)(EFUSE_BASE);
    
    efuse_reg->ctrl = 0x1;
    co_delay_100us(1);
    while( (efuse_reg->ctrl & CO_BIT(1))  == 0 );
	*(data + 0) = efuse_reg->d0;    
	*(data + 1) = efuse_reg->d1;    
	*(data + 2) = efuse_reg->d2;
}

