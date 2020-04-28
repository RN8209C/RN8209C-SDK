#include "led.h"
#include "driver/gpio.h"


void   user_led_init(void)
{
	LED_RED_OFF;
	LED_GREEN_OFF;
	LED_BLUE_OFF;
	gpio_config_t io_conf;
    	io_conf.intr_type = GPIO_INTR_DISABLE;
    	io_conf.mode = GPIO_MODE_OUTPUT;
    	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_LED;
    	io_conf.pull_down_en = 0;
    	io_conf.pull_up_en = 1;  
    	gpio_config(&io_conf);	

}

