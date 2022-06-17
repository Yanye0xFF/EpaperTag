/*
 * softspi.c
 */

#include "softspi.h"
#include "utils.h"
#include "gpio.h"
#include "user_sys.h"
#include "watchdog.h"

static void sdelay(uint32_t i) {
    while(i-->0);
}

/**
 * @brief gpio init, default mode is MODE0
 * */
void softspi_init(void) {
    // CS
    system_set_port_mux(SPI_CS_PORT, SPI_CS_PIN, PORT_FUNC_GPIO);
    gpio_set_dir(SPI_CS_PORT, SPI_CS_PIN, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() | (1 << SPI_CS_PIN));
    // MOSI
    system_set_port_mux(SPI_MOSI_PORT, SPI_MOSI_PIN, PORT_FUNC_GPIO);
    gpio_set_dir(SPI_MOSI_PORT, SPI_MOSI_PIN, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() & (~(1 << SPI_MOSI_PIN)));
    // SCLK
    system_set_port_mux(SPI_SCLK_PORT, SPI_SCLK_PIN, PORT_FUNC_GPIO);
    gpio_set_dir(SPI_SCLK_PORT, SPI_SCLK_PIN, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() & (~(1 << SPI_SCLK_PIN)));
}

uint32_t softspi_write(uint8_t *data, uint32_t length, uint8_t take_cs, uint8_t release_cs) {
    uint32_t i, j, reg;
    uint8_t out, bit;
    // take cs
    if(take_cs) {
        gpio_porta_write(gpio_porta_read() & (~(1 << SPI_CS_PIN)));
    }

    for(i = 0; i < length; i++) {
        out = data[i];
        for(j = 0; j < 8; j++) {
            gpio_porta_write(gpio_porta_read() & (~(1 << SPI_SCLK_PIN)));
            // MSB first
            bit = (out >> (7 - j)) & 0x1;
            reg = gpio_porta_read();
            reg &= (~(1 << SPI_MOSI_PIN));
            reg |= (bit << SPI_MOSI_PIN);
            gpio_porta_write(reg);
            sdelay(10);
            gpio_porta_write(gpio_porta_read() | (1 << SPI_SCLK_PIN));
            sdelay(10);
        }
    }

    // release cs
    if(release_cs) {
        gpio_porta_write(gpio_porta_read() | (1 << SPI_CS_PIN));
    }
    return length;
}

void softspi_close(void) {
    // EPD屏在休眠期间一直有供电，将它除RST外的所有io管脚置为高阻
    gpio_set_dir_as_high_imp(SPI_COMM_PORT, SPI_CS_PIN);
    gpio_set_dir_as_high_imp(SPI_COMM_PORT, SPI_SCLK_PIN);
    gpio_set_dir_as_high_imp(SPI_COMM_PORT, SPI_MOSI_PIN);
}
