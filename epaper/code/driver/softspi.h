/*
 * softspi.h
 * @brief 3-wire spi and only tx
 */

#ifndef _SOFTSPI_H_
#define _SOFTSPI_H_

#include "stdint.h"
#include "system.h"

// PA6 MOSI
// PA4 SCLK
// PA5 CS
#define SPI_COMM_PORT    GPIO_PORT_A

#define SPI_MOSI_PORT    GPIO_PORT_A
#define SPI_MOSI_PIN     GPIO_BIT_6

#define SPI_SCLK_PORT    GPIO_PORT_A
#define SPI_SCLK_PIN     GPIO_BIT_4

#define SPI_CS_PORT      GPIO_PORT_A
#define SPI_CS_PIN       GPIO_BIT_5

void softspi_init(void);

uint32_t softspi_write(uint8_t *data, uint32_t length, uint8_t take_cs, uint8_t release_cs);

void softspi_close(void);

#endif
