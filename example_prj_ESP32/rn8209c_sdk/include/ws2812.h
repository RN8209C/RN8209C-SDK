#ifndef WS2812_52832_H____
#define WS2812_52832_H____
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LED_OFF_VALUE	    0x000000

#define WHITE_VALUE		    0XFFFFFF//	0xf0f8ff
#define WHITE_BLUE_1		0X3399FF//0x87cefa
#define WHITE_BLUE_2		0X0099FF//0X6495ED
#define BLUE_VALUE			0X0000FF
#define BLUE_GREEN_1		0X00FF99//0X009E96
#define BLUE_GREEN_2		0X00FF33//0X00FFFF
#define GREEN_VALUE		    0X00FF00
#define GREEN_YELLOW_1		0X66FF00//0X22AC38
#define GREEN_YELLOW_2		0XCCFF00//0X99FF00
#define YELLOW_VALUE		0XFFFF00
#define YELLOW_ORANGE_1	    0XFFCC00//0XFCC800
#define YELLOW_ORANGE_2	    0XFF9900//0XFF9900
#define ORANGE_VALUE		0XFF8000
#define ORANGE_RED_1		0XFF6600//0XEB6100
#define ORANGE_RED_2		0XFF3300//0XFF4500
#define RED_VALUE			0XFF0000//0XFF0000
#define RED_PURPLE_1		0XFF0033//0XFF00FF
#define RED_PURPLE_2		0XFF0099//0XC71585
#define PURPLE_VALUE		0X800080//0XA020F0
#define DARK_PURPLE_VALUE	0X4B0082
#define CYAN_VALUE		    0X00FFFF

void setcolor(uint32_t rgb);
void ws2812_control_init( );// Update the LEDs to the new state. Call as needed.// This function will block the current task until the RMT peripheral is finished sending // the entire sequence.
#endif




