/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 * 
 */

/** @file
 *
 * @defgroup ble_sdk_app_proximity_main main.c
 * @{
 * @ingroup ble_sdk_app_proximity_eval
 * @brief Proximity Application main file.
 *
 * This file contains is the source code for a sample proximity application using the
 * Immediate Alert, Link Loss and Tx Power services.
 *
 * This application would accept pairing requests from any peer device.
 *
 * It demonstrates the use of fast and slow advertising intervals.
 */

#include "ble_includes.h"
#define DEBUG_LOG

//global variable
bool mconnected = false;
bool oled_statu = false;
bool m_adv_statu = false;
uint8_t now_min = 0;

userprofile_struct_t user_profile;

HTU21D_Struct HTU;
bool htu21d_data_has_updata = false;

bool cry_sound_adc_flag = 0;
bool adc_convert_finish_flag = 1;
uint16_t  sound_frequency=0;				//哭声频率
uint16_t  sound_value=0;				

const uint8_t *COMPILED_DATE=__DATE__;//获得编译日期
const uint8_t *COMPILED_TIME=__TIME__;//获得编译时间
const uint8_t * Week[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
tm 	timer;

uint8_t mbatter_level = 100;
//global variable


/**@brief Function for application main entry.
 */
int main(void)
{

    // Initialize	
//		nrf_gpio_cfg_output(23);
//		nrf_gpio_pin_write(23, 0);
//		nrf_gpio_pin_write(23, 1);
//		nrf_delay_ms(1000);
//		nrf_gpio_pin_write(23, 0);
		uart_init();
    leds_init();
    timers_init();
    gpiote_init();
    buttons_init();
		app_button_enable();
		wdt_init();	
#ifdef  DEBUG_LOG
		printf("uart_init\r\n");
		printf("leds_init\r\n");
		printf("timers_init\r\n");
		printf("gpiote_init\r\n");
		printf("buttons_init\r\n");
		printf("wdt_init\r\n");
#endif	
	
    bond_manager_init();
    ble_stack_init();
    gap_params_init();
#ifdef  DEBUG_LOG
		printf("bond_manager_init\r\n");
		printf("ble_stack_init\r\n");
		printf("gap_params_init\r\n");
#endif


    advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
    services_init();
    conn_params_init();
    sec_params_init();	
    radio_notification_init();
#ifdef  DEBUG_LOG
		printf("advertising_init\r\n");
		printf("services_init\r\n");
		printf("conn_params_init\r\n");
		printf("radio_notification_init\r\n");
#endif

    advertising_start();
#ifdef  DEBUG_LOG
		printf("advertising_start\r\n");
#endif		
		
		
			
		HTU21D_Init();
		
#ifdef  DEBUG_LOG
		printf("HTU21D_Init\r\n");
		printf("OLED_Init\r\n");
#endif		
		
		my_timer_init();
		Auto_Time_Set();
		
#ifdef  DEBUG_LOG
		printf("rtc my_timer_init\r\n");
		printf("Auto_Time_Set\r\n");
#endif
	
		wdt_start();
#ifdef  DEBUG_LOG
		printf("wdt_start\r\n");
		printf("Enter main loop\r\n");		
#endif	

		OLED_POWER_CONTROL(1);
		nrf_delay_ms(2000);
		OLED_POWER_CONTROL(0);
    for (;;)
    {
			
			updata_by_min();
			updata_by_hour();
			power_manage();
    }
}

/** 
 * @}
 */
