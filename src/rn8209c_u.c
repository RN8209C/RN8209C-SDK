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

#include "rn8209c_u.h"

static STU_8209C Stu8209c;

rn8209c_delay_ms_fun rn8209c_delay_ms;
rn8209c_uart_tx_fun rn8209c_uart_tx;
rn8209c_uart_rx_fun rn8209c_uart_rx;
rn8209c_tx_pin_reset_fun rn8209c_tx_pin_reset;
static uint16_t hfconst;
float  kp;

void get_user_param(STU_8209C user)
{
	Stu8209c = user;
	float temp;
	// 4为电压增益倍数 16为电流增益倍数
	if(Stu8209c.KV==0 && Stu8209c.R == 0)
		hfconst = 0x1000; //芯片复位值
	else
	{
		temp = 16.1079*Stu8209c.KV*Stu8209c.R*4*16*100000/Stu8209c.EC;
		hfconst = (uint16_t)temp;
	}
	rn8209c_debug("hfconst is %d\n",hfconst);
	if(Stu8209c.EC ==0)
		Stu8209c.EC = 3200;
	//  3.22155*10^12/(2^32) = 750
	temp = ((float)750)/(Stu8209c.EC*hfconst);
	kp = temp;
	rn8209c_debug(" param kp is %f\n",kp);
	rn8209c_debug(" param ku is %d\n",Stu8209c.Ku);
	rn8209c_debug(" param kia is %d\n",Stu8209c.Kia);
	
}
STU_8209C read_stu8209c_calibrate_param(void)
{
	return Stu8209c;
}
void rn8209c_calibrate_power_k_phase_a(void)
{
	 rn8209c_calibrate_power_k(phase_A,Stu8209c.Ku, Stu8209c.Kia);
}
void rn8209c_calibrate_power_k_phase_b(void)
{
	 rn8209c_calibrate_power_k(phase_B,Stu8209c.Ku, Stu8209c.Kib);
}

static void s_uart_write_data(uint8_t *datain,uint16_t len)
{
    uint8_t readbuf[50];
    rn8209c_uart_rx(readbuf,50,0);
    rn8209c_uart_tx(datain,len);
}



static uint8_t  s_rn8209c_write(unsigned char wReg,unsigned char *pBuf,unsigned char ucLen)
{
    uint8_t i;
    uint8_t j;
    uint8_t temp;
    uint8_t chksum;
    uint8_t Repeat;
    uint8_t	err;
    uint8_t buf[10];
    uint8_t buf_len=0;
    uint8_t buf_r[10];
    if( (ucLen == 0) || (ucLen > 4) )
    {
        rn8209c_debug( "line:%d-error:%d\n",__LINE__,1);
        return 1;
    }


    for( Repeat =RN8209_CMD_REPEAT_TIMES; Repeat != 0 ; Repeat--)
    {
        err = 0;

        temp =wReg|0x80;

        buf_len=0;
        buf[buf_len++] = temp;
        chksum = temp;

        for(i = 0; i <ucLen; i++ )
        {
            buf[buf_len++] = pBuf[i];
            chksum +=pBuf[i];

        }

        chksum = ~chksum;

        buf[buf_len++] = chksum;



        //uart_write_bytes(UART_NUM_0,(const char*)buf,buf_len);
        // RN8209C_WAIT();


        //   buf_len = 0;
        if(wReg == 0xEA)//RN8209写使能或写保护
        {
            buf[buf_len++] = 0X43;
        }
        else//读写入WData寄存器检查
        {
            buf[buf_len++] = wReg;
        }

        s_uart_write_data(buf,buf_len);
        rn8209c_uart_rx(buf_r,ucLen+1,CMD_WAIT_MS);


        j = 0;
        if(wReg == 0xEA)
        {
            if(pBuf[0] == 0XE5)
            {
                temp = buf_r[0];
                if(!(temp&0x10))
                {
                    err = 2;
                    rn8209c_delay_ms(100);
                    rn8209c_debug("line:%d-error:0x%02X  cmd:0x%x times:%d\n",__LINE__,temp,wReg,RN8209_CMD_REPEAT_TIMES-Repeat);

                }
            }
            else if(pBuf[0] == 0XDC)
            {
                temp = buf_r[0];
                if(temp&0x10)
                {
                    err = 3;
                    rn8209c_delay_ms(100);
                    rn8209c_debug( "line:%d-error:0x%02X times:%d\n",__LINE__,temp,RN8209_CMD_REPEAT_TIMES-Repeat);

                }
            }
        }
        else
        {

            for(i = 0; i < ucLen; i++)
            {

                temp = buf_r[j++];
                if((wReg == REG_SYSCON)&&(i==0)) temp = 0;   //REG_SYSCON high byte will change by uart param

                if(temp != pBuf[i])
                {
                    rn8209c_debug("line:%d-error:0x%02X times:%d\n",__LINE__,temp,RN8209_CMD_REPEAT_TIMES-Repeat);
                    err = 4;
                    rn8209c_delay_ms(100);
                    break;
                }
            }
        }
        if(err == 0)
        {
            //  rn8209c_debug("success  write  max times:%d cmd:0x%x\n",RN8209_CMD_REPEAT_TIMES-Repeat,wReg);
            return(err);
        }

    }
    rn8209c_debug( "error write  max times:%d cmd:0x%x\n",RN8209_CMD_REPEAT_TIMES-Repeat,wReg);
    return err;


}

static uint8_t s_rn8209c_read(unsigned char wReg,unsigned char *pBuf,unsigned char ucLen)
{
    unsigned char i;
    uint8_t temp;
    uint8_t Repeat;
    unsigned char j=0;
    unsigned char chksum=0;
    uint8_t	err;

    uint8_t buf[10];
    uint8_t buf_len=0;
    uint8_t buf_r[10];

    if(ucLen == 0) return(1);

    for( Repeat=RN8209_CMD_REPEAT_TIMES; Repeat != 0 ; Repeat--)
    {
        err  = 0;
        temp = wReg ;
        chksum=wReg;
        j = 0;

        buf_len=0;
        buf[buf_len++] = temp;

        s_uart_write_data(buf,buf_len);
        rn8209c_uart_rx(buf_r,ucLen+1,CMD_WAIT_MS);


        for(i = 0; i <ucLen; i++)
        {
            pBuf[i] = buf_r[j++];
            chksum += pBuf[i] ;
        }

        chksum = ~chksum;
        temp = buf_r[j++];

        if(temp!=chksum)
        {
            rn8209c_debug("error reREG_  checksum 2:%d-%d -cmd:0x%x times:%d\r\n",temp,chksum,wReg,RN8209_CMD_REPEAT_TIMES-Repeat);
            err = 2;
            //for(i = ucLen; i > 0; i--) pBuf[i-1] = 0;
            for(i = ucLen; i > 0; i--) pBuf[ucLen-i] = 0;
            rn8209c_delay_ms(100);
        }
        if(err == 0)
        {
            // rn8209c_debug("read cmd:0x%02X Repeat:%d\n",wReg,RN8209_CMD_REPEAT_TIMES-Repeat);
            return(err);
        }

    }

    rn8209c_debug("error reREG_  max times:%d\n",RN8209_CMD_REPEAT_TIMES-Repeat);
    return(err);
}


static uint8_t s_rn8209c_calc_crc(uint8_t *pcrc)
{

    uint8_t regbuf[2],ret=0;

    *pcrc=0;
    //系统控制寄存器
    ret|=s_rn8209c_read(REG_SYSCON,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //写HFCONST
    ret|=s_rn8209c_read(REG_HFConst,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //启动功率
    ret|=s_rn8209c_read(REG_PStart,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];
 

    ret|=s_rn8209c_read(REG_DStart,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //计量控制

    ret|=s_rn8209c_read(REG_EMUCON,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //计量控制2 为13.xxhz     13.xxhz --->00b0   3.xxhz--->0030
    ret|=s_rn8209c_read(REG_EMUCON2,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //A通道有功功率增益
    ret|=s_rn8209c_read(REG_GPQA,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //B通道有功功率增益
    ret|=s_rn8209c_read(REG_GPQB,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];


    //A通道相位校正
    ret|=s_rn8209c_read(REG_PhsA,regbuf,1);
    *pcrc+=regbuf[0];

    //B通道相位校正
    ret|=s_rn8209c_read(REG_PhsB,regbuf,1);
    *pcrc+=regbuf[0];

    //A通道有功功率offset
    ret|=s_rn8209c_read(REG_APOSA,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //B通道有功功率offset
    ret|=s_rn8209c_read(REG_APOSB,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //A通道无功功率offset

    ret|=s_rn8209c_read(REG_RPOSA,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //B通道无功功率offset
    ret|=s_rn8209c_read(REG_RPOSB,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //A通道电流offset
    ret|=s_rn8209c_read(REG_IARMSOS,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //B通道电流offset
    ret|=s_rn8209c_read(REG_IBRMSOS,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //B通道电流增益
    ret|=s_rn8209c_read(REG_IBGain,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    //直流通道
    ret|=s_rn8209c_read(REG_D2FPL,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    ret|=s_rn8209c_read(REG_D2FPH,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    ret|=s_rn8209c_read(REG_DCIAH,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    ret|=s_rn8209c_read(REG_DCIBH,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    ret|=s_rn8209c_read(REG_DCUH,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    ret|=s_rn8209c_read(REG_DCL,regbuf,2);
    *pcrc+=regbuf[0];
    *pcrc+=regbuf[1];

    return ret;

}

/********************************************************
功能描述：初始化芯片
参数说明：
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
uint8_t rn8209c_init_para(void )
{

    uint8_t regbuf[6];
    uint8_t ret=0;
    double dtemp;
    uint16_t  PSTART;
    dtemp=0.8*Stu8209c.power_start/(kp*256);
    PSTART=(uint16_t )dtemp;
//写使能
    regbuf[0] = 0xE5;
    ret|=s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_debug("ku is %d,ki is %d,kIB is %d\n",Stu8209c.Ku,Stu8209c.Kia,Stu8209c.Kib);
    rn8209c_debug("GPQA is %d,GPQB is %d\n",Stu8209c.GPQA,Stu8209c.GPQB);
    rn8209c_debug("PHSA is %d,PHSB is %d\n",Stu8209c.PhsA,Stu8209c.PhsB);
    rn8209c_debug("CST_QPHSCAL is %d\n",Stu8209c.Cst_QPhsCal);
    rn8209c_debug("APOSA is %d,APOSB is %d\n",Stu8209c.APOSA,Stu8209c.APOSB);
    rn8209c_debug("RPOSA is %d,RPOSB is %d\n",Stu8209c.RPOSA,Stu8209c.RPOSB);
    rn8209c_debug("IARMSOS is %d,IBRMSOS is %d\n",Stu8209c.IARMSOS,Stu8209c.IBRMSOS);
    rn8209c_debug("IBGain is %d\n",Stu8209c.IBGain);
    rn8209c_delay_ms(100);
    //系统控制寄存器
    regbuf[0] = 0x00;
    regbuf[1] = 0x0f;  //关闭通道b，a增益16 电压增益4
    ret|=s_rn8209c_write(REG_SYSCON,regbuf,2);
    //写HFCONST
    regbuf[0] = hfconst>>8;
    regbuf[1] = (uint8_t)hfconst;
    ret|=s_rn8209c_write(REG_HFConst,regbuf,2);
	
    //启动功率
    
    regbuf[0] = PSTART>>8;
    regbuf[1] =  (uint8_t)PSTART;
    ret|=s_rn8209c_write(REG_PStart,regbuf,2);
    regbuf[0] = PSTART>>8;
    regbuf[1] =  (uint8_t)PSTART;
    ret|=s_rn8209c_write(REG_DStart,regbuf,2);
	
    //计量控制
    regbuf[0] = 0x80;  //电能读后清零
    regbuf[1] = 0x03;
    ret|=s_rn8209c_write(REG_EMUCON,regbuf,2);
    //计量控制2 为13.xxhz     13.xxhz --->00b0   3.xxhz--->0030
    regbuf[0] = 0x00;
    regbuf[1] = 0xB0;
    ret|=s_rn8209c_write(REG_EMUCON2,regbuf,2);
    //A通道有功功率增益
    //StDef_RN8209DPara_Reg.Cst_GPQA = 0;
    regbuf[0] = Stu8209c.GPQA>>8;
    regbuf[1] = Stu8209c.GPQA;
    ret|=s_rn8209c_write(REG_GPQA,regbuf,2);
    //B通道有功功率增益
    //StDef_RN8209DPara_Reg.Cst_GPQB = 0;
    regbuf[0] = Stu8209c.GPQB>>8;
    regbuf[1] = Stu8209c.GPQB;
    ret|=s_rn8209c_write(REG_GPQB,regbuf,2);
    //A通道相位校正
    //StDef_RN8209DPara_Reg.Cst_PhsA = 0;
    regbuf[0] = Stu8209c.PhsA;
    ret|=s_rn8209c_write(REG_PhsA,regbuf,1);
    //B通道相位校正
    //StDef_RN8209DPara_Reg.Cst_PhsB = 0;
    regbuf[0] = Stu8209c.PhsB;
    ret|=s_rn8209c_write(REG_PhsB,regbuf,1);
    //A通道有功功率offset
    //StDef_RN8209DPara_Reg.Cst_APOSA = 0;
    regbuf[0] = Stu8209c.APOSA>>8;
    regbuf[1] = Stu8209c.APOSA;
    ret|=s_rn8209c_write(REG_APOSA,regbuf,2);
    //B通道有功功率offset
    //StDef_RN8209DPara_Reg.Cst_APOSB = 0;
    regbuf[0] = Stu8209c.APOSB>>8;
    regbuf[1] =  Stu8209c.APOSB;
    ret|=s_rn8209c_write(REG_APOSB,regbuf,2);
    //A通道无功功率offset
    Stu8209c.RPOSA = 0;
    regbuf[0] =  Stu8209c.RPOSA>>8;
    regbuf[1] =  Stu8209c.RPOSA;
    ret|=s_rn8209c_write(REG_RPOSA,regbuf,2);
    //B通道无功功率offset
    Stu8209c.RPOSB = 0;
    regbuf[0] =  Stu8209c.RPOSB>>8;
    regbuf[1] =  Stu8209c.RPOSB;
    ret|=s_rn8209c_write(REG_RPOSB,regbuf,2);
    //A通道电流offset
    //StDef_RN8209DPara_Reg.Cst_IARMSOS = 0;
    regbuf[0] = Stu8209c.IARMSOS>>8;
    regbuf[1] = Stu8209c.IARMSOS;
    ret|=s_rn8209c_write(REG_IARMSOS,regbuf,2);
    //B通道电流offset
    //StDef_RN8209DPara_Reg.Cst_IBRMSOS = 0;
    regbuf[0] = Stu8209c.IBRMSOS>>8;
    regbuf[1] =Stu8209c.IBRMSOS;
    ret|=s_rn8209c_write(REG_IBRMSOS,regbuf,2);
    //B通道电流增益
    //StDef_RN8209DPara_Reg.Cst_IBGain = 0;
    regbuf[0] = Stu8209c.IBGain>>8;
    regbuf[1] = Stu8209c.IBGain;
    ret|=s_rn8209c_write(REG_IBGain,regbuf,2);
/*
    regbuf[0] =  Stu8209c.Cst_QPhsCal>>8;
    regbuf[1] =  Stu8209c.Cst_QPhsCal;
     ret|=s_rn8209c_write(REG_QPHSCAL,regbuf,2);*/
    //直流通道
    regbuf[0]=0;
    regbuf[1]=0;
    ret|=s_rn8209c_write(REG_D2FPL,regbuf,2);
    regbuf[0]=0;
    regbuf[1]=0;
    ret|=s_rn8209c_write(REG_D2FPH,regbuf,2);
    regbuf[0]=0;
    regbuf[1]=0;
    ret|=s_rn8209c_write(REG_DCIAH,regbuf,2);
    regbuf[0]=0;
    regbuf[1]=0;
    ret|=s_rn8209c_write(REG_DCIBH,regbuf,2);
    regbuf[0]=0;
    regbuf[1]=0;
    ret|=s_rn8209c_write(REG_DCUH,regbuf,2);
    regbuf[0]=0;
    regbuf[1]=0;
    ret|=s_rn8209c_write(REG_DCL,regbuf,2);
    //写保护
    regbuf[0] = 0xDC;
    ret|=s_rn8209c_write(REG_WriteEn,regbuf,1);
    //获取校表寄存器校验值
    return ret;
}

/*
对于直接接入式电能表，启动电流为：0.004Ib
对于互感式接入的电能表，启动电流为：0.002In

*/
/********************************************************
功能描述：  初始化ic，初始化成功返回true，返回参数的crc
参数说明：  power_start : 启动功率默认4.4w()
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
bool rn8209c_init( rn8209c_delay_ms_fun delay,\
                  rn8209c_uart_tx_fun uart_tx,\
                  rn8209c_uart_rx_fun uart_rx,\
                  rn8209c_tx_pin_reset_fun tx_pin_reset,\
                  STU_8209C param)
{
    uint8_t regbuf[6];
    uint8_t i;
    uint8_t ret=0;
    uint8_t pcrc=0;

    rn8209c_delay_ms=delay;
    rn8209c_uart_tx=uart_tx;
    rn8209c_uart_rx=uart_rx;
    rn8209c_tx_pin_reset=tx_pin_reset;
    get_user_param(param);

    rn8209c_tx_pin_reset();

    //读取RN8209 DeviceID
    if(s_rn8209c_read(REG_DeviceID,regbuf,3) == 0)
    {
        if((regbuf[0]!=0x82)||(regbuf[1]!=0x09)||(regbuf[2]!=0x00))
        {
            rn8209c_debug("error id error:%02x%02x%02x \r\n",regbuf[0],regbuf[1],regbuf[2]);
            return false;
        }
    }
    else
    {
        rn8209c_debug( "error id read error:%02x%02x%02x \r\n",regbuf[0],regbuf[1],regbuf[2]);
        return false;
    }
    rn8209c_debug(" id success :%02x%02x%02x \r\n",regbuf[0],regbuf[1],regbuf[2]);
    for(i=0; i<3; i++)
    {
        ret=rn8209c_init_para();
        //   RN8209C_WAIT();

        if(0==ret)
        {

            if(0==s_rn8209c_calc_crc(&pcrc))
            {
                rn8209c_debug( "rn8209c_init write calc crc success:%d\n",pcrc);




                return true;
            }
            else
            {
                rn8209c_debug("rn8209c_init write calc crc err:%d\n",i);
            }
        }
        else
        {
            rn8209c_debug("rn8209c_init write err:%d\n",i);
        }
    }



    return false;
}





/*1.0 将校表台电压设置220V,5A, 1.0功率校准*/
/********************************************************
功能描述：   计算电压、电流显示转换系数
参数说明：   phase_A   phase_B
					校准仪器的电压电流:
					voltage_real 精确到1000倍(V)   current_real 精确到10000倍数(A)
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
void rn8209c_calibrate_voltage_current(uint8_t phase,uint32_t voltage_real,uint32_t current_real )
{
    uint8_t regbuf[3];
    //uint32_t tempValue;
    uint32_t regtemp[12];
    uint32_t regtemp_i[12];
    uint32_t regtotal=0,regtotal_i=0;
    uint8_t i = 0;
    uint8_t current_reg;
    double dtmp=0,dtmp1;

    current_reg=REG_IARMS;
    if(phase != phase_A)
    {
        current_reg=REG_IBRMS;
    }

    for(i=0; i<24; i++)
    {
        if(s_rn8209c_read(REG_URMS,regbuf,3)==0)
        {
            regtemp[i%12] = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
        }
        if(s_rn8209c_read(current_reg,regbuf,3)==0)
        {
            regtemp_i[i%12] = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
        }
        rn8209c_delay_ms(150);
    }

    //第一个数据不要
    for(i=2; i<12; i++)
    {
        regtotal += regtemp[i];
        regtotal_i += regtemp_i[i];
        // rn8209c_debug("i=%d,vol:%d current:%d \r\n",i,regtemp[i],regtemp_i[i]);
    }


    //Stu8209c.Ku= regtotal*1000/ (10*voltage_real);
    dtmp=regtotal;
    dtmp1=voltage_real;
    dtmp=dtmp*100/dtmp1;
    Stu8209c.Ku=dtmp;




    dtmp=regtotal_i;
    dtmp1=current_real;
    dtmp=dtmp*1000/dtmp1;
    //Stu8209c.Kia = regtotal_i*1000 / current_real;


    if(phase == phase_A)
        Stu8209c.Kia = dtmp;
    else
        Stu8209c.Kib = dtmp;

}




/*2.0 将校表台电压设置220V,5A, 1.0功率校准*/
/********************************************************
功能描述：   误差法校准功率增益
参数说明：
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
void rn8209c_calibrate_power_k(uint8_t phase,uint32_t ku,uint32_t ki)//
{
    const uint16_t regGPx[]= {REG_GPQA,REG_GPQB};
    // const uint16_t regArry[]= {REG_PowerPA,REG_PowerPB};
    uint8_t regbuf[5];
    double k = 0,err;
    uint16_t GPQx;

    err=kp*10000*ku *ki/32768-10000;
    //判断是否是负数
    if(err <0)
    {
        err = -err;
    }

    k = (-err/10000.0)/(1+err/10000.0);
    if(k > 0)
    {
        GPQx = (uint16_t)(k*32768);

    }
    else
    {
        GPQx = (uint16_t)(k*32768+65536);

    }
    //写使能
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    regbuf[0] = GPQx>>8;
    regbuf[1] = GPQx;
    s_rn8209c_write(regGPx[phase],regbuf,2);
    memset(regbuf,0x00,2);
    s_rn8209c_read(regGPx[phase],regbuf,2);
    if(phase == phase_A)
    {
        Stu8209c.GPQA = GPQx;
#if 0
        if(s_rn8209c_read(regArry[phase],regbuf,2)==0)
        {
            tempValue = regbuf[0]*256+regbuf[1];
            Stu8209c.Kpa= tempValue / 1100;
        }
#endif
    }
    else if(phase == phase_B)
    {
        Stu8209c.GPQB = GPQx;
#if 0
        if(s_rn8209c_read(regArry[phase],regbuf,2)==0)
        {
            tempValue = regbuf[0]*256+regbuf[1];
            Stu8209c.Kpb= tempValue / 1100;
        }
#endif
    }
}



/********************************************************
功能描述：
参数说明：1w 输入为10000
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
static int16_t rn8209c_calc_phs_err(uint8_t phase,uint32_t power_ref)
{
    /*
    err=preg*(1+gpqa)/pow_ref-1    //pow_ref 0.0001w
    转化为实际的单位
    err=10000*preg*(1+gpqa)/pow_ref-1

    //扩大10000倍数
    err=10000*10000*10000*preg*(1+gpqa)/pow_ref-10000


    */
    int16_t err=0;
    uint8_t regbuf[5],i=0;
    uint16_t gpqa;
    double dtemp,k;
    uint32_t regtemp[12],regtotal=0;
    const uint16_t regArry[]= {REG_PowerPA,REG_PowerPB};
    // const uint16_t regGPx[]= {REG_GPQA,REG_GPQB};
    gpqa=Stu8209c.GPQB;
    if(phase==phase_A)
        gpqa=Stu8209c.GPQA;
    for(i=0; i<24; i++)
    {
        if(s_rn8209c_read(regArry[phase],regbuf,4)==0)
        {
            regtemp[i%12] = (regbuf[0]<<24)+(regbuf[1]<<16)+(regbuf[2]<<8)+(regbuf[3]);
            //求补码
            if(regtemp[i%12]&0x80000000)
            {
                regtemp[i%12] = ~regtemp[i%12];
                regtemp[i%12] += 1;
            }
        }
        rn8209c_delay_ms(150);
    }

    //第一个数据不要
    for(i=1; i<12; i++)
    {
        regtotal += regtemp[i];
    }
    regtotal /= 11;

//floa
    k=gpqa;
    k=k/32768;
    if(gpqa&0x8000)
    {
        k=k-2;
    }

    dtemp=regtotal;
    //err=((1000000000000*kp*regtotal*(1+gpqa)/power_ref)-10000);
k=0;//客户的校准方法似乎是错的
    dtemp=dtemp*(kp*100000000)*(1+k)/power_ref - 10000;
    err=(int16_t)dtemp;
    return err;
}
/* 3 .0 将校表台电压设置220V,5A，0.5L*/
/********************************************************
功能描述：   误差法校准相位
参数说明：1w 输入为10000
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
void rn8209c_calibrate_phs(uint8_t phase,uint32_t power_ref)
{
    double k = 0;
    uint8_t phsValue = 0;
    const uint16_t regGPx[]= {REG_PhsA,REG_PhsB};
    uint8_t regbuf[2];
    int16_t err;

    while(0!=s_rn8209c_read(regGPx[phase],regbuf,1));
    if(0!=regbuf[0])
    {
        //写使能
        regbuf[0] = 0xE5;
        s_rn8209c_write(REG_WriteEn,regbuf,1);
        rn8209c_delay_ms(10);
        regbuf[0] = 0;
        s_rn8209c_write(regGPx[phase],regbuf,1);
    }
    err=rn8209c_calc_phs_err(phase,power_ref);
    k=-err;
    k = asin(k/10000.0/1.732)*180/3.142;

    if(k > 0)
    {
        k = (k/0.02);
    }
    else
    {
        k = (k/0.02+256);
    }

    phsValue=(uint8_t)k;
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_delay_ms(10);
    //写寄存器
    regbuf[0] = phsValue;
    s_rn8209c_write(regGPx[phase],regbuf,1);
    if(phase == phase_A)
    {
        Stu8209c.PhsA= phsValue;
    }
    else if(phase == phase_B)
    {
        Stu8209c.PhsB= phsValue;
    }
}

/* 4.0 5% Ib 功率Offset校正 将校表台电压设置220V,0.25A,1.0 */
/********************************************************
功能描述：   校准功率offset
参数说明： power_ref 标准表显示功率w*10000
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
void rn8209c_calibrate_power_offset(uint8_t phase,uint32_t power_ref)
{
    uint8_t regbuf[5];
    uint32_t regtemp[12],regtotal=0;
    const uint16_t regArry[]= {REG_PowerPA,REG_PowerPB};
    const uint16_t reg_APOSArry[]= {REG_APOSA,REG_APOSB};
    const uint16_t reg_GPQxArry[]= {REG_GPQA,REG_GPQB};
    uint8_t i = 0;
    uint16_t temp;
    double gGPQx = 0;
    double k = 0;
    double power_real=0,power_reg=0;
    //写使能
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_delay_ms(10);
    //写寄存器
    regbuf[0] =0;
    regbuf[1] = 0;
    s_rn8209c_write(reg_APOSArry[phase],regbuf,2);


    for(i=0; i<24; i++)
    {
        if(s_rn8209c_read(regArry[phase],regbuf,4)==0)
        {
            regtemp[i%12] = (regbuf[0]<<24)+(regbuf[1]<<16)+(regbuf[2]<<8)+(regbuf[3]);
            //求补码
            if(regtemp[i%12]&0x80000000)
            {
                regtemp[i%12] = ~regtemp[i%12];
                regtemp[i%12] += 1;
            }
        }
        rn8209c_delay_ms(100);
    }

    //第一个数据不要
    for(i=1; i<12; i++)
    {
        regtotal += regtemp[i];
    }
    regtotal /= 11;

    s_rn8209c_read(reg_GPQxArry[phase],regbuf,2);
    temp = regbuf[0]*256+regbuf[1];
    if(temp&0x8000)
    {
        gGPQx = (temp-65536)/32768.0;
    }
    else
    {
        gGPQx = temp/32768.0;
    }


    power_real=power_ref/(kp*10000);
    power_reg=regtotal;
    k = (power_real-power_reg)/(1+gGPQx);
    if(k > 0)
    {
        temp = (uint16_t)k;
    }
    else
    {
        temp = (uint16_t)(k+65536);
    }
    //写使能
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_delay_ms(10);
    //写寄存器
    regbuf[0] = temp>>8;
    regbuf[1] = temp;
    s_rn8209c_write(reg_APOSArry[phase],regbuf,2);
    if(phase == phase_A)
    {
        Stu8209c.APOSA = temp;
    }
    else if(phase == phase_B)
    {
        Stu8209c.APOSB= temp;
    }

}
/*5.0 将校表台电压设置220V,100A, 0.5无功校准*/
/********************************************************
功能描述：   误差法校准无功功率增益
参数说明：   power_q_ref 无功功率1w*10000
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
void rn8209c_calibrate_power_Q(uint8_t phase, uint32_t power_q_ref)
{

    //const uint16_t regGPx[]= {REG_GPQA,REG_GPQB};
    // const uint16_t regArry[]= {REG_PowerPA,REG_PowerPB};
    uint8_t i;
    uint32_t regtemp[12],regtotal=0;
    uint8_t regbuf[5];
    uint16_t Qphs;
//    uint16_t tempValue;
    double no_power,err;
    //写使能
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_delay_ms(10);
    regbuf[0] = 0;
    regbuf[1] = 0;
    s_rn8209c_write(REG_QPHSCAL,regbuf,2);
//    uint8_t phase=phase_A;
    //有功功率
    for(i=0; i<12; i++)
    {
        if(rn8209c_read_power_Q(phase,&regtemp[i])==0)
        {

        }
        rn8209c_delay_ms(150);
    }
    //第一个数据不要
    for(i=1; i<12; i++)
    {
        regtotal += regtemp[i];
    }
    no_power = regtotal;
    no_power /= 11;
    err=power_q_ref;
    err = no_power/err-1;

    err=err*0.5774;

    if(err > 0)
    {
        Qphs = (uint16_t)(err*32768);

    }
    else
    {
        Qphs = (uint16_t)(err*32768+65536);

    }
    Stu8209c.Cst_QPhsCal=Qphs;

    regbuf[0] = Qphs>>8;
    regbuf[1] = Qphs;
    s_rn8209c_write(REG_QPHSCAL,regbuf,2);
    rn8209c_delay_ms(10);
    memset(regbuf,0x00,2);
    s_rn8209c_read(REG_QPHSCAL,regbuf,2);

}


/*6.0 电流Offset校正将校表台电压设置220V,0A,1.0，只提供电压*/
/********************************************************
功能描述：   计算电流通道offset
参数说明：
返回说明：
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
void rn8209c_calibrate_current_offset(uint8_t phase)
{
    uint8_t regbuf[5];
    uint32_t regtemp[12],regtotal=0;
    const uint16_t regArry[]= {REG_IARMS,REG_IBRMS};
    const uint16_t regIx_OS[]= {REG_IARMSOS,REG_IBRMSOS};
    uint8_t i = 0;
    uint16_t temp;
    //写使能
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_delay_ms(100);
    //写寄存器
    regbuf[0] = 0;
    regbuf[1] = 0;
    s_rn8209c_write(regIx_OS[phase],regbuf,2);
    for(i=0; i<24; i++)
    {
        if(s_rn8209c_read(regArry[phase],regbuf,3)==0)
        {
            regtemp[i%12] = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
        }
        rn8209c_delay_ms(100);
    }

    //第一个数据不要
    for(i=1; i<12; i++)
    {
        regtotal += regtemp[i];
    }
    regtotal /= 11;
    regtotal = regtotal * regtotal;
    //求反码
    regtotal = ~regtotal;
    temp = (regtotal>>8);
    //符号位
    if(regtotal & 0x80000000)
        temp |= 0x8000;
    //写使能
    regbuf[0] = 0xE5;
    s_rn8209c_write(REG_WriteEn,regbuf,1);
    rn8209c_delay_ms(100);
    //写寄存器
    regbuf[0] = temp/256;
    regbuf[1] = temp%256;
    s_rn8209c_write(regIx_OS[phase],regbuf,2);
    if(phase == phase_A)
    {
        Stu8209c.IARMSOS = temp;
    }
    else if(phase == phase_B)
    {
        Stu8209c.IBRMSOS= temp;
    }
}
//read energy
uint8_t rn8209c_read_energy(uint32_t *energy)//
{
    uint8_t  regbuf[3];
    uint32_t tempValue;

    if(s_rn8209c_read(REG_EnergyP,regbuf,3)==0)
    {
        tempValue = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
	*energy = tempValue;
        return 0;
    }

    return 1;

}
/********************************************************
功能描述：
参数说明：
返回说明： 扩大1000倍
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
uint8_t rn8209c_read_voltage(uint32_t *vol)//
{
    uint8_t  regbuf[3];
    uint32_t tempValue;
    double dtemp;

    if(s_rn8209c_read(REG_URMS,regbuf,3)==0)
    {
        tempValue = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
        if(tempValue & 0x800000)
        {
            tempValue = 0;

        }
        else
        {

            dtemp=tempValue;

            *vol = (uint32_t)(dtemp*1000/Stu8209c.Ku);
        }
        return 0;
    }

    return 1;

}

/********************************************************
功能描述：
参数说明：
返回说明： 扩大10000倍
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
uint8_t rn8209c_read_current(uint8_t phase,uint32_t *current)
{
    uint8_t  regbuf[3],reg=REG_IARMS;
    uint32_t tempValue;
    double dtemp;
    if(phase != phase_A)
    {

        reg=REG_IBRMS;
    }
    if(s_rn8209c_read(reg,regbuf,3)==0)
    {
        tempValue = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
        if(tempValue & 0x800000)
        {
            tempValue = 0;

        }
        else
        {

            dtemp=tempValue;
            *current = (uint32_t)(dtemp*10000/Stu8209c.Kia);
            if(phase != phase_A)
            {

                *current = (uint32_t)(dtemp*10000/Stu8209c.Kib);
            }
        }
        return 0;
    }


    return 1;
}


/********************************************************
功能描述：   读取有功功率
参数说明：
返回说明：   扩大10000倍
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
uint8_t rn8209c_read_power(uint8_t phase,uint32_t *p)
{
    uint8_t  regbuf[4];
    uint32_t tempValue;
    double dtemp;
    uint8_t reg=REG_PowerPA;
    if(phase != phase_A)
        reg=REG_PowerPB;

    if(s_rn8209c_read(reg,regbuf,4)==0)
    {
        tempValue = (regbuf[0]<<24)+(regbuf[1]<<16)+(regbuf[2]<<8)+(regbuf[3]);
        if(tempValue&0x80000000)
        {
            tempValue = ~tempValue;
            tempValue += 1;
        }

        dtemp=tempValue;
        *p = (uint32_t)(dtemp*10000*kp);
        return 0;
    }

    return 1;
}


/********************************************************
功能描述：   读取累计电量
参数说明：
返回说明：   扩大100倍
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
uint8_t rn8209c_read_power_energy(uint8_t phase,uint32_t *p)
{
    uint8_t  regbuf[3];
    uint32_t tempValue;
//    double dtemp;
    uint8_t reg=REG_EnergyP;
    if(phase != phase_A)
        reg=REG_EnergyD;
    if(s_rn8209c_read(reg,regbuf,3)==0)
    {
        tempValue = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
        //*p = (uint32_t)(tempValue*100.0/1200);
        *p = tempValue;
        return 0;
    }

    return 1;
}
uint8_t rn8209c_read_emu_status()
{
	uint8_t  regbuf[3];
    	uint32_t tempValue;
//   double dtemp;
    	uint8_t reg=REG_EMUStatus;

    	if(s_rn8209c_read(reg,regbuf,3)==0)
    	{
        	tempValue = (regbuf[0]<<16)+(regbuf[1]<<8)+(regbuf[2]);
		if(tempValue &0x00020000)
			return 2;
		else
       		return 1;
    	}

    return 0;
}

/********************************************************
功能描述：   读取无功功率,默认为通道A
参数说明：
返回说明：   扩大10000倍
调用方式：
全局变量：
读写时间：
注意事项：
日期    ：
********************************************************/
uint8_t rn8209c_read_power_Q(uint8_t phase,uint32_t *p)
{


    uint8_t  regbuf[4];
    uint32_t tempValue;
    double dtemp;


    if(s_rn8209c_read(REG_PowerQ,regbuf,4)==0)
    {
        tempValue = (regbuf[0]<<24)+(regbuf[1]<<16)+(regbuf[2]<<8)+(regbuf[3]);
        if(tempValue&0x80000000)
        {
            tempValue = ~tempValue;
            tempValue += 1;
        }

        dtemp=tempValue;
        *p = (uint32_t)(dtemp*10000*kp);

        return 0;
    }


    return 1;
}













