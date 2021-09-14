#include "bsp.h"

#include "app_button.h"
#include "nrf_soc.h"
#include "SoftwareRTC.h"
#include "app_timer.h"
#include "simple_uart.h"
#include "nrf_delay.h"


extern void inter_call_sport(uint8_t swc);
extern uint8_t Rotation;






uint32_t error_code1;
uint8_t tmp_sec;
uint8_t min;








#define FPU_EXCEPTION_MASK 0x0000009F

void FPU_IRQHandler(void)
{
    uint32_t *fpscr = (uint32_t *)(FPU->FPCAR+0x40);
    (void)__get_FPSCR();

    *fpscr = *fpscr & ~(FPU_EXCEPTION_MASK);
}
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    error_code1=error_code;
    nrf_delay_ms(100);
    nrf_delay_ms(50);
    nrf_gpio_cfg_output(RESET_PIN);
    nrf_gpio_pin_set(RESET_PIN);
    nrf_delay_ms(2000);
    NVIC_SystemReset();
}
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    LOG_DEBUG_PRINTF("fault err:%x-%x-%x\r\n",id,pc,info);
    app_error_handler(0,0,0);
}
void HardFault_Handler(void)
{
    LOG_DEBUG_PRINTF("hardfault\r\n");
    app_error_handler(0,0,0);
}

