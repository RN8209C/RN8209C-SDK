#include "simulate_uart.h"
#include "key_led_relay.h"
#include "nrf52.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "system_time.h"
#include "byte_fifo.h"
#include "nrf_delay.h"
/*
	need  open TIMER_ENABLED in sdk_config,and enable TIMER1_ENABLE TIMER2_ENABLE 
*/

#define SIMULATE_TX_HIGH		nrf_gpio_pin_set(SIMULATE_TX_PIN)
#define SIMULATE_TX_LOW		nrf_gpio_pin_clear(SIMULATE_TX_PIN)
const nrf_drv_timer_t TIMER_RX = NRF_DRV_TIMER_INSTANCE(1);
const nrf_drv_timer_t TIMER_TX = NRF_DRV_TIMER_INSTANCE(2);

static uint8_t uart_parity  ;
static uint8_t rx_buf[SIMULATE_RX_BUF_LEN];
static uint8_t tx_buf[SIMULATE_TX_BUF_LEN];
STU_BYTE_QUEUE tx_queue;
STU_BYTE_QUEUE rx_queue;
static uint32_t rx_timer;
static uint32_t uart_baud;
void rx_timer_interrupt(nrf_timer_event_t event_type, void* p_context)
{
	static uint8_t rx_data;
	static uint8_t rx_bit_num = 0;
	switch (event_type)
	{
	    	case NRF_TIMER_EVENT_COMPARE0:
			rx_data |= (nrf_gpio_pin_read(SIMULATE_RX_PIN)<<rx_bit_num);
		//	USER_RTT("RX PIN LEVEL IS%d\n",rx_data);
			rx_bit_num ++;
			if(rx_bit_num==8)
			{
				queue_byte_in(&rx_queue,rx_data);
				rx_timer= get_sys_time();
				if(uart_parity == 0)
				{
					//USER_RTT("no parity\n");
					rx_bit_num = 0;
					rx_data = 0;
					nrf_drv_timer_disable(&TIMER_RX);
					nrf_drv_gpiote_in_event_enable(SIMULATE_RX_PIN,1);
				}	
			}
			else if(rx_bit_num == 9)
			{
				//USER_RTT("have parity\n");
				rx_bit_num = 0;
				rx_data = 0;
				nrf_drv_timer_disable(&TIMER_RX);
				nrf_drv_gpiote_in_event_enable(SIMULATE_RX_PIN,1);
			}
	    	default:
	        	break;
	}
}

void recv_data(void)
{
	uint8_t buf[SIMULATE_RX_BUF_LEN];
	uint16_t  read_len;
	if(queue_bytes(&rx_queue))
	{
		if(timepassed( rx_timer, 15 ))
		{
			USER_RTT("send data  to uart,data len is %d\n",queue_bytes(&rx_queue));
			read_len = queue_buf_read(&rx_queue,buf,SIMULATE_RX_BUF_LEN);
			simulate_uart_send( buf, read_len);
		}
	}
}

void in_pin_hangler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin ==SIMULATE_RX_PIN )
	{
		nrf_delay_us(30 );
		if(nrf_gpio_pin_read(SIMULATE_RX_PIN)==0)
		{
			nrf_drv_gpiote_in_event_disable(SIMULATE_RX_PIN);
	  		nrf_drv_timer_enable(&TIMER_RX);
		}	
	}
}
void rx_pin_gpiote_init(void)
{
	 nrf_drv_gpiote_init();
	 nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(1);
	 in_config.pull = NRF_GPIO_PIN_PULLUP;
	 nrf_drv_gpiote_in_init(SIMULATE_RX_PIN,&in_config,in_pin_hangler);
	 nrf_drv_gpiote_in_event_enable(SIMULATE_RX_PIN,1);
}
void rx_timer_init(void)
{
    	uint32_t time_ticks ;
    	uint32_t err_code = NRF_SUCCESS;
	time_ticks = 1000000/uart_baud;
	nrf_gpio_cfg_input(SIMULATE_RX_PIN,NRF_GPIO_PIN_PULLUP);

    	nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
	timer_cfg.mode = NRF_TIMER_MODE_TIMER;
	timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_16;
	timer_cfg.interrupt_priority = TIMER_INTERRUPT_PRIORITY;
	timer_cfg.frequency = NRF_TIMER_FREQ_1MHz;
    	err_code = nrf_drv_timer_init(&TIMER_RX, &timer_cfg, rx_timer_interrupt);
    	APP_ERROR_CHECK(err_code);
    	nrf_drv_timer_extended_compare(&TIMER_RX, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}


uint16_t simulate_uart_recv(uint8_t *buf,uint16_t read_len)
{
	if(read_len>SIMULATE_RX_BUF_LEN)
		return queue_buf_read(&rx_queue,buf,SIMULATE_RX_BUF_LEN);
	else
		return queue_buf_read(&rx_queue,buf,read_len);
}
void simulate_uart_send(uint8_t *buf,uint16_t buflen)
{
	//USER_RTT("uart send len is %d\n",buflen);
	queue_buf_write(&tx_queue,buf,buflen);
	nrf_drv_timer_enable(&TIMER_TX);
}
void tx_timer_interrupt(nrf_timer_event_t event_type, void* p_context)
{
	static uint8_t data=0;
	static uint8_t flag=0;
	static uint8_t bit_num=0;
	static uint8_t parity = 0;
	switch (event_type)
	{
	    	case NRF_TIMER_EVENT_COMPARE0:
			if(flag == 0)
			{
				SIMULATE_TX_LOW;
				flag = 1;
				queue_byte_out(&tx_queue,&data);
				if(uart_parity)
				{
					parity = 0;
					for(uint8_t i=0 ; i<8 ; i++)
					{
						parity += (data>>i)&0x01;
					}
					
					if(parity%2==0)
					{
						if(uart_parity== PARITY_EVEN)
							parity = 0;
						else
							parity = 1;
					}
					else
					{
						if(uart_parity== PARITY_EVEN)
							parity = 1;
						else
							parity = 0;
					}
					//USER_RTT("PARITY is is %d\n",parity);
						
				}
			}
			else if(flag == 1)
			{
				if(data&0x01)
					SIMULATE_TX_HIGH;
				else
					SIMULATE_TX_LOW;
				data = data>>1;
				bit_num ++;
				if(uart_parity)
				{
					if(bit_num==8)
					{
						data = parity;
					//	USER_RTT("PARITY is  %d,data is %d\n",parity,data);
					}
					else if(bit_num == 9)
					{
						bit_num = 0;
						flag = 2;
					}
				}
				else
				{
					if(bit_num == 8)
					{
						flag = 2;
						bit_num = 0;
					}
				}
			}
			else if(flag == 2)
			{
				SIMULATE_TX_HIGH;
				flag = 0;
				if(queue_byte_is_empty(&tx_queue)==1)
				{
					//USER_RTT("tx fifo is empty\n");
					nrf_drv_timer_disable(&TIMER_TX);	
				}
			}
			break;
	}
}
void tx_timer_init(void)
{
	uint32_t time_ticks ;
    	uint32_t err_code = NRF_SUCCESS;
	time_ticks = 1000000/uart_baud;
	nrf_gpio_cfg_output(SIMULATE_TX_PIN);
	nrf_gpio_pin_set(SIMULATE_TX_PIN);


    	nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
	timer_cfg.mode = NRF_TIMER_MODE_TIMER;
	timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_16;
	timer_cfg.interrupt_priority = TIMER_INTERRUPT_PRIORITY;
	timer_cfg.frequency = NRF_TIMER_FREQ_1MHz;
    	err_code = nrf_drv_timer_init(&TIMER_TX, &timer_cfg, tx_timer_interrupt);
    	APP_ERROR_CHECK(err_code);
    	nrf_drv_timer_extended_compare(&TIMER_TX, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
		
}
static uint8_t uart_init_flag = 0;
void simulate_uart_init(uint32_t baud,uint8_t parity)
{
	if(uart_init_flag==0)
	{
		uart_init_flag = 1;
		uart_baud = baud;
		uart_parity = parity;
		rx_timer_init();
		tx_timer_init();
		rx_pin_gpiote_init();
		queue_byte_init(&tx_queue,tx_buf,SIMULATE_TX_BUF_LEN);
		queue_byte_init(&rx_queue,rx_buf,SIMULATE_RX_BUF_LEN);
	}
}
void simulate_uart_uinit(void)
{
	if(uart_init_flag == 1)
	{
		uart_init_flag = 0;
		nrf_drv_timer_uninit(&TIMER_TX);
		nrf_drv_timer_uninit(&TIMER_RX);
		nrf_drv_gpiote_uninit();
	}
}
