/*
 * main.c
 *
 * Created: 17.12.2025 11:36:12
 * Author : Yu Heng
 */ 

#define F_CPU 16000000L // Specify oscillator frequency
#define BAUD_RATE 9600
#define UBBR ((F_CPU)/(16UL*BAUD_RATE) - 1) // for UART_init

#define START_BYTE 0xFF

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include "spi.h"
#include "w25qxx.h"
#include "uart.h"

#define START_ADDR 0x0000

void write_flash(uint32_t page, uint8_t *data, size_t size) {
	page = page & ~(SPM_PAGESIZE - 1);
	
	for (uint16_t i = 0; i < ((size - 1) / SPM_PAGESIZE) + 1; i++) {
		boot_page_erase(page + SPM_PAGESIZE * i);
		boot_spm_busy_wait();
		
		for (uint16_t j = 0; j < SPM_PAGESIZE; j += 2)
		boot_page_fill(page + SPM_PAGESIZE * i + j, 0xFFFF);
		
		for (uint16_t j = 0; j < SPM_PAGESIZE; j += 2) {
			uint32_t curr_index = (SPM_PAGESIZE * i) + j;
			
			uint16_t word;
			
			if (curr_index < size) {
				uint8_t lo = data[curr_index];
				uint8_t hi = (curr_index + 1 < size) ? data[curr_index + 1] : 0xFF;
				word = lo | (hi << 8);
				} else {
				word = 0xFFFF;
			}
			
			boot_page_fill(page + SPM_PAGESIZE * i + j, word);
		}
		
		boot_page_write(page + SPM_PAGESIZE * i);
		boot_spm_busy_wait();
	}
	
	boot_rww_enable();
}

void write_from_flash(uint32_t image_size) {
	
	// because of the size of SRAM, need to write page by page
	
	cli(); // always remember to disable interrupts when writing to flash
	
	uint8_t buf[SPM_PAGESIZE] = {0};
	
	for (int i = 0; SPM_PAGESIZE * i < image_size; i++) {
		if (image_size - SPM_PAGESIZE * i >= SPM_PAGESIZE) {
			W25QXX_read_stream(SPM_PAGESIZE * i, buf, SPM_PAGESIZE);
			write_flash(SPM_PAGESIZE * i, buf, SPM_PAGESIZE);
		} else {
			W25QXX_read_stream(SPM_PAGESIZE * i, buf, image_size - SPM_PAGESIZE * i);
			write_flash(SPM_PAGESIZE * i, buf, image_size - SPM_PAGESIZE * i);
		}
	}
	
	sei();
}

// only for testing purposes
void TEMP_write_to_flash() {
		// temporary write to the flash
		W25QXX_clear_A();
		for (int i = 0; i < 1536; i++) {
			;
			//W25QXX_write_app(&(data[i]), 1);
		}
		W25QXX_write_remainder(0x01);
}

typedef enum {
	NONE,
	READY,
	UPLOADING,
	SUCCESS,
	FAILURE
} UploadStatus;

void send_telemetry(UploadStatus status) {
	uint8_t tm[57] = {0};
	tm[0] = START_BYTE; // start character
	tm[1] = 0x37;
	tm[55] = status;
	tm[56] = status; // checksum is just status because other fields 0x0
	UART1_send_bytes(tm, 57);
}

int main(void)
{	
	MCUSR = 0; // clear reset flags
	wdt_disable();
	
	UART_init(UBBR);
	SPI_init();
	W25QXX_clear_A(); // implement moving to B in the future
	
	sei();
	
	// Indicator that reset occurred
	DDRB = 0xFF;
	for (int i = 0; i < 2; i++) {
		PORTB ^= (1 << PB7);
		_delay_ms(500);
	}
	
	send_telemetry(READY);
	
	char init_text[20] = "CatSat Bootloader\r\n";
	print(init_text);
	strcpy(init_text, "----------------\r\n");
	print(init_text);
	strcpy(init_text, "Booting...\r\n");
	print(init_text);
	
	uint8_t buf[100] = {0};
	
	uint8_t length = 0;
	uint8_t command = 0;
	
	uint8_t reset_flag = 0;
	
	char waiting[15] = "Start Upload\r\n";
	char finishd[15] = "\r\nFinished\r\n";
	char dot[2] = ".";
	char confirm[15] = "All's Good!\r\n";
	char command_id[5] = "XX\r\n";
	
	while (1) {
		
		// print(waiting);
		UART1_receive_bytes(buf);
		// print(finishd);
		
		length = buf[0];
		command = buf[1];
		
		switch (command) {
			case 0xAB:
				print(confirm);
				strcpy(command_id, "AB\r\n");
				print(command_id);
				break;
			case 0x05:
				send_telemetry(UPLOADING);
				print(waiting);
				// Upload Start -- reset everything
				W25QXX_reset_app();
				W25QXX_write_app(&(buf[2]), length - 1); // exclude the command byte
				break;
			case 0x06:
				print(dot);
				W25QXX_write_app(&(buf[2]), length - 1);
				break;
			case 0x07:
				print(finishd);
				W25QXX_write_app(&(buf[2]), length - 2);
				uint8_t app_checksum = buf[length];
				if (W25QXX_write_remainder(app_checksum)) {
					reset_flag = 1;
					send_telemetry(SUCCESS);
				} else {
					send_telemetry(FAILURE);
				}
				// W25QXX_visualise_page(0x0, 512);
				break;
			default:
				strcpy(command_id, "DE\r\n");
				print(command_id);
				break;
		}
		
		if (reset_flag == 1) break;
	}
	
	write_from_flash(app_size);
	
	char hello[30] = "Jumping to the start\r\n";
	print(hello);
	
	// deinitialise everything
	DDRB = 0;
	DDRE = 0;
	DDRD = 0;
	SPCR = 0;
	UCSR0B = 0;
	UCSR1B = 0;
	
	eeprom_write_dword((uint32_t*)0x00, app_size); // write to eeprom
	
	asm(
	"clr r1\n"
	"jmp 0x0"
	);
	
	while (1) ; // should be unreachable
}
