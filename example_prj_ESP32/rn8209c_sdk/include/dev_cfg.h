#ifndef __DEV_CFG_H__
#define __DEV_CFG_H__

#define MK115
// #define MK114
// #define MK117

#if defined MK114
    #include "mk114_led.h"
    #define 	DEV_NAME				"MK114"
    #define     LED_INIT()              mk114_led_init()
    
#elif defined MK115
    #include "mk115_led.h"
    #define 	DEV_NAME				"MK115"
    #define     LED_INIT()              mk115_led_init()
#elif defined MK117
    #include "mk117_led.h"
    #define 	DEV_NAME				"MK117"
    #define     LED_INIT()              mk117_led_init()
#endif


#endif
