
#ifndef SIMPLE_UART_H
#define SIMPLE_UART_H

#include <stdbool.h>
#include <stdint.h>
#include "byte_fifo.h"
#include "dev_cfg.h"
#include "hardware_config.h"
#define UART_BUF_SIZE 1024
extern STU_BYTE_QUEUE uartQueue;
extern uint8_t uart_buf[UART_BUF_SIZE];

#ifdef MK114B
#define UART_TX_PIN   22//14//22
#define UART_RX_PIN   20//13//20
#endif

#ifdef MK117B
#define 	UART_TX_PIN	 current_pin.uart_tx//22//14
#define 	UART_RX_PIN	 current_pin.uart_rx//20//13
#endif

bool is_queue_empty(void);
void uartSend(unsigned char *datain,unsigned short len);
bool RevQueOut(unsigned char *dataout);


void UartInit(uint32_t io_tx,uint32_t io_rx,uint32_t baud);
void Uart_close(uint32_t io_tx,uint32_t io_rx);
bool is_uart_enable(void);
void simple_uart_put(uint8_t cr);
void uart_init(void);
void uart_recv_test(void);
uint16_t  uartrecv(uint8_t *buf,uint16_t read_len);

#endif
