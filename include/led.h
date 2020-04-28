#ifndef __LED__RELAY_H
#define __LED__RELAY_H
#include<stdint.h>


  
#define LED_BLUE_PIN	    	14   
#define LED_GREEN_PIN	    	27     
#define LED_RED_PIN	   		26     

#define GPIO_OUTPUT_PIN_LED  ((1ULL << LED_BLUE_PIN) | (1ULL << LED_GREEN_PIN) | (1ULL << LED_RED_PIN) )


#define LED_STATE_OFF			0x01   	
#define LED_STATE_ON			0x00	


#define  	LED_BLUE_ON		gpio_set_level(LED_BLUE_PIN, LED_STATE_ON)
#define  	LED_BLUE_OFF		gpio_set_level(LED_BLUE_PIN, LED_STATE_OFF)
#define	LED_GREEN_ON		gpio_set_level(LED_GREEN_PIN, LED_STATE_ON)
#define	LED_GREEN_OFF		gpio_set_level(LED_GREEN_PIN, LED_STATE_OFF)
#define 	LED_RED_ON		gpio_set_level(LED_RED_PIN, LED_STATE_ON)
#define 	LED_RED_OFF		gpio_set_level(LED_RED_PIN, LED_STATE_OFF)

#define LED_ALL_OFF			LED_RED_OFF;LED_GREEN_OFF;LED_BLUE_OFF

#define LED_YELLOW_ON		LED_RED_ON;LED_GREEN_ON
#define LED_YELLOW_OFF		LED_RED_OFF;LED_GREEN_OFF

#define 	LED_PURPLE_ON		LED_RED_ON;LED_BLUE_ON
#define 	LED_PURPLE_OFF	LED_RED_OFF;LED_BLUE_OFF







void   user_led_init(void);



#endif
