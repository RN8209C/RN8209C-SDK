
#ifndef RN8209_U_H__
#define RN8209_U_H__

#include "string.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "math.h"
#define DEBUG_8209C

#ifdef DEBUG_8209C
#define rn8209c_debug(...)  printf(__VA_ARGS__)
#else
#define rn8209c_debug(...)
#endif


#define		REG_SYSCON 		0x00
#define        	REG_EMUCON 		0x01
#define        	REG_HFConst     	0x02
#define        	REG_PStart      		0x03
#define        	REG_DStart      		0x04
#define		REG_GPQA        	0x05  //Power correction A
#define        	REG_GPQB        	0x06  //Power correction B
#define        	REG_PhsA        		0x07
#define        	REG_PhsB        		0x08
#define		REG_QPHSCAL		0x09
#define		REG_APOSA 		0x0a
#define        	REG_APOSB 		0x0b
#define        	REG_RPOSA 		0x0c
#define        	REG_RPOSB 		0x0d
#define        	REG_IARMSOS     	0x0e
#define        	REG_IBRMSOS     	0x0f
#define        	REG_IBGain      	0x10
#define		REG_D2FPL       	0x11
#define        	REG_D2FPH       	0x12
#define        	REG_DCIAH       	0x13
#define        	REG_DCIBH       	0x14
#define         REG_DCUH			0x15
#define         REG_DCL   			0x16
#define         REG_EMUCON2		0x17
#define		REG_PFCnt    		0x20
#define        	REG_DFcnt    		0x21
#define        	REG_IARMS       	0x22  //current a
#define        	REG_IBRMS       	0x23  //current b
#define        	REG_URMS        	0x24   //vlotage
#define		REG_UFreq       	0x25
#define        	REG_PowerPA     	0x26
#define        	REG_PowerPB     	0x27
#define        	REG_PowerQ     	0x28
#define         REG_EnergyP  		0x29
#define         REG_EnergyP2 		0x2a
#define         REG_EnergyD  		0x2b
#define         REG_EnergyD2    	0x2c
#define         REG_EMUStatus   	0x2d	//???锟斤拷??锟斤拷??锟斤拷
#define         REG_SPL_IA      	0x30
#define         REG_SPL_IB      	0x31
#define         REG_SPL_U       	0x32
#define         REG_IE  			0x40
#define         REG_IF  			0x41
#define         REG_RIF    			0x42
#define         REG_SysStatus  	0x43
#define         REG_RData      		0x44
#define         REG_WData      		0x45
#define         REG_DeviceID   	0x7f
#define         REG_WriteEn   		0xea

//#define HFCONST  0x42e6

//#define KP        0.0000136867


#define VOLTAGE_REF  220
#define CURRENT_REF  5


#define CMD_WAIT_MS   40
#define RN8209_CMD_REPEAT_TIMES  3

#define phase_A   0
#define phase_B (1+phase_A)


typedef void (*rn8209c_delay_ms_fun)(int ms);
typedef void (*rn8209c_uart_tx_fun)(uint8_t *dataout,int len);
typedef int (*rn8209c_uart_rx_fun)(uint8_t *dataout,int len,int time_out);
typedef void (*rn8209c_tx_pin_reset_fun)(void);

typedef struct
{
    float power_start;  //启动功率
    uint32_t EC;  //脉冲常数
    float KV;  //分压系数
    uint32_t  R;  //电流电阻
    uint16_t GPQA;    //A通道功率增益校正-----------------
    uint16_t GPQB;    //B通道功率增益校正
    uint8_t  PhsA;    //A通道相位校正--------------------------
    uint8_t  PhsB;    //B通道相位校正
    uint16_t Cst_QPhsCal; //无功相位校正------------------------
    uint16_t APOSA;   //A通道有功功率offset校正--------------------
    uint16_t APOSB;   //B通道有功功率offset校正
    uint16_t RPOSA;   //A通道无功功率offset校正
    uint16_t RPOSB;   //B通道无功功率offset校正
    uint16_t IARMSOS; //A通道电流有效值offset校正------------------
    uint16_t IBRMSOS; //B通道电流有效值offset校正
    uint16_t IBGain;  //B通道电流增益

    uint32_t Ku;//电压比例-----------------
    uint32_t Kia;//--------------------
    uint32_t Kib;
    uint16_t deviation;//误差
}__attribute__((packed)) STU_8209C;

  

//extern STU_8209C Stu8209c;
bool rn8209c_init( rn8209c_delay_ms_fun delay,\
                  rn8209c_uart_tx_fun uart_tx,\
                  rn8209c_uart_rx_fun uart_rx,\
                  rn8209c_tx_pin_reset_fun tx_pin_reset,\
                  STU_8209C param);/*接口初始化*/


uint8_t rn8209c_init_para( );

uint8_t rn8209c_read_voltage(uint32_t *vol);/*扩大1000倍*/
uint8_t rn8209c_read_current(uint8_t phase,uint32_t *current);/* 扩大10000倍*/
uint8_t rn8209c_read_power(uint8_t phase,uint32_t *p);/*扩大10000倍*/
uint8_t rn8209c_read_power_energy(uint8_t phase,uint32_t *p); /*扩大1000倍*/
uint8_t rn8209c_read_power_Q(uint8_t phase,uint32_t *p);/*扩大10000*/


void rn8209c_calibrate_voltage_current(uint8_t phase,uint32_t voltage_real,uint32_t current_real );  /*1.0 将校表台电压设置220V,5A, 1.0功率校准*/
void rn8209c_calibrate_power_k(uint8_t phase,uint32_t ku,uint32_t ki);/*2.0 将校表台电压设置220V,5A, 1.0功率校准*/
void rn8209c_calibrate_phs(uint8_t phase,uint32_t power_ref);/* 3 .0 将校表台电压设置220V,5A，0.5L*/
void rn8209c_calibrate_power_offset(uint8_t phase,uint32_t power_ref);/* 4.0 5% Ib 功率Offset校正 将校表台电压设置220V,0.25A,1.0 */
void rn8209c_calibrate_power_Q(uint8_t phase,uint32_t power_q_ref);/*5.0 将校表台电压设置220V,16A, 0.5无功校准*/
void rn8209c_calibrate_current_offset(uint8_t phase) ;/*6.0 电流Offset校正将校表台电压设置220V,0A,1.0，只提供电压*/

STU_8209C read_stu8209c_calibrate_param();
void rn8209c_calibrate_power_k_phase_a();
void rn8209c_calibrate_power_k_phase_b();

#endif





















