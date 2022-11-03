#include "ws2812.h"
#include <unistd.h>
#include <stdint.h>
#include "driver/rmt.h"
#include "driver/gpio.h"

#define RGBPIN_SET(A)	gpio_set_level(RGB_LED_PIN,A)

#define BITS_PER_LED_CMD 	24 
#define LED_BUFFER_ITEMS 	( BITS_PER_LED_CMD* NUMLEDS)

#define RMT_CLK_DIV      		2    /*!< RMT counter clock divider */
#define DURATION	 		12.5 /* minimum time of a single RMT duration*/  //80MHZ  ʱ��Ƭ12.5ns 


// These values are determined by measuring pulse timing with logic analyzer and adjusting to match datasheet. 
#define T0H 	  		(350/(RMT_CLK_DIV*DURATION)) 
#define T1H 			(900/(RMT_CLK_DIV*DURATION))
#define T0L  			(900/(RMT_CLK_DIV*DURATION))
#define T1L  			(350/(RMT_CLK_DIV*DURATION))


// This is the buffer which the hw peripheral will access while pulsing the output pin
rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];
rmt_item32_t network_led_data_buffer[LED_BUFFER_ITEMS];

void ws2812_power_control()
{	
	RGBPIN_SET(0);
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = 1ULL<<WS2812_POWER_IO;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	gpio_set_level(WS2812_POWER_IO, 1);
	usleep(100000);
}

void RGB_Rst(void)
{
    RGBPIN_SET(0);
	usleep(320);
}



void ws2812_control_init()
{
	ws2812_power_control();

  	rmt_config_t power_led;
  	power_led.rmt_mode = RMT_MODE_TX;		    		//����
  	power_led.channel = POWER_LED_RMT_TX_CHANNEL;		//ͨ��
  	power_led.gpio_num = 27;							//�ܽ�
  	power_led.mem_block_num = 3;						//3
  	power_led.tx_config.loop_en = false;
  	power_led.tx_config.carrier_en = false;
  	power_led.tx_config.idle_output_en = true;
  	power_led.tx_config.idle_level = 0;
  	power_led.clk_div = RMT_CLK_DIV;					//ʱ�ӷ�Ƶ

  	ESP_ERROR_CHECK(rmt_config(&power_led));
  	ESP_ERROR_CHECK(rmt_driver_install(power_led.channel, 0, 0));

	rmt_config_t network_led;
  	network_led.rmt_mode = RMT_MODE_TX;		    		//����
  	network_led.channel = NETWORK_LED_RMT_TX_CHANNEL;	//ͨ��
  	network_led.gpio_num = RGB_LED_PIN;				    //�ܽ�
  	network_led.mem_block_num = 3;						//3
  	network_led.tx_config.loop_en = false;
  	network_led.tx_config.carrier_en = false;
  	network_led.tx_config.idle_output_en = true;
  	network_led.tx_config.idle_level = 0;
  	network_led.clk_div = RMT_CLK_DIV;					//ʱ�ӷ�Ƶ


  	ESP_ERROR_CHECK(rmt_config(&network_led));
  	ESP_ERROR_CHECK(rmt_driver_install(network_led.channel, 0, 0));
}



void setup_rmt_data_buffer(uint32_t rgb) 
{
  	for (uint32_t led = 0; led < NUMLEDS; led++) 
  	{
    		uint32_t bits_to_send = rgb;
   		uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
    		for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++) 
		{
      			uint32_t bit_is_set = bits_to_send & mask;
      			led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ?
                                                      (rmt_item32_t){{{T1H, 1, T1L, 0}}} : 
                                                      (rmt_item32_t){{{T0H, 1, T0L, 0}}};
      			mask >>= 1;
    		}
  	}
}

void ws2812_write_leds(uint32_t rgb) 
{
  	setup_rmt_data_buffer(rgb);
  	ESP_ERROR_CHECK(rmt_write_items(POWER_LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false));
  	ESP_ERROR_CHECK(rmt_wait_tx_done(POWER_LED_RMT_TX_CHANNEL, portMAX_DELAY));
}

void setcolor(uint32_t rgb)
{
	static uint32_t last_rgb = 0xFFFFFFFF;

	if(last_rgb == rgb)
	{
		return;
	}
	else
	{
		last_rgb = rgb;
	}
	
	rgb = (rgb<<8 &0x00ff0000)|(rgb>>8 &0x0000ff00)|(rgb&0x000000ff);
    RGB_Rst();
	ws2812_write_leds(rgb);
}

void setup_rmt_data_buffer_test(uint32_t *rgb)
{
  	for (uint32_t led = 0; led < NUMLEDS; led++) 
  	{
    		uint32_t bits_to_send = rgb[led];
   		uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
    		for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++) 
		{
      			uint32_t bit_is_set = bits_to_send & mask;
      			led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ?
                                                      (rmt_item32_t){{{T1H, 1, T1L, 0}}} : 
                                                      (rmt_item32_t){{{T0H, 1, T0L, 0}}};
      			mask >>= 1;
    		}
  	}
}
void ws2812_write_leds_test(uint32_t *rgb)
{
  	setup_rmt_data_buffer_test(rgb);
  	ESP_ERROR_CHECK(rmt_write_items(POWER_LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false));
  	ESP_ERROR_CHECK(rmt_wait_tx_done(POWER_LED_RMT_TX_CHANNEL, portMAX_DELAY));
}

void setcolor_test(uint32_t *rgb)
{
	for(uint8_t i=0 ; i<NUMLEDS  ; i++)
	{
		rgb[i] =  (rgb[i]<<8 &0x00ff0000)|(rgb[i]>>8 &0x0000ff00)|(rgb[i]&0x000000ff);
	}

    RGB_Rst();
	ws2812_write_leds_test(rgb);
}

void set_network_led_rmt_data_buffer(uint32_t rgb) 
{
	uint32_t led, bit;
	
	for (led = 0; led < NUMLEDS; led++) 
  	{
    	uint32_t bits_to_send = rgb;
   		uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);

    	for (bit = 0; bit < BITS_PER_LED_CMD; bit++) 
		{
      			uint32_t bit_is_set = bits_to_send & mask;
      			network_led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ?
									(rmt_item32_t){{{T1H, 1, T1L, 0}}} : 
									(rmt_item32_t){{{T0H, 1, T0L, 0}}};
      			mask >>= 1;
    		}
  	}
}

void set_network_led(uint32_t rgb)
{
	static uint32_t last_rgb = 0xFFFFFFFF;

	if(last_rgb == rgb)
	{
		return;
	}
	else
	{
		last_rgb = rgb;
	}

	rgb = (rgb<<8 &0x00ff0000)|(rgb>>8 &0x0000ff00)|(rgb&0x000000ff);

	gpio_set_level(27, 0);
	usleep(320);

	set_network_led_rmt_data_buffer(rgb);
  	ESP_ERROR_CHECK(rmt_write_items(NETWORK_LED_RMT_TX_CHANNEL, network_led_data_buffer, LED_BUFFER_ITEMS, false));
  	ESP_ERROR_CHECK(rmt_wait_tx_done(NETWORK_LED_RMT_TX_CHANNEL, portMAX_DELAY));
}
