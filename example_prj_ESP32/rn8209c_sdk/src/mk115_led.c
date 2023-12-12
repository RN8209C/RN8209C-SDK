#include "mk115_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void mk115_led_init(void)
{
	ws2812_control_init();
	LED_ALL_OFF;
}

