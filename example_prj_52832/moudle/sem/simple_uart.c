/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdint.h>
#include "nrf.h"
#include "simple_uart.h"
#include "nrf_gpio.h"
#include "bsp.h"
#include "nrf_uart.h"


STU_BYTE_QUEUE uartQueue;
uint8_t uart_buf[UART_BUF_SIZE];
 bool uart_open_flg=false;


void UartInit(uint32_t io_tx,uint32_t io_rx,uint32_t baud)
{
	nrf_gpio_cfg_output(io_tx);
	NRF_UART0->PSELTXD = io_tx;
	nrf_gpio_cfg_input(io_rx, NRF_GPIO_PIN_NOPULL);
	NRF_UART0->PSELRXD = io_rx;
	NRF_UART0->BAUDRATE         = (baud<< UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX    = 1;
	NRF_UART0->TASKS_STARTRX    = 1;
	NRF_UART0->EVENTS_RXDRDY    = 0;
	//NRF_UART0->CONFIG = NRF_UART_PARITY_INCLUDED;

	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos);
	NVIC_SetPriority(UART0_IRQn, 3);
	NVIC_EnableIRQ(UART0_IRQn);
	uart_open_flg=true;
	
}
void uart_init(void)
{
	UartInit(UART_TX_PIN,UART_RX_PIN,NRF_UART_BAUDRATE_115200);
	//UartInit(UART_TX_PIN,UART_RX_PIN,NRF_UART_BAUDRATE_4800);
	queue_byte_init(&uartQueue,uart_buf,UART_BUF_SIZE);
}

void Uart_close(uint32_t io_tx,uint32_t io_rx)
{
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Disabled << UART_INTENSET_RXDRDY_Pos);
	NVIC_DisableIRQ(UART0_IRQn);
	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
//	nrf_gpio_cfg_default(io_tx);
	nrf_gpio_cfg_default(io_rx);
	uart_open_flg=false;
}

void simple_uart_put(uint8_t cr)
{
    uint16_t counter=16*1000;
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        counter--;
        if(counter==0)
        {
            NRF_UART0->EVENTS_TXDRDY = 0;
            return;
        }
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}

void uart_recv_test(void)
{
	uint8_t buf[UART_BUF_SIZE];
	uint16_t  read_len;
	if(queue_bytes(&uartQueue))
	{
	//	USER_RTT("send data  to uart,data len is %d\n",queue_bytes(&rx_queue));
		read_len = queue_buf_read(&uartQueue,buf,UART_BUF_SIZE);
		uartSend( buf, read_len);
	}
}
void uartSend(unsigned char *datain,unsigned short len)
{
    unsigned short i;

    for(i=0; i<len; i++)
    {
        simple_uart_put(datain[i]);
    }

}
uint16_t  uartrecv(uint8_t *buf,uint16_t read_len)
{
	if(read_len>UART_BUF_SIZE)
		return queue_buf_read(&uartQueue,buf,UART_BUF_SIZE);
	else
		return queue_buf_read(&uartQueue,buf,read_len);
}



uint8_t simple_uart_get(void)
{
    unsigned short counter=0;
#define DELAY_MS   (16*1000)

    while ((NRF_UART0->EVENTS_RXDRDY != 1)&&(counter++<DELAY_MS))
    {
        // Wait for RXD data to be received
    }

    NRF_UART0->EVENTS_RXDRDY = 0;
    return (uint8_t)NRF_UART0->RXD;
}



bool is_uart_enable(void)
{
	return uart_open_flg;
}





void UART0_IRQHandler(void)
{
    queue_byte_in(&uartQueue,simple_uart_get());
}
int fputc(int ch, FILE *f)
{
	uint8_t temp = (uint8_t)ch;
	uartSend(&temp,1);
	return ch;
}
