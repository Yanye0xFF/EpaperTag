/*
 * ssd1680.c
 */

#include "ssd1680.h"
#include "utils.h"
#include "string.h"
#include "user_timer.h"
#include "user_sys.h"
#include "user_mem.h"
#include "system.h"
#include "app.h"
#include "softspi.h"
#include "stdbool.h"
#include "watchdog.h"

#if (!USE_SOFT_SPI)
#include "ssp.h"
#endif

// 2bit/pixel, EPD_RAM_WIDTH * 2 * 250
static uint8_t frameBuffer[EPD_RAM_SIZE];
static flush_done_callback_t callback;

static volatile STATUS epd_status = IDLE;
static int32_t busy_timeout;
static os_timer_t epd_timer;
static void epd_timer_callback(void *parg);

static void epd_write_cmd(epd_cmd_t *cmd);

void epd_init(void) {
    // init buys, reset pin and softspi
    // BUSY PIN input pulldown
    system_set_port_mux(EPD_BUSY_PORT, EPD_BUSY_PIN, PORT_FUNC_GPIO);
    gpio_set_dir(EPD_BUSY_PORT, EPD_BUSY_PIN, GPIO_DIR_IN);
    // pull down default, if HIGH epd is busy
    system_set_port_pull((EPD_BUSY_PORT * 8) + EPD_BUSY_PIN , false);

    // DC PIN
    system_set_port_mux(EPD_DC_PORT, EPD_DC_PIN, PORT_FUNC_GPIO);
    gpio_set_dir(EPD_DC_PORT, EPD_DC_PIN, GPIO_DIR_OUT);
    // 'data' default
    gpio_portc_write(gpio_portc_read() | BIT(EPD_DC_PIN));

    //
    pmu_set_gpio_to_CPU(EPD_RESET_PORT, BIT(7));
    // RESET PIN output high
    system_set_port_mux(EPD_RESET_PORT, EPD_RESET_PIN, PORT_FUNC_GPIO);
    gpio_set_dir(EPD_RESET_PORT, EPD_RESET_PIN, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() | BIT(EPD_RESET_PIN));
    
#if USE_SOFT_SPI
    softspi_init();
#else
    // ssp1 for EPD
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSP1_CLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSP1_CSN);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSP1_DOUT);
    // mode 0:master;  1:slave.
    ssp_init_x(ssp1, EPD_SPI_SPEED, 2, 8, 0);    
    ssp_clear_rx_fifo_x(ssp1);
    ssp_disable_x(ssp1);
#endif
}

void epd_open(void) {
    // reset epd, send init sequence
    epd_cmd_t cmd;

    gpio_porta_write(gpio_porta_read() & (~BIT(EPD_RESET_PIN)));
    delay_ms(10);
    gpio_porta_write(gpio_porta_read() | BIT(EPD_RESET_PIN));
    delay_ms(10);

    //no pull, or 50uA leakage current in sleep mode.
    //pmu_set_gpio_pull(EPD_RESET_PORT, BIT(7), false);
    pmu_set_gpio_value(EPD_RESET_PORT, BIT(7), 1);
    pmu_set_gpio_output(EPD_RESET_PORT, BIT(7), true);
    pmu_set_gpio_to_PMU(EPD_RESET_PORT, BIT(7));

    cmd.cmd = SW_RESET;
    cmd.len = 0;
    epd_write_cmd(&cmd);
    
    busy_timeout = 100;
    do {
        delay_ms(1);
        busy_timeout--;
    }while(busy_timeout && (gpio_portc_read() & BIT(EPD_BUSY_PIN)));

    cmd.cmd = DRIVER_OUTPUT_CTRL;
    cmd.len = 3;
    #ifdef EPD128X296
    cmd.data = "\x27\x01\x00";
    #else
    cmd.data = "\xF9\x00\x00";
    #endif
    epd_write_cmd(&cmd);

    cmd.cmd = DATA_ENTRY_MODE;
    cmd.len = 1;
    // AM=0, Y increment, X increment
     cmd.data = "\x03";
    // AM=1, Y decrement, X increment
    // cmd.data = "\x05";
    // AM=1, X decrement, Y increment
    // cmd.data = "\x06";
    epd_write_cmd(&cmd);

    cmd.cmd = SET_RAM_X_ADDR;
    cmd.len = 2;
    // 0x0F-->(15+1)*8=128
    cmd.data = "\x00\x0F";
    epd_write_cmd(&cmd);

    cmd.cmd = SET_RAM_Y_ADDR;
    cmd.len = 4;
    #ifdef EPD128X296
    cmd.data = "\x00\x00\x27x01";
    #else
    cmd.data = "\x00\x00\xF9x00";
    #endif
    epd_write_cmd(&cmd);

    cmd.cmd = BORDER_WAVEFORM_CTRL;
    cmd.len = 1;
    cmd.data = "\x05";
    epd_write_cmd(&cmd);

    cmd.cmd = TEMP_SENSOR_CTRL;
    cmd.len = 1;
    // select Internal temperature sensor
    cmd.data = "\x80";
    epd_write_cmd(&cmd);

    cmd.cmd = DISP_UPDATE_CTRL;
    cmd.len = 2;
    cmd.data = "\x00\x80";
    epd_write_cmd(&cmd);

    cmd.cmd = SET_RAM_X_COUNTER;
    cmd.len = 1;
    cmd.data = "\x00";
    epd_write_cmd(&cmd);

    cmd.cmd = SET_RAM_Y_COUNTER;
    cmd.len = 2;
    cmd.data = "\x00\x00";
    epd_write_cmd(&cmd);
}

void epd_clear(void) {
    memset(frameBuffer, COLOR_FILL_WHITE, EPD_RAM_SIZE);
}

/**
 * @param offset 写入起始地址, frameBuffer的偏移量, 单位:字节
 * @param data 待写入的数据
 * @param length 待写入数据的长度, 单位:字节
 * */
uint8_t epd_write(uint32_t offset, uint8_t *data, uint32_t length) {
    if(offset >= EPD_RAM_SIZE) {
        return EPD_OFFSET_ERR;
    }
    if((offset + length) > EPD_RAM_SIZE) {
        return EPD_LENGTH_ERR;
    }
    memcpy((frameBuffer + offset), data, length);
    return EPD_OK;
}

/**
 * @param index display ram index (0~15)
 * @param mask framebuffer mask
 * @param disram
 * */
static inline void bit_scale(uint32_t index, uint8_t mask, uint8_t *displayRAM) {
    uint8_t out = 0x00, bits, cursor = 7;
    uint32_t idx = (index << 1);
    // 2bytes convert to 1byte
    for(int j = 0; j < 4; j++) {
        bits = ((frameBuffer[idx] >> (j << 1)) & mask);
        bits = (((bits >> 1) | bits) & 0x1);
        out |= (bits << cursor);
        cursor--;
    }
    idx++;
    for(int j = 0; j < 4; j++) {
        bits = ((frameBuffer[idx] >> (j << 1)) & mask);
        bits = (((bits >> 1) | bits) & 0x1);
        out |= (bits << cursor);
        cursor--;
    }
    *(displayRAM + index) = out;
}

bool epd_flush(void) {
    uint32_t i, j;
    epd_cmd_t epd_cmd;
#if USE_SOFT_SPI
    uint8_t cmd;
#endif
    if(epd_status == BUSY) {
        return false;
    }
    epd_status = BUSY;

    // free mem:20692

    // 1bit/pixel
    uint8_t *display_ram = (uint8_t *)os_malloc(EPD_PART_RAM_SIZE, OS_MEM_NON_RETENTION);

    gpio_portc_write(gpio_portc_read() & (~(1 << EPD_DC_PIN)));
    #if USE_SOFT_SPI
    cmd = WRITE_RAM_BW;
    softspi_write(&cmd, 1, true, false);
    #else
    ssp_put_byte_x(ssp1, WRITE_RAM_BW);
    ssp_enable_x(ssp1);
    ssp_wait_busy_bit_x(ssp1);
    #endif
    gpio_portc_write(gpio_portc_read() | (1 << EPD_DC_PIN));
    for(j = 0; j < EPD_HEIGHT; j++) {
        for(i = 0; i < EPD_RAM_WIDTH; i++) {
            bit_scale((j * EPD_RAM_WIDTH + i), COLOR_MASK_BW, display_ram);
        }
    }
    #if USE_SOFT_SPI
    softspi_write(display_ram, EPD_PART_RAM_SIZE, false, false);
    #else
    for(i = 0; i < 125; i++) {
        ssp_put_data_x(ssp1, (display_ram + i * 32), 32);
        ssp_wait_busy_bit_x(ssp1);
        wdt_feed();
    }
    ssp_disable_x(ssp1);
    #endif

    gpio_portc_write(gpio_portc_read() & (~(1 << EPD_DC_PIN)));
    #if USE_SOFT_SPI
    cmd = WRITE_RAM_RED;
    softspi_write(&cmd, 1, false, false);
    #else
    ssp_put_byte_x(ssp1, WRITE_RAM_RED);
    ssp_enable_x(ssp1);
    ssp_wait_busy_bit_x(ssp1);
    #endif
    
    gpio_portc_write(gpio_portc_read() | (1 << EPD_DC_PIN));
    for(j = 0; j < EPD_HEIGHT; j++) {
        for(i = 0; i < EPD_RAM_WIDTH; i++) {
            bit_scale((j * EPD_RAM_WIDTH + i), COLOR_MASK_RED, display_ram);
        }
    }

    #if USE_SOFT_SPI
    softspi_write(display_ram, EPD_PART_RAM_SIZE, false, true);
    #else
    for(i = 0; i < 125; i++) {
        ssp_put_data_x(ssp1, (display_ram + i * 32), 32);
        ssp_wait_busy_bit_x(ssp1);
        wdt_feed();
    }
    ssp_disable_x(ssp1);
    #endif

    os_free(display_ram);
    
    epd_cmd.cmd = DISPLAY_UPDATE_CTRL2;
    epd_cmd.len = 1;
    epd_cmd.data = "\xF7";
    epd_write_cmd(&epd_cmd);

    epd_cmd.cmd = MASTER_ACTIVATION;
    epd_cmd.len = 0;
    epd_write_cmd(&epd_cmd);

    busy_timeout = 26;
    os_timer_setfn(&epd_timer, epd_timer_callback, NULL);
    os_timer_arm(&epd_timer, EPD_REFRESH_TIMEOUT, true);
    return true;
}

void epd_sleep(void) {
    epd_cmd_t epd_cmd;
    epd_cmd.cmd = DEEP_SLP_MODE;
    epd_cmd.len = 1;
    epd_cmd.data = "\x03";
    epd_write_cmd(&epd_cmd);
}

void epd_close(void) {
    #if (USE_SOFT_SPI == 1)
    softspi_close();
    #else
    ssp_disable_x(ssp1);
    system_set_port_mux(SPI_CS_PORT, SPI_CS_PIN, PORT_FUNC_GPIO);
    system_set_port_mux(SPI_MOSI_PORT, SPI_MOSI_PIN, PORT_FUNC_GPIO);
    system_set_port_mux(SPI_SCLK_PORT, SPI_SCLK_PIN, PORT_FUNC_GPIO);
    softspi_close();
    #endif
    gpio_set_dir_as_high_imp(EPD_BUSY_PORT, EPD_BUSY_PIN);
    gpio_set_dir_as_high_imp(EPD_DC_PORT, EPD_DC_PIN);
}

void epd_set_flush_done_callback(flush_done_callback_t cbk) {
    callback = cbk;
}

static void epd_write_cmd(epd_cmd_t *cmd) {
    gpio_portc_write(gpio_portc_read() & (~(1 << EPD_DC_PIN)));
    
#if USE_SOFT_SPI
    softspi_write(&(cmd->cmd), 1, true, false);
#else
    ssp_put_byte_x(ssp1, cmd->cmd);
    ssp_enable_x(ssp1);
    ssp_wait_busy_bit_x(ssp1);
#endif
    if(cmd->len) {
        gpio_portc_write(gpio_portc_read() | (1 << EPD_DC_PIN));
        #if USE_SOFT_SPI
        softspi_write((uint8_t *)cmd->data, cmd->len, false, true);
        #else
        ssp_put_data_x(ssp1, (const uint8_t *)(cmd->data), cmd->len);
        ssp_wait_busy_bit_x(ssp1);
        ssp_disable_x(ssp1);
        #endif
    }else {
#if USE_SOFT_SPI
        softspi_write(NULL, 0, false, true);
#endif
    }
}

static void epd_timer_callback(void *parg) {
    //if(busy_timeout && (gpio_portc_read() & BIT(EPD_BUSY_PIN))) {
    if(busy_timeout){
        busy_timeout--;
    }else {
        os_timer_disarm(&epd_timer);
        epd_status = IDLE;
#if (USE_SOFT_SPI == 0)
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSP1_CLK);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSP1_CSN);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSP1_DOUT);
        // mode 0:master;  1:slave.
        ssp_init_x(ssp1, EPD_SPI_SPEED, 2, 8, 0);
        ssp_clear_rx_fifo_x(ssp1);
        ssp_disable_x(ssp1);
#endif
        epd_sleep();
        epd_close();
        if(callback) {
            callback((busy_timeout == 0) ? FLUSH_TIMEOUT : FLUSH_DONE);
        }
        busy_timeout = 0;
    }
}

bool epd_is_idle(void) {
    return (epd_status == IDLE);
}

uint8_t *epd_get_framebuffer(void) {
    return frameBuffer;
}

