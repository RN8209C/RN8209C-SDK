#include "rn8209c_user.h"
#include "led.h"


void app_main(void)
{
	user_led_init();  // led init
	rn8209c_process_init(1); //calibrate mode
}




