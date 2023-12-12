
#include "utility.h"
#include "app_timer.h"
#include "bsp.h"
#include "byte_fifo.h"
#include "simple_uart.h"
#include "key_led_relay.h"
#include "simulate_uart.h"
#include "system_time.h"
#include "rn8209c_user.h"
#include "hardware_config.h"

void Open32768(void)
{
    uint32_t CNT_Temp = 0;
    NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    // Wait for the external oscillator to start up.
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        if(CNT_Temp++ >= (16*1000*100))/*100ms*/
            break;
    }
}
int main(void)
{
    hardware_choose_init();
	app_timer_init();
	sysTime_Init();
	Open32768();
	led_key_relay_init();
	
	set_led_state(LED_STATE_SERIAL);
	while(1)
	{
		rn8209_user_init(1);	
		led_flash_process();
	}
}


