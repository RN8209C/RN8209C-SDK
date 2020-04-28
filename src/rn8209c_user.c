#include "rn8209c_user.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "rn8209_flash.h"
#include "led.h"
#include "cJSON.h"


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

	rn8209c_debug("the ver is ------------%s\n",ver);
}

/*********************************芯片驱动接口函数*************************************/
static void s_delay_ms(int ms)
{
      vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void s_uart_tx(uint8_t *dataout,int len)
{
    uart_write_bytes(UART_NUM_2, (const char *)dataout, (size_t) len);

}

static int s_uart_rx(uint8_t *dataout,int len,int time_out)
{
    return uart_read_bytes(UART_NUM_2, dataout, (uint32_t )len, time_out/portTICK_PERIOD_MS);
}
static void s_rn8209c_uart_init(void)
{
    uart_config_t uart_config =
    {
        .baud_rate = 4800,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

 //   uart_driver_delete(UART_NUM_2);

    uart_param_config(UART_NUM_2, &uart_config);

    uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM_2, 256, 0, 0, NULL,0);

}
static void s_rn8209c_pin_restart(void)
{

#define GPIO_TX_PIN 17
#define GPIO_TX_PIN_SEL  ( 1ULL<<GPIO_TX_PIN)

    uart_driver_delete(UART_NUM_2);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_TX_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);


    gpio_set_level(GPIO_TX_PIN, 0);
    s_delay_ms(30);
    gpio_set_level(GPIO_TX_PIN, 1);

    s_rn8209c_uart_init();

}
/*************************************芯片驱动接口函数***********************************************/


/**************************************校准相关函数*************************************/
static void calibrate_success_deal( )
{
	stu8209c_flash.param = stu8209c_user;
    	write_rn8209_param();
	LED_ALL_OFF;
	LED_BLUE_ON;
   //  rn8209c_debug("ku is %d,ki is %d,gpqa is %d,phsa id %d\n",stu8209c_user.param.Ku,stu8209c_user.param.Kia,stu8209c_user.param.GPQA,stu8209c_user.param.PhsA);
}
static void calibrate_false_deal( )
{
	LED_ALL_OFF;
	LED_RED_ON;
}
static void user_control()
{
	rn8209c_debug("user control\n");
}
void calibrate_start_deal()
{
	LED_ALL_OFF;
	LED_PURPLE_ON;
}
void relay_open()
{
	
	#define RN8209_RELAY_PIN	25
	gpio_config_t io_conf;
    	io_conf.intr_type = GPIO_INTR_DISABLE;
    	io_conf.mode = GPIO_MODE_OUTPUT;
    	io_conf.pin_bit_mask = 1UL<<RN8209_RELAY_PIN;
    	io_conf.pull_down_en = 0;
    	io_conf.pull_up_en = 1;
    
    	gpio_config(&io_conf);
	gpio_set_level(RN8209_RELAY_PIN,1);
}

static void cmd_uart_init(void)
{
    uart_config_t uart_config =
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 256, 256, 0, NULL,0);
	rn8209c_debug("uart_init over\n");
}
static void cmd_uart_tx(uint8_t *dataout,int len)
{
    uart_write_bytes(UART_NUM_0, (const char *)dataout, (size_t) len);
}

static int cmd_uart_rx(uint8_t *dataout,int len,int time_out)
{
    return uart_read_bytes(UART_NUM_0, dataout, (uint32_t )len, time_out/portTICK_PERIOD_MS);
}
/**************************************校准相关函数*************************************/



bool init_8209c_interface()
{
//Stu8209c  结构体赋值
    return  rn8209c_init(s_delay_ms,\
                         s_uart_tx ,\
                         s_uart_rx,\
                         s_rn8209c_pin_restart,\
                         stu8209c_user);
}



void rn8209_process(uint8_t cmd_mode)
{
	uint8_t uart_rxbuf[200];
	//uint8_t uart_txbuf[200];
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
		case 0: //初始化参数
			if(init_8209c_interface()==1)
			{
				mode = 2;
				if(init_flag)
				{
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
		case 1:  //校准步骤
			if(calibrate_step==1)
			{
				rn8209c_calibrate_voltage_current(phase_A,ref_voltage,ref_current);
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
    			rn8209c_read_power(phase_A,&rn8209_value.power);
    			rn8209c_read_current(phase_A,&rn8209_value.current);
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
			break;
		case 2:
			rn8209c_read_voltage(&rn8209_value.voltage);
    			rn8209c_read_power(phase_A,&rn8209_value.power);
    			rn8209c_read_current(phase_A,&rn8209_value.current);
			break;
		case 3:
			break;
	}
	s_delay_ms(10);
}
void rn8209_user_init(uint8_t cmd_mode)
{
	
	if(cmd_mode)
	{
		relay_open(); 
		cmd_uart_init();
	}
	while(1)
	{
		rn8209_process(cmd_mode);
	}
}	
void rn8209c_process_init(uint8_t cmd)
{
	read_rn8209_param();
	xTaskCreate(rn8209_user_init, "rn8209c process", RN8209_TASK_STACK_SIZE,cmd, RN8209_TASK_PRIO, NULL);
}