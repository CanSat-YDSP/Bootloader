/*
 * W25Qxx.c
 *
 * Created: 20/10/2025 1:07:14 pm
 *  Author: Yu Heng
 */ 

#define F_CPU 16000000L

#include "w25qxx.h"
#include "spi.h"
#include "uart.h"
#include <avr/delay.h>

void W25QXX_init() {
	W25QXX_clear_chip();
}

void W25QXX_readID(uint8_t* mfr, uint8_t* memtype, uint8_t* cap) {
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x9F);
	*mfr = SPI_transaction(0x00);
	*memtype = SPI_transaction(0x00);
	*cap = SPI_transaction(0x00);
	SPI_deselect(SS_W25Qxx);
}

uint8_t W25QXX_test() {
	uint8_t mfr, memtype, cap;
	W25QXX_readID(&mfr, &memtype, &cap);
	
	return (mfr == 0xEF);
}

uint8_t W25QXX_read_data(uint32_t addr) {
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x03);
	SPI_transaction((addr>>16) & 0xFF);
	SPI_transaction((addr>>8) & 0xFF);
	SPI_transaction((addr) & 0xFF);
	uint8_t data = SPI_transaction(0x00);
	SPI_deselect(SS_W25Qxx);
	return data;
}

void W25QXX_read_stream(uint32_t addr, uint8_t *buf, size_t len) {
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x03);
	SPI_transaction((addr>>16) & 0xFF);
	SPI_transaction((addr>>8) & 0xFF);
	SPI_transaction((addr) & 0xFF);
	
	for (size_t i = 0; i < len; i++) {
		buf[i] = SPI_transaction(0x00);
	}
	
	SPI_deselect(SS_W25Qxx);
}

void W25QXX_release_power_down() {
	SPI_select(SS_W25Qxx);
	SPI_transaction(0xAB);
	SPI_deselect(SS_W25Qxx);
}

uint8_t W25QXX_read_status() {
	uint8_t byte;
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x05);
	byte = SPI_transaction(0x00);
	//UART0_send_bytes(&byte, 1);
	SPI_deselect(SS_W25Qxx);
	return byte;
}

void W25QXX_write_enable() {
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x06);
	SPI_deselect(SS_W25Qxx);
	while (!(W25QXX_read_status() & 0x02)); // bit1 = WEL
}

// max 256 bytes!
void W25QXX_write_page(uint32_t addr, uint8_t *bytes, size_t len) {
	W25QXX_write_enable();
	
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x02);
	SPI_transaction((addr>>16) & 0xFF);
	SPI_transaction((addr>>8) & 0xFF);
	SPI_transaction((addr) & 0xFF);
	
	for (int i = 0; i < len; i++) {
		SPI_transaction(bytes[i]);
	}
	
	SPI_deselect(SS_W25Qxx);
	
	while (W25QXX_read_status() & 0x01);
}

void W25QXX_clear_chip() {
	W25QXX_write_enable();
	SPI_select(SS_W25Qxx);
	SPI_transaction(0x60);
	SPI_deselect(SS_W25Qxx);
	
	while (W25QXX_read_status() & 0x01);
}

void W25QXX_visualise_page(uint32_t addr, size_t len) {
	
	uint8_t response[len];
	W25QXX_read_stream(addr, response, len);
	
	char header[34];
	sprintf(header, "Starting from address: 0x%06x\r\n", addr);
	print(header);
	
	char output[6];
	size_t counter = 0;
	
	for (int i = 0; i < ((len-1)/8) + 1; i++) {
		sprintf(output, "+%04d ", i * 8);
		print(output);
		for (int j = 0; j < 8; j++) {
			sprintf(output, "| %02x ", response[counter]);
			counter++;
			print(output);
		}
		print("|\r\n");
	}
}

void W25QXX_write(uint32_t addr, uint8_t *bytes, size_t len) {
	W25QXX_clear_chip();
	
	size_t number_of_pages = (size_t)((len-1) / 256) + 1;
	
	for (int i = 0; i < number_of_pages; i++) {
		size_t remaining = len - i*256;
		size_t length = (remaining < 256) ? remaining : 256;
		W25QXX_write_page(addr + i*256, bytes + i*256, length);
	}
}

void W25QXX_clear_64(uint32_t addr) {
	W25QXX_write_enable();
	SPI_select(SS_W25Qxx);
	SPI_transaction(0xD8);
	SPI_transaction((addr>>16) & 0xFF);
	SPI_transaction((addr>>8) & 0xFF);
	SPI_transaction((addr) & 0xFF);
	SPI_deselect(SS_W25Qxx);
	
	while (W25QXX_read_status() & 0x01);
}

void W25QXX_clear_A() {
	for (uint32_t addr = 0; addr < 0x200000; addr += 0x10000) {
	    W25QXX_clear_64(addr);
    }
}

void W25QXX_clear_B() {
	for (uint32_t addr = 0x200000; addr < 0x400000; addr += 0x10000) {
		W25QXX_clear_64(addr);
	}
}

uint8_t curr_buf[256];
uint16_t curr_index = 0;
uint32_t curr_addr = 0x000000;
uint8_t curr_checksum = 0;
uint8_t ready_for_reflash = 0;

uint32_t app_size = 0;

void W25QXX_reset_app() {
	curr_index = 0;
	curr_addr = 0x000000;
	curr_checksum = 0;
	ready_for_reflash = 0;
}

void W25QXX_write_app(uint8_t *buf, size_t len) {
	
	 curr_checksum ^= checksum_checker(buf, len); // function from uart.h
	
	if (curr_index + len > 256) {
		uint8_t difference = 256 - curr_index;
		
		for (int i = 0; i < 256 - difference; i++) {
			curr_buf[curr_index] = buf[i];
			curr_index++;
		}
		
		W25QXX_write_page(curr_addr, curr_buf, 256);
		curr_addr += 256;
		curr_index = 0;
		
		for (int i = 0; i < len - difference; i++) {
			curr_buf[curr_index] = buf[difference + i];
			curr_index++;
		}
	}
	else {
		for (int i = 0; i < len; i++) {
			curr_buf[curr_index] = buf[i];
			curr_index++;
		}
	}
}

// remember to add uint8_t checksum

uint8_t W25QXX_write_remainder(uint8_t checksum) {
	if (curr_index > 0) {
		W25QXX_write_page(curr_addr, curr_buf, curr_index);
	}
	
	app_size = curr_addr + curr_index;
	 
	curr_addr = 0x000000; // reset
	curr_index = 0; // reset
	
	char output[30];
	sprintf(output, "Calculated Checksum: %02x\r\n", curr_checksum);
	print(output);
	
	sprintf(output, "Received Checksum: %02x\r\n", checksum);
	print(output);
	
	if (curr_checksum == checksum) {
		ready_for_reflash = 1;
	}
	else {
		W25QXX_clear_A();	// clear out corrupted memory
	}
	
	return curr_checksum == checksum;
}