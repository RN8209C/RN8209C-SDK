#include "system_time.h"


/********************************************************************/ // variable variable define 
// timer 
APP_TIMER_DEF(system_time_id);  




static uint32_t sys_time = 0 ;

/*******************************************************/
static void system_time_Handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	sys_time++;
}

void sysTime_Init(void)
{
	uint32_t err_code;
	 // Create timers.
    err_code = app_timer_create(&system_time_id,
                                APP_TIMER_MODE_REPEATED,
                                system_time_Handle);
	APP_ERROR_CHECK(err_code);
	    err_code = app_timer_start(system_time_id, APP_TIMER_TICKS(1) , NULL); // 1ms timer
    APP_ERROR_CHECK(err_code);
}

uint32_t get_sys_time(void)
{
	return sys_time;
}
uint8_t timepassed(uint32_t timer,uint32_t passed_ms)
{
	if((get_sys_time()-timer)>=passed_ms)
		return 1;
	else
		return 0;
}





















