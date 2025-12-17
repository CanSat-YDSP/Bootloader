/*
 * spi.h
 *
 * Created: 18/10/2025 11:02:34 pm
 *  Author: Yu Heng
 */ 

#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>

// defining port macros
#define SS_W25Qxx PA3
#define SS_BMP390 PA4
#define SS PB0
#define SCK PB1
#define MOSI PB2
#define MISO PB3

#define DDR_SPI DDRB
#define PORT_SPI PORTB

#define DDR_SS DDRA
#define PORT_SS PORTA

void SPI_init(void);
void SPI_select(uint8_t addr);
void SPI_deselect(uint8_t addr);
uint8_t SPI_transaction(uint8_t data);

#endif /* SPI_H_ */