#ifndef __SIMULATE__UART__H
#define __SIMULATE__UART__H
#include<stdio.h>
#include<stdint.h>
#include "nrf_drv_timer.h"
#include "bsp.h"
#include "app_error.h"
#define USER_DEBUG 1
#if USER_DEBUG
#define USER_RTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define USER_RTT(...)
#endif

#define 	SIMULATE_TX_PIN	14//22//14
#define 	SIMULATE_RX_PIN	13//20//13
#define 	PARITY_NONE	0
#define	PARITY_EVEN	1
#define 	PARITY_ODD	2

#define SIMULATE_TX_BUF_LEN	300
#define SIMULATE_RX_BUF_LEN	300
#define TIMER_INTERRUPT_PRIORITY		3
void recv_data(void);
void simulate_uart_init(uint32_t baud,uint8_t parity);
void simulate_uart_send(uint8_t *buf,uint16_t buflen);
uint16_t simulate_uart_recv(uint8_t *buf,uint16_t read_len);
void simulate_uart_uinit(void);
#endif
