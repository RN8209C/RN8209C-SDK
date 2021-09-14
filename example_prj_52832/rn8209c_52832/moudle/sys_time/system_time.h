#ifndef __SYSTEM__TIME__H
#define __SYSTEM__TIME__H

#include<stdint.h>
#include "app_timer.h"










void sysTime_Init(void);
uint32_t get_sys_time(void);
uint8_t timepassed(uint32_t timer,uint32_t passed_ms);


#endif

