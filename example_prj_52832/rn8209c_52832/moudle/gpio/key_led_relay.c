#include"key_led_relay.h"
#include "system_time.h"
typedef struct
{
	uint8_t start_led_flash;
	uint32_t led_timer;
	uint16_t led_on_time;
	uint16_t led_off_time;
	uint16_t flash_cnt;  //如果为0 则一直闪烁
	uint8_t led_pin;
	uint8_t state;
}t_led;

static t_led led;




static void stop_led_flash(void)
{
	led.start_led_flash = 0;
	led.state = 0;	
	LED_GREEN_OFF;
	LED_RED_OFF;
	LED_BLUE_OFF;	
}



static void set_led_flash(uint8_t      led_id,uint16_t OnTime, uint16_t OffTime,uint32_t cnt)
{
	led.led_pin = led_id;
	led.led_on_time = OnTime;
	led.led_off_time =  OffTime;
	led.flash_cnt = cnt;
	led.state = 0;
	led.start_led_flash = 1;
}
//指示灯设置函数
void set_led_state(uint8_t     led_state)
{
	stop_led_flash();
	switch(led_state)
	{
		case LED_STATE_SERIAL:
			set_led_flash(LED_BLUE_YELLOW_FLASH_PIN,500,500,0);
			break;
		case LED_STATE_CALIBRATE_PROCESS:
			LED_PURPLE_ON;
			break;
		case LED_STATE_CALIBRATE_SUCCESS:
			LED_BLUE_ON;
			break;
		case LED_STATE_CALIBRATE_FALSE:	
			LED_RED_ON;
			break;
	}

}

void led_flash_process(void)
{
#define FLASH_ALWAY	100
	static  uint16_t cnt;
	static uint32_t timer;
	if(led.start_led_flash)
	{
		switch(led.state)
		{
			case 0:
				if(led.flash_cnt)
					cnt = led.flash_cnt;
				else
					cnt = FLASH_ALWAY;
				led.state = 1;

				timer = get_sys_time();
				break;
			case 1:  //熄灭led灯，有一个变化过程
				if(timepassed(timer,20))
				{
					led.state = 2;
				}
				break;
			case 2:	
				if(led.led_pin == LED_RED_PIN)
				{	
					LED_RED_ON;
				}
				else if(led.led_pin == LED_BLUE_PIN)
				{
					LED_BLUE_ON;
				}
				else if(led.led_pin == LED_GREEN_PIN)
				{
					LED_GREEN_ON;
				}
				else if(led.led_pin == LED_YELLOW_PIN)
				{
					LED_YELLOW_ON;
				}
				else if(led.led_pin == LED_BLUE_YELLOW_FLASH_PIN)
				{
					LED_BLUE_ON;
					LED_YELLOW_OFF;
				}
				else if(led.led_pin == LED_WHITE_PIN)
				{
					LED_WHITE_ON;
				}
				led.led_timer = get_sys_time();
				led.state = 3;
				break;
			case 3:
				if(timepassed( led.led_timer, led.led_on_time ))
				{
					led.state = 4;
				}
				break;
			case 4:
				if(led.led_pin == LED_RED_PIN)
				{	
					LED_RED_OFF;
				}
				else if(led.led_pin == LED_BLUE_PIN)
				{
					LED_BLUE_OFF;
				}
				else if(led.led_pin == LED_GREEN_PIN)
				{
					LED_GREEN_OFF;
				}
				else if(led.led_pin == LED_YELLOW_PIN)
				{
					LED_YELLOW_OFF;
				}
				else if(led.led_pin == LED_BLUE_YELLOW_FLASH_PIN)
				{
					LED_BLUE_OFF;
					LED_YELLOW_ON;
				}
				else if(led.led_pin == LED_WHITE_PIN)
				{
					LED_WHITE_OFF;
				}
				led.led_timer = get_sys_time();
				led.state = 5;
				break;
			case 5:
				if(timepassed( led.led_timer, led.led_off_time ))
				{
					cnt --;
					if(led.flash_cnt==0)
						cnt  = FLASH_ALWAY;
					if(cnt == 0)
					{
						LED_WHITE_OFF;
						led.start_led_flash = 0;
					}
					else
					{
						led.state = 2;
					}	
				}
				break;
		}
	}
	
}



void led_key_relay_init(void)
{
	nrf_gpio_cfg_output(LED_RED_PIN);
	nrf_gpio_cfg_output(LED_GREEN_PIN);
	nrf_gpio_cfg_output(LED_BLUE_PIN);
	nrf_gpio_cfg_output(RELAY_PIN);
	LED_RED_OFF;
	LED_GREEN_OFF;
	LED_BLUE_OFF;
}
