/*
 * W25Qxx.h
 *
 * Created: 20/10/2025 1:07:01 pm
 *  Author: Yu Heng
 */ 


#ifndef W25QXX_H_
#define W25QXX_H_

#include <avr/io.h>
#include <stddef.h>

void W25QXX_readID(uint8_t* mfr, uint8_t* memtype, uint8_t* cap);
uint8_t W25QXX_test();

uint8_t W25QXX_read_data(uint32_t addr);
void W25QXX_release_power_down();
uint8_t W25QXX_read_status();
void W25QXX_write_page(uint32_t addr, uint8_t *bytes, size_t len);
void W25QXX_clear_chip();

void W25QXX_read_stream(uint32_t addr, uint8_t *buf, size_t len);

void W25QXX_write_app(uint8_t *buf, size_t len);
void W25QXX_write_remainder(); //rmb to add back uint8_t checksum
void W25QXX_visualise_page(uint32_t addr, size_t len);

#endif /* W25QXX_H_ */