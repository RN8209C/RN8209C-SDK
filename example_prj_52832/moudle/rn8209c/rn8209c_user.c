/*************************************************************************
*   	Copyright 2019-2021  MOKO TECHNOLOGY LTD
*
*	Licensed under the Apache License, Version 2.0 (the "License");   
*	you may not use this file except in compliance with the License.   
*	You may obtain a copy of the License at  
*
*	http://www.apache.org/licenses/LICENSE-2.0   
*
*	Unless required by applicable law or agreed to in writing, software   
*	distributed under the License is distributed on an "AS IS" BASIS,   
*	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   
*	See the License for the specific language governing permissions and   
*	limitations under the License.
**************************************************************************/

#include "rn8209c_user.h"
#include "simulate_uart.h"
#include "rn8209_flash.h"
#include "key_led_relay.h"
#include "cJSON.h"
#include "utility.h"
#include "simple_uart.h"
#include "nrf_delay.h"
#include "system_time.h"


static STU_8209C stu8209c_user;
struct rn8209 rn8209_value;
static char id[12+1];
static char type[16+1];
static char ver[16+1];

void set_user_param(STU_8209C param)
{
    stu8209c_user = param;
	memcpy(id,MAC,strlen(MAC)+1);
	memcpy(type,DEV_NAME,strlen(DEV_NAME)+1);
	memcpy(ver,FIRMWARE_VERSION,strlen(FIRMWARE_VERSION)+1);

	rn8209c_debug("the rn8209c SDK ver is %s\n",SDK_VER);
}
static void s_delay_ms(int ms)
{
      nrf_delay_ms(ms);
}

static void calibrate_success_deal( )
{
	stu8209c_flash.param = stu8209c_user;
    write_rn8209_param();
	set_led_state(LED_STATE_CALIBRATE_SUCCESS);
}
static void calibrate_false_deal( )
{
	set_led_state(LED_STATE_CALIBRATE_FALSE);
}
static void user_control()
{
	rn8209c_debug("user control\n");
	NVIC_SystemReset();
}

static void s_uart_tx(uint8_t *dataout,int len)
{
    simulate_uart_send(dataout,(uint16_t)len);
}

static int s_uart_rx(uint8_t *dataout,int len,int time_out)
{
	s_delay_ms(time_out);
    return simulate_uart_recv(dataout,(uint16_t)len);
}
static void s_rn8209c_uart_init(void)
{
	simulate_uart_uinit();
	simulate_uart_init(4800,PARITY_EVEN);
}
static void s_rn8209c_pin_restart(void)
{

	#define GPIO_TX_PIN current_pin.rn8209c_tx
	nrf_gpio_cfg_output(GPIO_TX_PIN);

    nrf_gpio_pin_clear(GPIO_TX_PIN);
    s_delay_ms(30);
    nrf_gpio_pin_set(GPIO_TX_PIN);
    s_rn8209c_uart_init();

}

static void cmd_uart_init(void)
{
	uart_init();

}
static void cmd_uart_tx(uint8_t *dataout,int len)
{
	uartSend(dataout,(uint16_t)len);
}

static int cmd_uart_rx(uint8_t *dataout,int len,int time_out)
{
	s_delay_ms(time_out);
   	 return uartrecv(dataout, (uint16_t )len);
}

bool init_8209c_interface(void)
{
//Stu8209c  �ṹ�帳ֵ
    return  rn8209c_init(s_delay_ms,\
                         s_uart_tx ,\
                         s_uart_rx,\
                         s_rn8209c_pin_restart,\
                         stu8209c_user);
}
void calibrate_start_deal()
{
	set_led_state(LED_STATE_CALIBRATE_PROCESS);
}
void relay_open()
{
	SET_RELAY_ON;
}

void read_rn8209()
{
	#define STATE_READ_VOLATAGE	0
	#define STATE_READ_EMU			1
	#define STATE_READ_CURRENT	2
	#define STATE_READ_POWER  		3
	#define STATE_READ_ENERGY		4
	#define STATE_TIME_OUT			5

	uint8_t uart_rx[25];
	uint8_t  rx_len = 0;
	static uint32_t timeout ;
	static uint8_t read_state = STATE_READ_VOLATAGE;
	static uint8_t state_mem ;
	static uint8_t check;
	uint32_t tempValue;
	double dtemp;
	uint8_t cmd;
	static uint8_t emu_ret=0;
	switch(read_state)
	{
		case STATE_READ_VOLATAGE:	
			cmd = REG_URMS;
           		state_mem = read_state;
			read_state = STATE_TIME_OUT;
			simulate_uart_send(&cmd,1);
			check = REG_URMS;
			timeout = get_sys_time();
			break;
		case STATE_READ_EMU:	
			cmd = REG_EMUStatus;
           		state_mem = read_state;
			read_state = STATE_TIME_OUT;
			simulate_uart_send(&cmd,1);
			check = REG_EMUStatus;
			timeout = get_sys_time();
			break;
		case STATE_READ_CURRENT:
			cmd = REG_IARMS;
           		state_mem = read_state;
			read_state = STATE_TIME_OUT;
			simulate_uart_send(&cmd,1);
			check = REG_IARMS;
			timeout = get_sys_time();
			break;
		case STATE_READ_POWER:
			cmd = REG_PowerPA;
           		state_mem = read_state;
			read_state = STATE_TIME_OUT;
			simulate_uart_send(&cmd,1);
			check = REG_PowerPA;
			timeout = get_sys_time();
			break;
		case STATE_READ_ENERGY:   
			cmd = REG_EnergyP;
			state_mem = read_state;
			read_state = STATE_TIME_OUT;
			simulate_uart_send(&cmd,1);
			check = REG_EnergyP;
			timeout = get_sys_time(); 
			break;
		case STATE_TIME_OUT:
			if(timepassed(timeout,40))
			{
				rx_len = simulate_uart_recv( uart_rx, 5 );
				if(rx_len && rx_len<=5)
				{
					for(uint8_t i=0 ; i<(rx_len-1) ; i++)
						check += uart_rx[i];
                  			check = ~ check;
					if(check == uart_rx[rx_len-1])
					{
						if(state_mem == STATE_READ_VOLATAGE)

						{
						        tempValue = (uart_rx[0]<<16)+(uart_rx[1]<<8)+(uart_rx[2]);
						        if(tempValue & 0x800000)
						        {
						            tempValue = 0;

						        }
						        else
						        {
						        	dtemp = tempValue;
								rn8209_value.voltage = (uint32_t)(dtemp*1000/stu8209c_user.Ku);
							//	rn8209c_debug("read voltage success,value is %d,temp is%d ,ku is %d\n",rn8209_value.voltage,tempValue,stu8209c_user.Ku);
						        }
						}
						else if(state_mem == STATE_READ_EMU)
						{
							 tempValue = (uart_rx[0]<<16)+(uart_rx[1]<<8)+(uart_rx[2]);
						        if(tempValue & 0x00020000)
						        {
						            	emu_ret = 2;
						        }
							 else
							 {
							  	emu_ret = 1;
							 }

						}
						else if(state_mem == STATE_READ_CURRENT)
						{
							 tempValue = (uart_rx[0]<<16)+(uart_rx[1]<<8)+(uart_rx[2]);
						        if(tempValue & 0x800000)
						        {
						            tempValue = 0;

						        }
						        else
						        {
						        	 dtemp  = tempValue;
								rn8209_value.current = (uint32_t)(dtemp*10000/stu8209c_user.Kia);
								if(emu_ret==2)
									rn8209_value.current = rn8209_value.current*(-1);
						        }
						}
						else if(state_mem == STATE_READ_POWER)
						{
							 tempValue = (uart_rx[0]<<24)+(uart_rx[1]<<16)+(uart_rx[2]<<8) +uart_rx[3];
						         if(tempValue&0x80000000)
						        {
						            tempValue = ~tempValue;
						            tempValue += 1;
						        }
							  dtemp = tempValue;
							  rn8209_value.power = (uint32_t)(dtemp*10000*kp);
							  if(emu_ret==2)
							  	rn8209_value.power = rn8209_value.power*(-1);
						}
						else if(state_mem == STATE_READ_ENERGY)
						{
							  tempValue = (uart_rx[0]<<16)+(uart_rx[1]<<8)+(uart_rx[2]);
						}
					}
				}
				read_state = state_mem +1;
				if(read_state == STATE_TIME_OUT)
					read_state = STATE_READ_VOLATAGE;
			}
			
			break;
	}
	
}
void rn8209_process(uint8_t cmd_mode)
{
	uint8_t uart_rxbuf[200];
	int buflen;
	static uint8_t mode=0;
	static uint8_t calibrate_step = 0;
	static uint32_t  ref_voltage =0;
	static uint32_t  ref_current =0;
	static uint32_t  ref_power =0;
	static uint8_t  init_flag = 0;
	static uint8_t 	init_result =0;
	if(cmd_mode)
	{
		
		buflen = cmd_uart_rx(uart_rxbuf,200,50);
		if(buflen)
		{
			cJSON *root = cJSON_Parse((char*)uart_rxbuf);
			if(root !=NULL)
			{
				cJSON *inquire = cJSON_GetObjectItem(root, "inquire");
				if(inquire !=NULL)
				{
					if(inquire->valueint == 1)
					{
						cJSON *resp = cJSON_CreateObject();
						cJSON_AddNumberToObject(resp,"inquire",1);
						cJSON_AddStringToObject(resp,"id",id);
						cJSON_AddStringToObject(resp,"type",type);
						cJSON_AddStringToObject(resp,"ver",ver);
						char *data_p = cJSON_PrintUnformatted(resp);
						if(data_p)
						{
							cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
							free(data_p);
						}
						cJSON_Delete(resp);
					}
					else if(inquire->valueint == 2)
					{
						cJSON *resp = cJSON_CreateObject();
						cJSON_AddNumberToObject(resp,"inquire",2);
						cJSON_AddNumberToObject(resp,"deviation",stu8209c_user.deviation);
						char *data_p = cJSON_PrintUnformatted(resp);
						if(data_p)
						{
							cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
							free(data_p);
						}
						cJSON_Delete(resp);
					}
					else if(inquire->valueint == 3)
					{
							//20210611 by ysh, read chip power after recv read cmd,to get current power info
						rn8209c_read_voltage(&rn8209_value.voltage);
						{
							uint8_t ret = rn8209c_read_emu_status();
							if(ret)
							{
								uint32_t temp_current=0 ;
								uint32_t temp_power=0 ;
								rn8209c_read_current(phase_A,&temp_current);
								rn8209c_read_power(phase_A,&temp_power);
								if(ret==1)
								{
									rn8209_value.current = temp_current;
									rn8209_value.power  = temp_power;
								}
								else
								{
									rn8209_value.current = (int32_t)temp_current*(-1);
									rn8209_value.power = (int32_t)temp_power *(-1);
								}
							}
						}
						cJSON *resp = cJSON_CreateObject();
						cJSON_AddNumberToObject(resp,"inquire",3);
						cJSON_AddNumberToObject(resp,"voltage",rn8209_value.voltage);
						cJSON_AddNumberToObject(resp,"current",rn8209_value.current);
						cJSON_AddNumberToObject(resp,"power",rn8209_value.power);
						char *data_p = cJSON_PrintUnformatted(resp);
						if(data_p)
						{
							cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
							free(data_p);
						}
						cJSON_Delete(resp);
					}
				}
				else
				{
					cJSON *set = cJSON_GetObjectItem(root,"set");
					if(set!=NULL)
					{
						if(set->valueint == 1)
						{
							cJSON *power_start = cJSON_GetObjectItem(root,"power_start");
							cJSON *EC = cJSON_GetObjectItem(root,"EC");
							cJSON *KV = cJSON_GetObjectItem(root,"KV");
							cJSON *R = cJSON_GetObjectItem(root,"R");

							cJSON *resp = cJSON_CreateObject();
							cJSON_AddNumberToObject(resp,"set",1);
							if(power_start!=NULL &&EC!=NULL && KV!=NULL && R!=NULL)
							{
								stu8209c_user.power_start = power_start->valuedouble;
								stu8209c_user.EC = EC->valueint;
								stu8209c_user.KV = KV->valuedouble;
								stu8209c_user.R = R->valueint;
								mode = 0;	
								init_flag = 1;
								calibrate_start_deal();
								init_result = 0;
								cJSON_AddStringToObject(resp,"resp","ack");	
							}
							else
								cJSON_AddStringToObject(resp,"resp","nack");
							char *data_p = cJSON_PrintUnformatted(resp);
							if(data_p)
							{
								cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
								free(data_p);
							}
							cJSON_Delete(resp);
						}
						else if(set->valueint == 2)
						{
							cJSON *step = cJSON_GetObjectItem(root,"step");
							cJSON *voltage = cJSON_GetObjectItem(root,"voltage");
							cJSON *current = cJSON_GetObjectItem(root,"current");
							cJSON *power = cJSON_GetObjectItem(root,"power");
							
							cJSON *resp = cJSON_CreateObject();
							cJSON_AddNumberToObject(resp,"set",2);
							if(step!=NULL  && voltage!=NULL && current !=NULL && power !=NULL && init_result)
							{
								calibrate_step = step->valueint;
								ref_voltage = voltage->valueint;
								ref_current = current->valueint;
								ref_power = power->valueint;
								mode = 1;
								cJSON_AddStringToObject(resp,"resp","ack");
							}
							else
								cJSON_AddStringToObject(resp,"resp","nack");
							char *data_p = cJSON_PrintUnformatted(resp);
							if(data_p)
							{
								cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
								free(data_p);
							}
							cJSON_Delete(resp);
						}
						else if(set->valueint == 3)
						{
							cJSON *result = cJSON_GetObjectItem(root,"result");
							cJSON *deviation = cJSON_GetObjectItem(root,"deviation");
							
							cJSON *resp = cJSON_CreateObject();
							cJSON_AddNumberToObject(resp,"set",3);
							if(result!=NULL && deviation!=NULL && result->valueint==1)
							{
								cJSON_AddStringToObject(resp,"resp","ack");
								stu8209c_user = read_stu8209c_calibrate_param();
								stu8209c_user.deviation = deviation->valueint;
								cJSON_AddNumberToObject(resp,"GPQA",stu8209c_user.GPQA);
								cJSON_AddNumberToObject(resp,"GPQB",stu8209c_user.GPQB);
								cJSON_AddNumberToObject(resp,"PhsA",stu8209c_user.PhsA);
								cJSON_AddNumberToObject(resp,"PhsB",stu8209c_user.PhsB);
								cJSON_AddNumberToObject(resp,"Cst_QPhsCal",stu8209c_user.Cst_QPhsCal);
								cJSON_AddNumberToObject(resp,"APOSA",stu8209c_user.APOSA);
								cJSON_AddNumberToObject(resp,"APOSB",stu8209c_user.APOSB);
								cJSON_AddNumberToObject(resp,"RPOSA",stu8209c_user.RPOSA);
								cJSON_AddNumberToObject(resp,"RPOSB",stu8209c_user.RPOSB);
								cJSON_AddNumberToObject(resp,"IARMSOS",stu8209c_user.IARMSOS);
								cJSON_AddNumberToObject(resp,"IBRMSOS",stu8209c_user.IBRMSOS);
								cJSON_AddNumberToObject(resp,"IBGain",stu8209c_user.IBGain);
								cJSON_AddNumberToObject(resp,"Ku",stu8209c_user.Ku);
								cJSON_AddNumberToObject(resp,"Kia",stu8209c_user.Kia);
								cJSON_AddNumberToObject(resp,"Kib",stu8209c_user.Kib);
								calibrate_success_deal();
							}
							else if(result!=NULL && result->valueint==0)
							{
								cJSON_AddStringToObject(resp,"resp","ack");
								stu8209c_user = read_stu8209c_calibrate_param();
								cJSON_AddNumberToObject(resp,"GPQA",stu8209c_user.GPQA);
								cJSON_AddNumberToObject(resp,"GPQB",stu8209c_user.GPQB);
								cJSON_AddNumberToObject(resp,"PhsA",stu8209c_user.PhsA);
								cJSON_AddNumberToObject(resp,"PhsB",stu8209c_user.PhsB);
								cJSON_AddNumberToObject(resp,"Cst_QPhsCal",stu8209c_user.Cst_QPhsCal);
								cJSON_AddNumberToObject(resp,"APOSA",stu8209c_user.APOSA);
								cJSON_AddNumberToObject(resp,"APOSB",stu8209c_user.APOSB);
								cJSON_AddNumberToObject(resp,"RPOSA",stu8209c_user.RPOSA);
								cJSON_AddNumberToObject(resp,"RPOSB",stu8209c_user.RPOSB);
								cJSON_AddNumberToObject(resp,"IARMSOS",stu8209c_user.IARMSOS);
								cJSON_AddNumberToObject(resp,"IBRMSOS",stu8209c_user.IBRMSOS);
								cJSON_AddNumberToObject(resp,"IBGain",stu8209c_user.IBGain);
								cJSON_AddNumberToObject(resp,"Ku",stu8209c_user.Ku);
								cJSON_AddNumberToObject(resp,"Kia",stu8209c_user.Kia);
								cJSON_AddNumberToObject(resp,"Kib",stu8209c_user.Kib);
								calibrate_false_deal();
							}
							else
								cJSON_AddStringToObject(resp,"resp","nack");
							char *data_p = cJSON_PrintUnformatted(resp);
							if(data_p)
							{
								cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
								free(data_p);
							}
							cJSON_Delete(resp);
						}
						else if(set->valueint == 4)
						{
							cJSON *resp = cJSON_CreateObject();
							cJSON_AddNumberToObject(resp,"set",4);
							cJSON_AddStringToObject(resp,"resp","ack");
							char *data_p = cJSON_PrintUnformatted(resp);
							if(data_p)
							{
								cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
								free(data_p);
							}
							cJSON_Delete(resp);
							user_control();
						}
					}
				}
				cJSON_Delete(root);
			}
			memset(uart_rxbuf,0,buflen);
		}
	}

	switch(mode)
	{
		case 0: //init
			if(init_8209c_interface()==1)
			{
				rn8209c_debug("rn8209 init success\n");
				mode = 2;
				if(init_flag)
				{
					USER_RTT("INIT SUCCESS\n");
					init_result = 1;
					init_flag = 0;
					cJSON *resp = cJSON_CreateObject();
					cJSON_AddNumberToObject(resp,"init_result",1);
					char *data_p = cJSON_PrintUnformatted(resp);
					if(data_p)
					{
						cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
						free(data_p);
					}
					cJSON_Delete(resp);
				}
			}
			else
			{
				mode =3;
				rn8209c_debug("rn8209 init false\n");
				if(init_flag)
				{
					cJSON *resp = cJSON_CreateObject();
					cJSON_AddNumberToObject(resp,"init_result",0);
					char *data_p = cJSON_PrintUnformatted(resp);
					if(data_p)
					{
						cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
						free(data_p);
					}
					cJSON_Delete(resp);
				}
			}
			break;
		case 1:  //calibrate
			{
				if(calibrate_step==1)
				{
					rn8209c_calibrate_voltage_current(phase_A,ref_voltage,ref_current);
				//	rn8209c_calibrate_power_k_phase_a();
				}
				else if(calibrate_step==2)
				{
					rn8209c_calibrate_power_k_phase_a();
				}
				else if(calibrate_step==3)
				{
					rn8209c_calibrate_phs(phase_A,ref_power);
				}
				else if(calibrate_step==4)
				{
					rn8209c_calibrate_power_offset(phase_A,ref_power);
				}
				else if(calibrate_step==5)
				{
					rn8209c_calibrate_power_Q(phase_A,ref_power);
				}
				else if(calibrate_step==6)
				{
					rn8209c_calibrate_current_offset(phase_A);
				}
				rn8209c_read_voltage(&rn8209_value.voltage);
	    			rn8209c_read_power(phase_A,(uint32_t*)&rn8209_value.power);
	    			rn8209c_read_current(phase_A,(uint32_t*)&rn8209_value.current);
				cJSON *resp = cJSON_CreateObject();
				cJSON_AddNumberToObject(resp,"set",2);	
				cJSON_AddNumberToObject(resp,"step",calibrate_step);
				cJSON_AddNumberToObject(resp,"voltage",rn8209_value.voltage);
				cJSON_AddNumberToObject(resp,"current",rn8209_value.current);
				cJSON_AddNumberToObject(resp,"power",rn8209_value.power);
				char *data_p = cJSON_PrintUnformatted(resp);
				if(data_p)
				{
					cmd_uart_tx((uint8_t *)data_p,strlen(data_p));
					free(data_p);
				}
				cJSON_Delete(resp);
				mode = 2;
			}
			break;
		case 2:
			if(cmd_mode)
			{
				rn8209c_read_voltage(&rn8209_value.voltage);
	    		//	rn8209c_read_power(phase_A,&rn8209_value.power);
	    		//	rn8209c_read_current(phase_A,&rn8209_value.current);
				uint8_t ret = rn8209c_read_emu_status();
				if(ret)
				{
					uint32_t temp_current=0 ;
					uint32_t temp_power=0 ;
					rn8209c_read_current(phase_A,&temp_current);
					rn8209c_read_power(phase_A,&temp_power);
					if(ret==1)
					{
						rn8209_value.current = temp_current;
						rn8209_value.power  = temp_power;
					}
					else
					{
						rn8209_value.current = (int32_t)temp_current*(-1);
						rn8209_value.power = (int32_t)temp_power *(-1);
					}
				}
			}
			//when not running in calibrate ,use no block  to avoid other func 
			else  
			{
				read_rn8209();
			}
			break;
		case 3:
			break;
	}
}
void rn8209_user_init(uint8_t cmd_mode)
{
	static uint8_t state =0 ;
	switch(state)
	{
		case 0:
			state = 1;
			read_rn8209_param();
			if(cmd_mode)
			{
				relay_open(); 
				cmd_uart_init();
			}
			break;
		case 1:
			rn8209_process(cmd_mode);
			break;
	}
}

