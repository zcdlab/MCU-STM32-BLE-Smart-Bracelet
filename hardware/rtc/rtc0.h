#ifndef __RTC0_H
#define __RTC0_H
#include <stdbool.h>
#include "nrf.h"
#include "stdint.h"
typedef struct 
{
	int hour;
	int min;
	int sec;			
	//公历日月年周
	int w_year;
	int  w_month;
	int  w_date;
	int  week;		 
}tm;


void Auto_Time_Set(void);
void updata_rtc_timer(void);
#endif
