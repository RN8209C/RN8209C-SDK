#ifndef __MK115__LED_H
#define __MK115__LED_H

#include <stdint.h>
#include "ws2812.h"
#include "driver/gpio.h"

#define RGB_LED_PIN  		26
#define WS2812_POWER_IO	    16

#define RGBPIN_SET(A)	    \
 do{                        \
    gpio_set_level(RGB_LED_PIN,A); \
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

void mk115_led_init(void);
#endif
