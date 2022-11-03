#ifndef WS2812_52832_H____
#define WS2812_52832_H____
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define POWER_LED_RMT_TX_CHANNEL RMT_CHANNEL_0
#define NETWORK_LED_RMT_TX_CHANNEL RMT_CHANNEL_1
#define NUMLEDS  			6
#define RGB_LED_PIN  		26
#define WS2812_POWER_IO	16
void setcolor(uint32_t rgb);
void setcolor_test(uint32_t *rgb);
void set_network_led(uint32_t rgb);
void ws2812_control_init( );// Update the LEDs to the new state. Call as needed.// This function will block the current task until the RMT peripheral is finished sending // the entire sequence.
#endif




