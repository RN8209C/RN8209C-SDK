#ifndef __MK117__LED_H
#define __MK117__LED_H
#include "driver/gpio.h"
#include <stdint.h>
#include "ws2812.h"

#define RGB_LED_PIN2  		27
#define RGB_LED_PIN  		26
#define WS2812_POWER_IO	    16

#define RGBPIN_SET(A)	    \
 do{                        \
    gpio_set_level(RGB_LED_PIN,A); \
    gpio_set_level(RGB_LED_PIN2,A); \
 }while(0)
#define LED_WHITE_ON	    setcolor(WHITE_VALUE )
#define LED_BLUE_ON		    setcolor(BLUE_VALUE )
#define	LED_GREEN_ON		setcolor(GREEN_VALUE )
#define LED_YELLOW_ON		setcolor(YELLOW_VALUE )
#define LED_ORANGE_ON		setcolor(ORANGE_VALUE )
#define LED_RED_ON			setcolor(RED_VALUE )
#define LED_PURPLE_ON		setcolor(PURPLE_VALUE )

#define	LED_CYAN_ON		    setcolor(CYAN_VALUE )
#define	LED_ALL_OFF		    setcolor(LED_OFF_VALUE )


void mk117_led_init(void);
#endif
