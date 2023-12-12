#include "mk117_led.h"
#include "driver/gpio.h"
#include "ws2812.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void mk117_led_init(void)
{
	ws2812_control_init();
	LED_ALL_OFF;
}

