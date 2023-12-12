#include "hardware_config.h"
#include "nrf52.h"

MCU_PIN_SELECT current_pin;

MCU_PIN_SELECT mcu_52832_pin = {
	.relay = 30,
	.led_ctr = 12,
	.net_led = 31,
	.power_led = 2,
	.uart_rx = 20,
	.uart_tx = 22,
	.rn8209c_rx = 14,
	.rn8209c_tx = 13
};

MCU_PIN_SELECT mcu_52833_pin = {
	.relay = 29,
	.led_ctr = 13,
	.net_led = 30,
	.power_led = 31,
	.uart_rx = 22,
	.uart_tx = 7,
	.rn8209c_rx = 15,
	.rn8209c_tx = 14
};

MCU_PIN_SELECT mcu_52840_pin = {
	.relay = 31,
	.led_ctr = 15,
	.net_led = 7,
	.power_led = 5,
	.uart_rx = 23,
	.uart_tx = 12,
	.rn8209c_rx = 17,
	.rn8209c_tx = 16
};

void hardware_choose_init(void)
{
	if (NRF_FICR->INFO.PART == MCU_52832)
	{
		current_pin = mcu_52832_pin;
	}
	else if (NRF_FICR->INFO.PART == MCU_52833)
	{
		current_pin = mcu_52833_pin;
	}
	else if (NRF_FICR->INFO.PART == MCU_52840)
	{
		current_pin = mcu_52840_pin;
	}
}
