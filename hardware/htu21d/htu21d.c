#include "htu21d.h"
#include "twi_master.h"
#include "math.h"
#include "stdio.h"
#include "nrf_delay.h"
#include "ble_nus.h"
#include "stdbool.h"

void HTU21D_Init(void)
{
	uint8_t command[1];
	uint8_t value;
	twi_master_init();
	
	Soft_Reset();
	
	nrf_delay_ms(20);
	
	command[0] = Read_User_Register();

	
	value = command[0] | 0x01;
	Write_User_Register(value);
	

	Updata_HTU();
}

uint8_t Read_User_Register(void)
{
	uint8_t command[1];
	
	command[0] = HTU21D_READ_COMMAND;
	twi_master_transfer(HTU21D_WRITE_ADDRESS, command, 1 , TWI_DONT_ISSUE_STOP);
	twi_master_transfer(HTU21D_READ_ADDRESS, command, 1 , TWI_ISSUE_STOP);
	return command[0];
}

void Write_User_Register(uint8_t value)
{
	uint8_t command[2];
	command[0] = HTU21D_WRITE_COMMAND;
	command[1] = value;
	twi_master_transfer(HTU21D_WRITE_ADDRESS, command, 2 , TWI_ISSUE_STOP);
}


void Soft_Reset(void)
{
	uint8_t command[1];
	command[0] = HTU21D_SOFT_RESET;
	twi_master_transfer(HTU21D_WRITE_ADDRESS, command, 1 , TWI_ISSUE_STOP);
}
uint8_t HTU21D_Check_CRC(uint16_t meaasge_from_sensor, uint8_t check_value_from_sensor)
{
	uint8_t i;
	uint32_t remainder = (uint32_t)meaasge_from_sensor<<8;
	uint32_t divsor = (uint32_t)SHIFTED_DIVISOR;
	remainder |= check_value_from_sensor;
	for(i=0;i<16;i++)
	{
		if(remainder&(uint32_t)1<<(23-i))	
			remainder ^= divsor;
		divsor >>= 1; 	
	}
	return (uint8_t)remainder;
}

float Read_Temperature(void)
{
	uint8_t Tem_Reg[3];
	uint16_t Temp = 0;
	float T;
	
	twi_master_issue_startcondition();
	twi_master_clock_byte(HTU21D_WRITE_ADDRESS);
	twi_master_clock_byte(HTU21D_TRIGGER_TEMPERATURE_MEASURE);
	
	twi_master_issue_startcondition();
	twi_master_clock_byte(HTU21D_READ_ADDRESS);
	twi_master_wait_while_scl_low();
	twi_master_clock_byte_in(Tem_Reg, false);
  twi_master_issue_stopcondition();
	
	Temp = Tem_Reg[0]<<8 | Tem_Reg[1];
	if( HTU21D_Check_CRC(Temp,Tem_Reg[2]) )
	{
		Temp &= 0xfffc;
		T = TK*Temp/POW2_16 + TBASE;
		return T;
	}
	else
	{
		return 0;
	}
	
}


float Read_RH(void)
{
	uint8_t RH_Reg[3];
	uint16_t RH_Temp = 0;
	float RH;
	
	twi_master_issue_startcondition();
	twi_master_clock_byte(HTU21D_WRITE_ADDRESS);
	twi_master_clock_byte(HTU21D_TRIGGER_RH_MEASURE);
	
	twi_master_issue_startcondition();
	twi_master_clock_byte(HTU21D_READ_ADDRESS);
	twi_master_wait_while_scl_low();
	twi_master_clock_byte_in(RH_Reg, false);
  twi_master_issue_stopcondition();

	RH_Temp = RH_Reg[0]<<8 | RH_Reg[1];
	if( HTU21D_Check_CRC(RH_Temp,RH_Reg[2]) )
	{
		RH_Temp &= 0xfff0;
		RH = RHK*RH_Temp/POW2_16 + RHBASE;
		return RH;
	}
	else
	{
		return 0;
	}	
	
}


bool Updata_HTU(void)
{
	float temp,rh;
//	temp=Read_Temperature();
//	rh=Read_RH();
	if( (temp=Read_Temperature()) !=0&&(rh=Read_RH()) != 0)
	{
		HTU.T = temp;
		HTU.RH = rh;
		return true;
	}
	else
	{
		return false;
	}
}



