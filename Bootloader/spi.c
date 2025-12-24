/*
 * spi.c
 *
 * Created: 18/10/2025 11:02:46 pm
 *  Author: Yu Heng
 */ 

#include "spi.h"

void SPI_init(void) {
	DDR_SPI = (1<<SCK) | (1<<MOSI) | (1<<SS); // set relevant pins to output, set SS to output to enter master mode
	DDR_SS = (1<<SS_W25Qxx) | (1<<SS_BMP390);
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0); // enable spi, set as master, use mode 0, set sck = f/16
	PORT_SS |= (1<<SS_W25Qxx) | (1<<SS_BMP390); // add when more slaves added
}

void SPI_select(uint8_t addr) {
	PORT_SS &= ~(1<<addr);
}

void SPI_deselect(uint8_t addr) {
	PORT_SS |= (1<<addr);
}

uint8_t SPI_transaction(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1<<SPIF))); // wait until SPIF is 1
	return SPDR;
}