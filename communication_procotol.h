#ifndef __COMMUNICATION_PROCOTOL_H
#define __COMMUNICATION_PROCOTOL_H
#include <stdint.h>
#include <stdbool.h>
#include <nrf51.h>

/*蓝牙通信信协议
*   1byte		1byte		Nbyte   lastbyte
*    key		 key     key 			0x55
*           header  value
*header: ack or nack:1bit(最高位), is ack packet:1bit, keyval size:6bit
*/

/*短信通信协议 JASON格式
*	{
*		lat:23.59267,
*   lon:119.3569,
*		cellId:8E17
*		areacode:24E1
*		temp:21.5	
*   rh:63.2 
*	  sos:0
* }
*/

#define ARRAY_LEN(a)            (sizeof(a)/sizeof(a[0]))          /* calculate array length */

#define ACK                     1<<7
#define NACK                    0<<7
#define ACK_PACKET              1<<6
#define N_ACK_PACKET            0<<6
#define GET_DATA_LEN						0x3F


#define BOND_COMMAND 						0X01
#define SET_LONGITUDE_COMMAND 	0X02
#define SET_LATITUDE_COMMAND 		0X03
#define SET_CELLIDHEX_COMMAND 	0X04
#define SET_AREACODEHEX_COMMAND 0X05
#define SET_PARENT_NUM_COMMAND 	0X06

#define SET_DEVICE_TIME					0x08

#define TEMPERATURE_COMMAND 		0X80
#define RH_COMMAND							0X81
#define SOS_COMMAND 						0X82
#define CELLIDHEX_COMMAND 			0X83
#define AREACODEHEX_COMMAND 		0X84
#define SIM_FEE_COMMAND					0X85
#define LATITUDE_COMMAND 				0X86
#define LONGITUDE_COMMAND 			0X87
#define CRY_COMMAND 						0X88


#define PHONE_LEN			13
typedef struct {
    uint8_t boundphone[PHONE_LEN];
} userprofile_struct_t;


void send_temp_rh_data(uint8_t command, float value);
void receive_handle( uint8_t* data);
void send_cry_signal(uint8_t command, uint8_t value);
#endif
