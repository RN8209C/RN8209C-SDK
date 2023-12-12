#include"key_led_relay.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"
#include <stdlib.h>
#include <string.h>
#include "system_time.h"

#ifdef MK114B

typedef struct
{
	uint8_t start_led_flash;
	uint32_t led_timer;
	uint16_t led_on_time;
	uint16_t led_off_time;
	uint16_t flash_cnt;  //���Ϊ0 ��һֱ��˸
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

static void set_led_flash(uint8_t led_id, uint16_t OnTime, uint16_t OffTime, uint32_t cnt)
{
	led.led_pin = led_id;
	led.led_on_time = OnTime;
	led.led_off_time =  OffTime;
	led.flash_cnt = cnt;
	led.state = 0;
	led.start_led_flash = 1;
}

//ָʾ�����ú���
void set_led_state(uint8_t led_state)
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
			case 1:  //Ϩ��led�ƣ���һ���仯����
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
#endif

#ifdef MK117B
#define LONG_PWM                13
#define SHORT_PWM	            6
#define LED_PWM_SIZE            (LONG_PWM + SHORT_PWM)

#define LED_CONTROL_PIONT		24
#define LED_RST_PERIOD			320  //16M 且LONG_PWM=13 SHORT_PWM=6
#define LED_NUMS  	            6

#define LED_STATE_0_RANGE_NUM	8
#define LED_STATE_1_RANGE_NUM	20

#define LED_OFF_VALUE	        0x000000

#define WHITE_VALUE		        0X808080
#define WHITE_BLUE_1		    0X4040FF
#define WHITE_BLUE_2		    0X2020FF
#define BLUE_VALUE			    0X0020FF
#define BLUE_GREEN_1		    0X0060FF
#define BLUE_GREEN_2		    0X008040
#define GREEN_VALUE		        0X006000
#define GREEN_YELLOW_1		    0X40A020
#define GREEN_YELLOW_2		    0X80A020
#define YELLOW_VALUE		    0X606000
#define YELLOW_ORANGE_1	        0X806000
#define YELLOW_ORANGE_2	        0XA06000
#define ORANGE_VALUE		    0XA04000
#define ORANGE_RED_1		    0XFF2D00
#define ORANGE_RED_2		    0XFD0F02
#define RED_VALUE			    0XA00000
#define RED_PURPLE_1		    0XA00080
#define RED_PURPLE_2		    0XA000FF
#define PURPLE_VALUE		    0X6010A0
#define DARK_PURPLE_VALUE	    0X401080

#define CYAN_VALUE		        0X0040A0

typedef struct
{
	uint8_t led_switch;
	uint32_t on_rgb;
	uint16_t on_time;
	uint32_t off_rgb;
	uint16_t off_time;
	uint16_t cnt;
	uint8_t state;
}led_control_t;

led_control_t network_led = {0};
led_control_t power_led = {0};

nrf_drv_pwm_t pwm_power= NRF_DRV_PWM_INSTANCE(0);
nrf_drv_pwm_t pwm_net= NRF_DRV_PWM_INSTANCE(1);

void pwm_interrupt_power(nrf_drv_pwm_evt_type_t event_type)   // 中断不开后续的状态不对
{
    
	if(event_type==NRF_DRV_PWM_EVT_FINISHED)
	{
		nrf_gpio_pin_clear(POWER_LED_PIN);
	}
}

void pwm_interrupt_net(nrf_drv_pwm_evt_type_t event_type)
{
	if(event_type==NRF_DRV_PWM_EVT_FINISHED)
	{
		nrf_gpio_pin_clear(NET_LED_PIN);
	}
}

void setcolor_net(uint32_t rgb)
{
	static uint16_t  seq_values[LED_RST_PERIOD + LED_CONTROL_PIONT*LED_NUMS] ={
        //320个周期的复位信号
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
    };	

	uint16_t led_buf[LED_CONTROL_PIONT];
	uint32_t temp = ((rgb<<8) &0x00ff0000) |((rgb>>8)&0x0000ff00)|(rgb&0x000000ff);

	for(uint8_t i=0 ; i<LED_CONTROL_PIONT ; i++)
	{
		if((temp<<i)&0x00800000)
		{
			led_buf[i] = 0X8000+LONG_PWM;
		}
		else
		{
			led_buf[i] = 0X8000+SHORT_PWM;
		}
	}
	for(uint8_t i=0 ; i<LED_NUMS ; i++)
	{
		memcpy(seq_values+LED_RST_PERIOD+i*LED_CONTROL_PIONT,led_buf,sizeof(led_buf));
	}
   	nrf_pwm_sequence_t const pwm_seq =
    	{
		.values.p_common = seq_values,
        	.length           = NRF_PWM_VALUES_LENGTH(seq_values),
        	.repeats          = 0,
        	.end_delay        = 0
    	};
	nrf_drv_pwm_simple_playback(&pwm_net, &pwm_seq, 1, NRF_DRV_PWM_FLAG_STOP);
}

void setcolor_power(uint32_t rgb)
{
	static uint16_t  seq_values[LED_RST_PERIOD + LED_CONTROL_PIONT*LED_NUMS] ={
        //320个周期的复位信号
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
        0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,0X8000,
    };	

	uint16_t led_buf[LED_CONTROL_PIONT];
	uint32_t temp = ((rgb<<8) &0x00ff0000) |((rgb>>8)&0x0000ff00)|(rgb&0x000000ff);

	for(uint8_t i=0 ; i<LED_CONTROL_PIONT ; i++)
	{
		if((temp<<i)&0x00800000)
		{
			led_buf[i] = 0X8000+LONG_PWM;
		}
		else
		{
			led_buf[i] = 0X8000+SHORT_PWM;
		}
	}
	for(uint8_t i=0 ; i<LED_NUMS ; i++)
	{
		memcpy(seq_values+LED_RST_PERIOD+i*LED_CONTROL_PIONT,led_buf,sizeof(led_buf));
	}
   	nrf_pwm_sequence_t const pwm_seq =
    	{
		.values.p_common = seq_values,
        	.length           = NRF_PWM_VALUES_LENGTH(seq_values),
        	.repeats          = 0,
        	.end_delay        = 0
    	};
	nrf_drv_pwm_simple_playback(&pwm_power, &pwm_seq, 1, NRF_DRV_PWM_FLAG_STOP);
}

void power_led_setcolor(uint32_t rgb)
{
    static uint32_t data = 0xFFFFFFFF;

    if (data != rgb)
    {
        data = rgb;
        setcolor_power(rgb);
    }
}

void net_led_setcolor(uint32_t rgb)
{
    static uint32_t data = 0xFFFFFFFF;

    if (data != rgb)
    {
        data = rgb;
        setcolor_net(rgb);
    }
}

void led_key_relay_init(void)
{
	nrf_gpio_cfg_output(RELAY_PIN);

	 // 打开LED电源开关
	nrf_gpio_cfg_output(LED_CTR_PIN);
	nrf_gpio_pin_clear(LED_CTR_PIN);
	nrf_delay_ms(800);
    nrf_gpio_pin_set(LED_CTR_PIN);

    // 配置电源灯
	nrf_gpio_cfg(
        POWER_LED_PIN,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_PULLDOWN,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_clear(POWER_LED_PIN);

    // 配置网络灯
	nrf_gpio_cfg(
        NET_LED_PIN,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_PULLDOWN,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_clear(NET_LED_PIN);

    nrf_drv_pwm_config_t const config =
    {
        .output_pins =
        {
            POWER_LED_PIN | NRF_DRV_PWM_PIN_INVERTED,   // channel 0
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED
        },
        .irq_priority = 2,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = LED_PWM_SIZE,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
 	APP_ERROR_CHECK(nrf_drv_pwm_init(&pwm_power, &config, pwm_interrupt_power));

    nrf_drv_pwm_config_t const config_net =
    {
        .output_pins =
        {
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED,
			NET_LED_PIN | NRF_DRV_PWM_PIN_INVERTED,
            NRF_DRV_PWM_PIN_NOT_USED
        },
        .irq_priority = 2,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = LED_PWM_SIZE,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    APP_ERROR_CHECK(nrf_drv_pwm_init(&pwm_net, &config_net, pwm_interrupt_net));

    power_led_setcolor(LED_OFF_VALUE);
    net_led_setcolor(LED_OFF_VALUE);
	nrf_delay_ms(200);
}

void set_power_led_struct(uint32_t on_rgb, uint16_t on_time, uint32_t off_rgb, uint16_t off_time, uint16_t cnt)
{
	power_led.led_switch = 1;
	power_led.state = 0;

	power_led.on_rgb = on_rgb;
	power_led.on_time = on_time;
	power_led.off_rgb = off_rgb;
	power_led.off_time = off_time;
	power_led.cnt = cnt;
}

void set_network_led_struct(uint32_t on_rgb, uint16_t on_time, uint32_t off_rgb, uint16_t off_time, uint16_t cnt)
{
	network_led.led_switch = 1;
	network_led.state = 0;

	network_led.on_rgb = on_rgb;
	network_led.on_time = on_time;
	network_led.off_rgb = off_rgb;
	network_led.off_time = off_time;
	network_led.cnt = cnt;
}

void network_led_process()
{
	static uint32_t timer;

	if(network_led.led_switch)
	{
		switch(network_led.state)
		{
			case 0:
				if(network_led.on_time == 0)
				{
					network_led.state = 2;
					break;
				}
	
				net_led_setcolor(network_led.on_rgb);

				timer = get_sys_time();
				network_led.state = 1;
				break;

			case 1:
				if(timepassed(timer, network_led.on_time))
				{	
					network_led.state = 2;
				}
				break;

			case 2:
				if(network_led.off_time == 0)
				{
					network_led.state = 4;
					break;
				}
				
				net_led_setcolor(network_led.off_rgb);

				timer = get_sys_time();
				network_led.state = 3;
				break;

			case 3:
				if(timepassed(timer, network_led.off_time))
				{
					network_led.state = 4;
				}
				break;

			case 4:
				if(network_led.cnt == 0xFFFF)
				{
					network_led.state = 0;
					break;
				}

				network_led.cnt--;
				
				if(network_led.cnt == 0)
				{
					network_led.led_switch = 0;
				}
				else
				{
					network_led.state = 0;
				}
				break;
				
			default:
				break;
		}
	}
	else
	{
		net_led_setcolor(LED_OFF_VALUE);
	}
}

void power_led_process()
{
	static uint32_t timer;

	if(power_led.led_switch)
	{
		switch(power_led.state)
		{
			case 0:
				if(power_led.on_time == 0)
				{
					power_led.state = 2;
					break;
				}
	
				power_led_setcolor(power_led.on_rgb);

				timer = get_sys_time();
				power_led.state = 1;
				break;

			case 1:
				if(timepassed(timer, power_led.on_time))
				{
					power_led.state = 2;
				}
				break;

			case 2:
				if(power_led.off_time == 0)
				{
					power_led.state = 4;
					break;
				}

				power_led_setcolor(power_led.off_rgb);

				timer = get_sys_time();
				power_led.state = 3;
				break;

			case 3:
				if(timepassed(timer, power_led.off_time))
				{
					power_led.state = 4;
				}
				break;

			case 4:
				if(power_led.cnt == 0xFFFF)
				{
					power_led.state = 0;
					break;
				}

				power_led.cnt--;
				
				if(power_led.cnt == 0)
				{
					power_led.led_switch = 0;
				}
				else
				{
					power_led.state = 0;
				}
				break;
				
			default:
				break;
		}
	}
	else
	{
		power_led_setcolor(LED_OFF_VALUE);
	}
}


void led_flash_process(void)
{
	network_led_process();
	power_led_process();
}

void set_led_state(uint8_t led_state)
{
	
	switch(led_state)
	{
		case LED_STATE_SERIAL:
			set_power_led_struct(BLUE_VALUE, 500, YELLOW_VALUE, 500, 0XFFFF);
			break;
		case LED_STATE_CALIBRATE_PROCESS:
			set_power_led_struct(PURPLE_VALUE, 500, LED_OFF_VALUE, 0, 0XFFFF);
			break;
		case LED_STATE_CALIBRATE_SUCCESS:
			set_power_led_struct(BLUE_VALUE, 500, LED_OFF_VALUE, 0, 0XFFFF);
			break;
		case LED_STATE_CALIBRATE_FALSE:	
			set_power_led_struct(RED_VALUE, 500, LED_OFF_VALUE, 0, 0XFFFF);
			break;
	}
}
#endif
