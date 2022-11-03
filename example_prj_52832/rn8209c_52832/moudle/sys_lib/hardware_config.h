#ifndef  __HARDWARE__CONFIG__H
#define __HARDWARE__CONFIG__H

#include<stdint.h>

#define MCU_52832	0x52832
#define MCU_52833	0x52833
#define MCU_52840	0x52840

typedef struct {
	uint8_t led_ctr;
	uint8_t net_led;
	uint8_t power_led;
	uint8_t relay;
	uint8_t uart_rx;
	uint8_t uart_tx;
	uint8_t rn8209c_rx;
	uint8_t rn8209c_tx;
}MCU_PIN_SELECT;

extern MCU_PIN_SELECT current_pin;

void hardware_choose_init(void);

#endif
