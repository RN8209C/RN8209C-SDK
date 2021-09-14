#ifndef __KEY__LED__RELAY__H
#define	__KEY__LED__RELAY__H
#include "nrf_gpio.h"
#include<stdio.h>
#include<stdint.h>

#define LED_RED_PIN   		31
#define LED_GREEN_PIN		29
#define LED_BLUE_PIN		28
#define RELAY_PIN			30



#define LED_RED_ON		nrf_gpio_pin_clear(LED_RED_PIN)
#define LED_RED_OFF		nrf_gpio_pin_set(LED_RED_PIN)
#define LED_GREEN_ON	nrf_gpio_pin_clear(LED_GREEN_PIN)
#define LED_GREEN_OFF	nrf_gpio_pin_set(LED_GREEN_PIN)
#define LED_BLUE_ON		nrf_gpio_pin_clear(LED_BLUE_PIN)
#define LED_BLUE_OFF	nrf_gpio_pin_set(LED_BLUE_PIN)

#define LED_YELLOW_PIN 	(LED_RED_PIN+LED_GREEN_PIN) 
#define LED_YELLOW_ON		LED_RED_ON;LED_GREEN_ON
#define LED_YELLOW_OFF		LED_RED_OFF;LED_GREEN_OFF

#define LED_PURPLE_PIN		(LED_RED_PIN+LED_BLUE_PIN)
#define LED_PURPLE_ON		LED_RED_ON;LED_BLUE_ON
#define LED_PURPLE_OFF		LED_RED_OFF;LED_BLUE_OFF

#define LED_WHITE_PIN		(LED_YELLOW_PIN+LED_BLUE_PIN)
#define LED_WHITE_ON		LED_YELLOW_ON;LED_BLUE_ON
#define LED_WHITE_OFF		LED_YELLOW_OFF;LED_BLUE_OFF

#define  LED_BLUE_YELLOW_FLASH_PIN		1

#define LED_RED_TOGGLE  	nrf_gpio_pin_toggle(LED_RED_PIN)
#define LED_GREEN_TOGGLE	nrf_gpio_pin_toggle(LED_GREEN_PIN)
#define LED_YELLOW_TOGGLE		LED_RED_TOGGLE;	LED_GREEN_TOGGLE

#define LED_STATE_SERIAL						1
#define LED_STATE_CALIBRATE_PROCESS			2
#define LED_STATE_CALIBRATE_SUCCESS			3
#define LED_STATE_CALIBRATE_FALSE				4
#define LED_STATE_PRODUCT_TEST_CONNECT		5
#define LED_STATE_BLE_ADV						6		
#define LED_STATE_BLE_CONNECTED_RELAY_ON	7
#define LED_STATE_ALL_LED_OFF					8
#define LED_STATE_OTA_PROCESS					9
#define LED_STATE_OTA_SUCCESS					10
#define LED_STATE_OTA_FLASE					11
#define LED_STATE_RESET_DEV					12
#define LED_STATE_RESET_ENERGY					13
#define LED_STATE_OVERLOAD						14

#define RELAY_STATE_OFF			0x00
#define RELAY_STATE_ON			0x01
#define SET_RELAY_ON 		nrf_gpio_pin_set(RELAY_PIN)
#define SET_RELAY_OFF		nrf_gpio_pin_clear(RELAY_PIN)


#define KEY_PIN				11
#define SHORT_PRESS_TIME	(100)
#define LONG_PRESS_TIME		(10000)
#define MIDDLE_PRESS_TIME	(3000)



void set_led_state(uint8_t     led_state);
void key_check(void);
void led_key_relay_init(void);
void led_flash_process(void);
uint8_t get_product_key_flag(void);
#endif
