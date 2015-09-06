#include "communication_procotol.h"
#include "ble_nus.h"
#include "oled.h"
#include "rtc0.h"
#include "stdio.h"

extern tm 	timer;
extern ble_nus_t  m_nus;
extern userprofile_struct_t user_profile;

void send_temp_rh_data(uint8_t command, float value)
{
	uint8_t send_data[5];
	uint8_t value_len = 0x02;
	send_data[0] = command;
	send_data[1] = NACK|N_ACK_PACKET|value_len;
	send_data[2] = (uint8_t)value;
	send_data[3] = (uint8_t)(value*10)%10;
	send_data[4] = 0x55;
	ble_nus_send_string(&m_nus, send_data,ARRAY_LEN(send_data));

}

void send_cry_signal(uint8_t command, uint8_t value)
{
	uint8_t send_data[4];
	uint8_t value_len = 0x01;
	send_data[0] = command;
	send_data[1] = NACK|N_ACK_PACKET|value_len;
	send_data[2] = (uint8_t)value;
	send_data[3] = 0x55;
	ble_nus_send_string(&m_nus, send_data,ARRAY_LEN(send_data));

}

void receive_handle( uint8_t* data)
{
	uint8_t i = 0;
	uint8_t re_data[20];
	uint8_t command = data[0];
	uint8_t data_len = data[1]&GET_DATA_LEN;
	uint8_t last_byte = data[data_len+2];
	if(last_byte ==0x55)  //表示全部字节接收完成，否则丢弃
	{
		for(i=0;i<data_len;i++)
			re_data[i] = data[i+2];
		
		if(command==SET_DEVICE_TIME)
		{ 
			timer.w_year = re_data[0]<<8 | re_data[1];
			timer.w_month = re_data[2];
			timer.w_date = re_data[3];
			timer.hour = re_data[4];
			timer.min = re_data[5];
			timer.sec = re_data[6];
			
			OLED_ShowNum(0,32,(uint32_t)timer.w_year,4,16);
			OLED_ShowNum(40,32,(uint32_t)timer.w_month,2,16);
			OLED_ShowNum(64,32,(uint32_t)timer.w_date,2,16);
			OLED_ShowNum(88,32,(uint32_t)timer.hour,2,16);
			OLED_ShowNum(112,32,(uint32_t)timer.min,2,16);
			OLED_Refresh_Gram();

		}
		if(command==BOND_COMMAND)
		{
			for(i=0;i<data_len;i++)
			{
				user_profile.boundphone[i] = re_data[i];	
			}
			user_profile.boundphone[PHONE_LEN -1] = 0 ;
			printf("boundphone:%s\r\n",(char*)user_profile.boundphone);		
			
		}
	}
}
