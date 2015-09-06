#ifndef __HTU21D_H
#define __HTU21D_H
#include "stdint.h"
#include "stdbool.h"

#define HTU21D_READ_ADDRESS 0x81    
#define HTU21D_WRITE_ADDRESS 0x80

#define HTU21D_WRITE_COMMAND 0xe6
#define HTU21D_READ_COMMAND	0xe7

#define HTU21D_TRIGGER_TEMPERATURE_MEASURE 0xe3
#define HTU21D_TRIGGER_RH_MEASURE 0xe5
#define HTU21D_SOFT_RESET 0xfe


#define TBASE  -46.85
#define TK     175.72
#define RHBASE -6
#define RHK		125
#define POW2_16 65536

#define SHIFTED_DIVISOR 0x988000

typedef struct{
	float T;
	float RH;
} HTU21D_Struct;


extern HTU21D_Struct HTU;
uint8_t HTU21D_Check_CRC(uint16_t meaasge_from_sensor, uint8_t check_value_from_sensor);
uint8_t Read_User_Register(void);
void Write_User_Register(uint8_t value);
void Soft_Reset(void);
float Read_Temperature(void);
float Read_RH(void);
void HTU21D_Init(void); 
bool Updata_HTU(void);
float Calc_Dewpoint(float RH, float T);
#endif
