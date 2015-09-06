#include "rtc0.h"
#include "nrf_gpio.h"
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
		{ 
		if(year%100==0) 
			{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
			}else return 1;   
		}else return 0;	
}	

const uint8_t table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
const uint8_t mon_table_not_leap_year[12]={31,28,31,30,31,30,31,31,30,31,30,31};
const uint8_t mon_table_leap_year[12]={31,29,31,30,31,30,31,31,30,31,30,31};

//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
} 


//比较两个字符串指定长度的内容是否相等
//参数:s1,s2要比较的两个字符串;len,比较长度
//返回值:1,相等;0,不相等
uint8_t str_cmpx(uint8_t*s1,uint8_t*s2,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)if((*s1++)!=*s2++)return 0;
	return 1;	   
}

extern tm timer;
extern const uint8_t *COMPILED_DATE;//获得编译日期
extern const uint8_t *COMPILED_TIME;//获得编译时间
const uint8_t Month_Tab[12][3]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"}; 
void Auto_Time_Set(void)
{
	uint8_t temp[3];
	uint8_t i;
	for(i=0;i<3;i++)temp[i]=COMPILED_DATE[i];   
	for(i=0;i<12;i++)if(str_cmpx((uint8_t*)Month_Tab[i],temp,3))break;	
	timer.w_month=i+1;//得到月份
	if(COMPILED_DATE[4]==' ')timer.w_date=COMPILED_DATE[5]-'0'; 
	else timer.w_date=10*(COMPILED_DATE[4]-'0')+COMPILED_DATE[5]-'0';  
	timer.w_year=1000*(COMPILED_DATE[7]-'0')+100*(COMPILED_DATE[8]-'0')+10*(COMPILED_DATE[9]-'0')+COMPILED_DATE[10]-'0';	   
	timer.hour=10*(COMPILED_TIME[0]-'0')+COMPILED_TIME[1]-'0';  
	timer.min=10*(COMPILED_TIME[3]-'0')+COMPILED_TIME[4]-'0';  
	timer.sec=10*(COMPILED_TIME[6]-'0')+COMPILED_TIME[7]-'0';  
} 

void updata_rtc_timer(void)
{
		if(timer.sec>=60)
		{
			timer.sec = 0;
			timer.min++;
			if(timer.min>=60)
			{
				timer.min = 0;
				timer.hour++;
				if(Is_Leap_Year(timer.w_year))
				{
					if(timer.hour>=24)
					{
						timer.hour = 0;
						timer.w_date++;
						if(timer.w_date>=mon_table_leap_year[timer.w_month])
						{
							timer.w_date = 0;
							timer.w_month++;
							if(timer.w_month>=12)
							{
								timer.w_month = 0;
								timer.w_year++;
							}
						}
					}
				}
				else
				{
					if(timer.hour>=24)
					{
						timer.hour = 0;
						timer.w_date++;
						if(timer.w_date>=mon_table_not_leap_year[timer.w_month])
						{
							timer.w_date = 0;
							timer.w_month++;
							if(timer.w_month>=12)
							{
								timer.w_month = 0;
								timer.w_year++;
							}
						}
					}
				}
			}
		}
}
