
#ifndef BSP_H__
#define BSP_H__

#include <stdint.h>
#include <stdbool.h>
#include "sem.h"
#include "SoftwareRTC.h"
#include "SEGGER_RTT.h"
#include "byte_fifo.h"




#define VERSION			1
#define PATCHLEVEL		0
#define SUBLEVEL		1





#define RESET_PIN						3
#define BAT_ADC							5
#define BAT_CHANNEL						NRF_SAADC_INPUT_AIN3
#define CHARGER_CHARGING_PIN			6
#define CHARGER_CONNECTED_PIN			7
#define ReadKey()						nrf_gpio_pin_read(KEY_PIN)
#define RFID_SEND_INTER_SEC				(5*2)
#define V_A								4200
#define V_B								294
#define ENABLE_BLE
#define SEC_2017						0x1ffb0300
#define SEC_2099						0xba37e000
#define ADV_QUICK_TIME					20











extern const uint8_t version[8];



void rtc_event_handler(void * val);
void GetTimeAndSec(  RTC_UTCTimeStruct *time1);
void get_timer(uint8_t*hour,uint8_t*min,uint8_t *sec);
void RtcTimeInit(void);



void LowerPower(void);

void FPU_IRQHandler(void);
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info);
void HardFault_Handler(void);












//==============================================================================================
#define USER_DEBUG 1
#if USER_DEBUG
#define USER_RTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define USER_RTT(...)
#endif

#ifdef FLASH_RTT_P
#define FLASH_RTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);
#else
#define FLASH_RTT(...)
#endif

//#define DEBUG_DEBUG_PRINTF
#ifdef DEBUG_DEBUG_PRINTF
#define LOG_DEBUG_PRINTF(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define LOG_DEBUG_PRINTF(...)
#endif

//#define BLE_RTT
#ifdef BLE_RTT
#define BLE_RTT_PRINTF(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define BLE_RTT_PRINTF(...)
#endif


//#define BLE_CONN_RTT
#ifdef BLE_CONN_RTT
#define BLE_C_RTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define BLE_C_RTT(...)
#endif

//#define UI_DEBUG
#ifdef UI_DEBUG
#define UI_DEBUG_PRINTF(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define UI_DEBUG_PRINTF(...)
#endif


//#define HR_DEBUG
#ifdef HR_DEBUG
#define HR_DEBUG_PRINTF(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define HR_DEBUG_PRINTF(...)
#endif


//#define ANCS_DEBUG
#ifdef ANCS_DEBUG
#define ANCS_DEBUG_PRINTF(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define ANCS_DEBUG_PRINTF(...)
#endif


//#define BSP_DEBUG
#ifdef BSP_DEBUG
#define BSP_DEBUG_PRINTF(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define BSP_DEBUG_PRINTF(...)
#endif








#endif





