/*
 * uart.h
 *
 * Created: 19/10/2025 12:07:46 am
 *  Author: Yu Heng
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stddef.h>

#define RX0 PE0
#define TX0 PE1

extern volatile uint8_t buffer_index;

uint8_t checksum_checker(uint8_t *buf, size_t len);

void UART_init(uint16_t ubbr);
void UART0_tx(uint8_t data);
uint8_t UART0_rx();
void UART1_tx(uint8_t data);
uint8_t UART1_rx();

void print(char* s);
void UART0_send_bytes(char *s, size_t size);

void UART1_send_bytes(char *s, size_t size);
void UART1_receive_bytes(uint8_t *buf);

uint8_t UART1_is_ready();

#endif /* UART_H_ */