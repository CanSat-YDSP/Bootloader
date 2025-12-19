/*
 * uart.c
 *
 * Created: 19/10/2025 12:07:58 am
 *  Author: Yu Heng
 */ 

#define START_CHAR 0xFF
#define BUFFER_SIZE 256

#include <avr/interrupt.h>

#include <stddef.h>
#include "uart.h"

volatile uint8_t XBEE_buffer[BUFFER_SIZE];
volatile uint8_t write_index = 0;
volatile uint8_t starting_character_index = 0;

uint8_t checksum_checker(uint8_t *buf, size_t len) {
	uint8_t checksum = 0;
	for (size_t i = 0; i < len; i++) {
		checksum ^= buf[i];
	}
	return checksum;
}

void UART_init(uint16_t ubbr) {
	
	DDRE |= (1 << PE1); // for USART0
	DDRD |= (1 << PD3); // for USART1
	
	// for UART 0
	UBRR0H = (ubbr>>8); // set baud rate
	UBRR0L = (ubbr);
	UCSR0B = (1<<TXEN0) | (1<<RXEN0); // enable TX and RX
	UCSR0C = (1<<UCSZ00) | (1<<UCSZ01); // | (1<<USBS0); // 8 bit, 2 stop-bits
	
	// for UART 1
	UBRR1H = (ubbr>>8);
	UBRR1L = (ubbr);
	UCSR1B = (1<<TXEN1) | (1<<RXEN1);
	UCSR1C = (1<<UCSZ10) | (1<<UCSZ11);
}

void UART0_tx(uint8_t data) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void UART1_tx(uint8_t data) {
	while (!(UCSR1A & (1<<UDRE1)));
	UDR1 = data;
}

void print(char *s) {
	while (*s != '\0') {
		UART0_tx(*s);
		s++;
	}
}

void UART1_send_bytes(char *s, size_t size) {
	char* end = s + size;
	while (s < end) {
		UART1_tx(*s);
		s++;
	}
}

uint8_t UART1_rx() {
	while (!(UCSR1A & (1<<RXC1)));
	return UDR1;
}

void UART1_receive_bytes(uint8_t *buf) {
	uint8_t checksum = 0;
	
	while (1) {
		uint8_t byte = UART1_rx();
		
		if (byte != 0xFF) continue; // wait until byte is 0xFF
		
		uint8_t length = UART1_rx();
		
		for (uint8_t i = 0; i <= length; i++) {
			buf[i+1] = UART1_rx();
		}
		
		buf[0] = length;
		
		checksum = buf[length + 1];
		
		if (checksum_checker(&(buf[1]), length) == checksum) break; // exit loop if good
	}
}

//void UART1_receive_bytes(uint8_t *buf)
//{
	//uint8_t start;
	//uint8_t length;
	//uint8_t checksum;
//
	//while (1) {
		//cli();
//
		//start = starting_character_index;
//
		//length = XBEE_buffer[(start + 1) % BUFFER_SIZE];
		//
		//if (length == 0 || length > BUFFER_SIZE - 3) {
			//sei();
			//continue;
		//}
		//
		//for (uint8_t i = 0; i < length; i++) {
			//buf[i] = XBEE_buffer[(start + 2 + i) % BUFFER_SIZE];
		//}
		//
		//checksum = XBEE_buffer[(start + 2 + length) % BUFFER_SIZE];
//
		//sei();
		//
		//if (checksum_checker(buf, length) == checksum) {
			//break;
		//}
	//}
//}

void UART0_send_bytes(char *s, size_t size) {
	char* end = s + size;
	while (s < end) {
		UART0_tx(*s);
		s++;
	}
}