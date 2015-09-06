#include "ble_app_main.h"


//global variable
extern bool mconnected ;
extern bool oled_statu ;
extern bool m_adv_statu;
extern uint8_t now_min;

extern userprofile_struct_t user_profile;

extern HTU21D_Struct HTU;
extern bool htu21d_data_has_updata;

extern bool cry_sound_adc_flag ;
extern bool adc_convert_finish_flag;
extern uint16_t  sound_frequency;				//哭声频率
extern uint16_t  sound_value;				

extern tm 	timer;

extern uint8_t mbatter_level;
//global variable


static uint16_t                         	m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
ble_nus_t                        					m_nus;
static app_timer_id_t                     m_rtc_timer_timer_id;                         /**< Timer to realize the blinking of Advertisement LED. */
static app_timer_id_t                     m_oled_timer_id;                    /**< Timer to realize the blinking of Advertisement LED. */
static app_timer_id_t                     m_cry_sound_timer_timer_id;                    /**< Timer to realize the blinking of Advertisement LED. */



/**@brief Advertisement states. */
typedef enum
{
    BLE_NO_ADV,                                                                             /**< No advertising running. */
    BLE_FAST_ADV_WHITELIST,                                                                 /**< Advertising with whitelist. */
    BLE_FAST_ADV,                                                                           /**< Fast advertising running. */
    BLE_SLOW_ADV,                                                                           /**< Slow advertising running. */
    BLE_SLEEP                                                                               /**< Go to system-off. */
} ble_advertising_mode_t;

static ble_tps_t                          m_tps;                                            /**< Structure used to identify the TX Power service. */
static ble_ias_t                          m_ias;                                            /**< Structure used to identify the Immediate Alert service. */
static ble_lls_t                          m_lls;                                            /**< Structure used to identify the Link Loss service. */
static bool                               m_is_link_loss_alerting;                          /**< Variable to indicate if a link loss has been detected. */
static bool                               m_is_alert_led_blinking;                          /**< Variable to indicate if the alert LED is blinking (indicating a mild alert). */
static bool                               m_is_adv_led_blinking;                            /**< Variable to indicate if the advertising LED is blinking. */

static ble_bas_t                          m_bas;                                            /**< Structure used to identify the battery service. */
static ble_ias_c_t                        m_ias_c;                                          /**< Structure used to identify the client to the Immediate Alert Service at peer. */
static ble_gap_sec_params_t               m_sec_params;                                     /**< Security requirements for this application. */
static ble_advertising_mode_t             m_advertising_mode;                               /**< Variable to keep track of when we are advertising. */

static volatile bool                      m_is_high_alert_signalled;                        /**< Variable to indicate whether or not high alert is signalled to the peer. */

static app_timer_id_t                     m_battery_timer_id;                               /**< Battery measurement timer. */
static app_timer_id_t                     m_alert_led_blink_timer_id;                       /**< Timer to realize the blinking of Alert LED. */
static app_timer_id_t                     m_adv_led_blink_timer_id;                         /**< Timer to realize the blinking of Advertisement LED. */



/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 * @retval     Result converted to millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 255) * ADC_PRE_SCALING_COMPENSATION)


/**@brief Function for error handling, which is called when an error has occurred. 
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    // This call can be used for debug purposes during development of an application.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Un-comment the line below to use.
    // ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover with a reset.
//    NVIC_SystemReset();

#ifdef 	DEBUG_LOG
		printf("app_error_handler:%d,line_num%d,p_file_name:%s\r\n",error_code,line_num,p_file_name);
#endif
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
		app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
void service_error_handler(uint32_t nrf_error)
{
		APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the ADC interrupt.
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void ADC_IRQHandler(void)
{
   
		if (NRF_ADC->EVENTS_END != 0 && cry_sound_adc_flag == 0)
    {
        uint8_t     adc_result;
        uint16_t    batt_lvl_in_milli_volts;
        uint8_t     percentage_batt_lvl;
        uint32_t    err_code;

        NRF_ADC->EVENTS_END     = 0;
        adc_result              = NRF_ADC->RESULT;
        NRF_ADC->TASKS_STOP     = 1;

        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) +
                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;
        percentage_batt_lvl     = battery_level_in_percent(batt_lvl_in_milli_volts);
			
//insert by su     		
				mbatter_level	 = percentage_batt_lvl;
#ifdef  DEBUG_LOG
			printf("mbatter_level:%d\r\n",mbatter_level);
#endif					 			
        err_code = ble_bas_battery_level_update(&m_bas, percentage_batt_lvl);
        if (
            (err_code != NRF_SUCCESS)
            &&
            (err_code != NRF_ERROR_INVALID_STATE)
            &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS)
            &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        )
        {
						APP_ERROR_HANDLER(err_code);
        }
    }
		
		if (NRF_ADC->EVENTS_END != 0 && cry_sound_adc_flag == 1)
    {
        uint8_t     adc_result;
        uint16_t    cry_sound_in_milli_volts;
        NRF_ADC->EVENTS_END     = 0;
        adc_result              = NRF_ADC->RESULT;
        NRF_ADC->TASKS_STOP     = 1;
        cry_sound_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result);
				if (cry_sound_in_milli_volts>50)		
				{
					sound_frequency++;
				}
#ifdef  DEBUG_LOG
// 				printf("%d,\r\n",cry_sound_in_milli_volts);
#endif	
    }	
		adc_convert_finish_flag = 1;
}


/**@brief Function for making the ADC start a battery level conversion.
 */
void adc_start(void)
{
    uint32_t err_code;

    // Configure ADC
    NRF_ADC->INTENSET   = ADC_INTENSET_END_Msk;
    NRF_ADC->CONFIG     = (ADC_CONFIG_RES_8bit                             << ADC_CONFIG_RES_Pos)     |
                          (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
                          (ADC_CONFIG_REFSEL_VBG                           << ADC_CONFIG_REFSEL_Pos)  |
                          (ADC_CONFIG_PSEL_AnalogInput2                    << ADC_CONFIG_PSEL_Pos)    |
                          (ADC_CONFIG_EXTREFSEL_None                       << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled;

    // Enable ADC interrupt
    err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_EnableIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
    NRF_ADC->TASKS_START = 1;
		cry_sound_adc_flag = 0;
		adc_convert_finish_flag = 0;
}



/**@brief Function for making the ADC start a battery level conversion.
 */
void cry_sound_adc_start(void)
{
    uint32_t err_code;

    // Configure ADC
    NRF_ADC->INTENSET   = ADC_INTENSET_END_Msk;
    NRF_ADC->CONFIG     = (ADC_CONFIG_RES_8bit                        		 << ADC_CONFIG_RES_Pos)     |
                          (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
                          (ADC_CONFIG_REFSEL_VBG                           << ADC_CONFIG_REFSEL_Pos)  |
                          (ADC_CONFIG_PSEL_AnalogInput3                    << ADC_CONFIG_PSEL_Pos)    |
                          (ADC_CONFIG_EXTREFSEL_None                       << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled;

    // Enable ADC interrupt
    err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_EnableIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
    NRF_ADC->TASKS_START = 1;
		cry_sound_adc_flag = 1;
		adc_convert_finish_flag = 0;
}




/**@brief Function for starting the blinking of the Advertisement LED.
 */
void adv_led_blink_start(void)
{  
		if (!m_is_adv_led_blinking)
    {
        uint32_t             err_code;
        static volatile bool is_led_on;
        
        is_led_on = true;

        nrf_gpio_pin_set(ADVERTISING_LED_PIN_NO);
        
        is_led_on             = true;
        m_is_adv_led_blinking = true;

        err_code = app_timer_start(m_adv_led_blink_timer_id, ADV_LED_ON_TIME, (void *)&is_led_on);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for stopping the blinking of the Advertisement LED.
 */
void adv_led_blink_stop(void)
{
    uint32_t err_code;

    err_code = app_timer_stop(m_adv_led_blink_timer_id);
    APP_ERROR_CHECK(err_code);

    m_is_adv_led_blinking = false;

    nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);
}


/**@brief Function for starting the blinking of the Alert LED.
 */
void alert_led_blink_start(void)
{
    if (!m_is_alert_led_blinking)
    {
        uint32_t             err_code;
        static volatile bool is_led_on;

        is_led_on = true;
        
        nrf_gpio_pin_set(ALERT_PIN_NO);
        
        m_is_alert_led_blinking = true;

        err_code = app_timer_start(m_alert_led_blink_timer_id,
                                   MILD_ALERT_LED_ON_TIME,
                                   (void *)&is_led_on);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for stopping the blinking of the Alert LED.
 */
void alert_led_blink_stop(void)
{
    uint32_t err_code;

    err_code = app_timer_stop(m_alert_led_blink_timer_id);
    APP_ERROR_CHECK(err_code);

    m_is_alert_led_blinking = false;

    nrf_gpio_pin_clear(ALERT_PIN_NO);
}


/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    uint32_t             err_code;
    ble_gap_adv_params_t adv_params;
    ble_gap_whitelist_t  whitelist;
    

    // Initialize advertising parameters with defaults values
    memset(&adv_params, 0, sizeof(adv_params));
    
    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.p_whitelist = NULL;

    // Configure advertisement according to current advertising state
    switch (m_advertising_mode)
    {
        case BLE_NO_ADV:
            m_advertising_mode = BLE_FAST_ADV_WHITELIST;
            // fall through

        case BLE_FAST_ADV_WHITELIST:
            err_code = ble_bondmngr_whitelist_get(&whitelist);
            APP_ERROR_CHECK(err_code);

            if ((whitelist.addr_count != 0) || (whitelist.irk_count != 0))
            {
                adv_params.fp          = BLE_GAP_ADV_FP_FILTER_CONNREQ;
                adv_params.p_whitelist = &whitelist;
                
                advertising_init(BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
                m_advertising_mode = BLE_FAST_ADV;
            }
            else
            {
                m_advertising_mode = BLE_SLOW_ADV;
            }
            
            adv_params.interval = APP_ADV_INTERVAL_FAST;
            adv_params.timeout  = APP_FAST_ADV_TIMEOUT;
            break;
            
        case BLE_FAST_ADV:
            advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
            
            adv_params.interval = APP_ADV_INTERVAL_FAST;
            adv_params.timeout  = APP_FAST_ADV_TIMEOUT;
            m_advertising_mode  = BLE_SLOW_ADV;            
            break;
            
        case BLE_SLOW_ADV:
            advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
            
            adv_params.interval = APP_ADV_INTERVAL_SLOW;
            adv_params.timeout  = APP_SLOW_ADV_TIMEOUT;
            m_advertising_mode  = BLE_SLEEP;
            
            break;
            
        default:
            break;
    }

    // Start advertising
    err_code = sd_ble_gap_adv_start(&adv_params);
    APP_ERROR_CHECK(err_code);
    
    adv_led_blink_start();
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *          This function will start the ADC.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	if(adc_convert_finish_flag == 1)
    adc_start();
}


/**@brief Function for handling the timeout that is responsible for toggling the Alert LED.
 *
 * @details This function will be called each time the timer that was started for the sake of
 *          blinking the Alert LED expires. This function toggle the state of the Alert LED.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
void mild_alert_timeout_handler(void * p_context)
{
    if (m_is_alert_led_blinking)
    {
        bool *          p_is_led_on;
        uint32_t        next_timer_interval;
        uint32_t        err_code;

        APP_ERROR_CHECK_BOOL(p_context != NULL);

        nrf_gpio_pin_toggle(ALERT_PIN_NO);

        p_is_led_on = (bool *)p_context;

        *p_is_led_on = !(*p_is_led_on);

        if(*p_is_led_on)
        {
            // The toggle operation above would have resulted in an ON state. So start a timer that
            // will expire after MILD_ALERT_LED_OFF_TIME.
            next_timer_interval = MILD_ALERT_LED_ON_TIME;
        }
        else
        {
            // The toggle operation above would have resulted in an OFF state. So start a timer that
            // will expire after MILD_ALERT_LED_OFF_TIME.
            next_timer_interval = MILD_ALERT_LED_OFF_TIME;
        }

        err_code = app_timer_start(m_alert_led_blink_timer_id,
                                   next_timer_interval,
                                   p_is_led_on);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling the timeout that is responsible for toggling the Advertisement LED.
 *
 * @details This function will be called each time the timer that was started for the sake of
 *          blinking the Advertisement LED expires. This function toggle the state of the
 *          Advertisement LED.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
void adv_led_blink_timeout_handler(void * p_context)
{
    if (m_is_adv_led_blinking)
    {
        bool *          p_is_led_on;
        uint32_t        next_timer_interval;
        uint32_t        err_code;
    
        APP_ERROR_CHECK_BOOL(p_context != NULL);

        nrf_gpio_pin_toggle(ADVERTISING_LED_PIN_NO);

        p_is_led_on = (bool *)(p_context);

        *p_is_led_on = !(*p_is_led_on);

        if(*p_is_led_on )
        {
            // The toggle operation above would have resulted in an ON state. So start a timer that
            // will expire after ADV_LED_ON_TIME.
            next_timer_interval = ADV_LED_ON_TIME;
        }
        else
        {
            // The toggle operation above would have resulted in an OFF state. So start a timer that
            // will expire after ADV_LED_OFF_TIME.
            next_timer_interval = ADV_LED_OFF_TIME;
        }

        err_code = app_timer_start(m_adv_led_blink_timer_id,
                                   next_timer_interval,
                                   p_is_led_on );
        APP_ERROR_CHECK(err_code);
    }
}


void rtc_timer_timeout_handler(void * p_context)
{
		UNUSED_PARAMETER(p_context);
		timer.sec++;
		updata_rtc_timer();
		if(sound_frequency>=20)
		{
				send_cry_signal(CRY_COMMAND, 1);
		}
		else
		{
				send_cry_signal(CRY_COMMAND, 0);
		}
#ifdef  DEBUG_LOG
				sound_value=(sound_value+sound_frequency)/2;
				printf("sound_frequency:%d\r\n",sound_frequency);
//				printf("sound_value:%d\r\n",sound_value);
				sound_frequency	=	0;
#endif					

			htu21d_data_has_updata = Updata_HTU();
			if(htu21d_data_has_updata)
			{
#ifdef  DEBUG_LOG
				printf("%d-%d-%d,%d:%d:%d\r\n",timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);
				printf("temp:%.1f,rh:%.1f\r\n",HTU.T,HTU.RH);
#endif				

				htu21d_data_has_updata = false;
				if(mconnected)
				{
						send_temp_rh_data(TEMPERATURE_COMMAND, HTU.T);
						nrf_delay_ms(1);
						send_temp_rh_data(RH_COMMAND, HTU.RH);
				}
			}
}
void cry_sound_timer_timeout_handler(void * p_context)
{
		UNUSED_PARAMETER(p_context);
		if(adc_convert_finish_flag == 1)
			cry_sound_adc_start();	
}

void oled_timer_timeout_handler(void * p_context)
{
		UNUSED_PARAMETER(p_context);
		OLED_POWER_CONTROL(0);
		oled_statu = 0;
	
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by this application.
 */
void leds_init(void)
{
    GPIO_LED_CONFIG(ADVERTISING_LED_PIN_NO);
    GPIO_LED_CONFIG(ALERT_PIN_NO);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create battery timer
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_create(&m_alert_led_blink_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                mild_alert_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_adv_led_blink_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                adv_led_blink_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
//insert by su	
		err_code = app_timer_create(&m_rtc_timer_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                rtc_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);

		err_code = app_timer_create(&m_cry_sound_timer_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                cry_sound_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);
		
		err_code = app_timer_create(&m_oled_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                oled_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);
//insert by su

}


/**@brief Function for the GAP initialization.
 *
 * @details This function shall be used to setup all the necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_KEYRING);
    APP_ERROR_CHECK(err_code);
    
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_tx_power_set(TX_POWER_LEVEL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 *
 * @param[in]  adv_flags  Indicates which type of advertisement to use, see @ref BLE_GAP_DISC_MODES.
 * 
 */
void advertising_init(uint8_t adv_flags)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    int8_t        tx_power_level = TX_POWER_LEVEL;
	
		//insert by su
//	  ble_advdata_t scanrsp;
//    ble_uuid_t nus_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, m_nus.uuid_type}};
		//insert by su
	
    ble_uuid_t adv_uuids[] = 
    {
        {BLE_UUID_TX_POWER_SERVICE,        BLE_UUID_TYPE_BLE}, 
        {BLE_UUID_IMMEDIATE_ALERT_SERVICE, BLE_UUID_TYPE_BLE}, 
        {BLE_UUID_LINK_LOSS_SERVICE,       BLE_UUID_TYPE_BLE},
//				{BLE_UUID_NUS_SERVICE,       			 BLE_UUID_TYPE_BLE},
    };

    m_advertising_mode = BLE_NO_ADV;

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));
    
    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(adv_flags);
    advdata.flags.p_data            = &adv_flags;
    advdata.p_tx_power_level        = &tx_power_level;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;


		//insert by su
//		memset(&scanrsp, 0, sizeof(scanrsp));
//    scanrsp.uuids_complete.uuid_cnt = sizeof(nus_adv_uuids) / sizeof(nus_adv_uuids[0]);
//    scanrsp.uuids_complete.p_uuids  = nus_adv_uuids;
		//insert by su

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
}





/**@brief Function for initializing the TX Power Service.
 */
void tps_init(void)
{
    uint32_t       err_code;
    ble_tps_init_t tps_init_obj;

    memset(&tps_init_obj, 0, sizeof(tps_init_obj));
    tps_init_obj.initial_tx_power_level = TX_POWER_LEVEL;
    
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&tps_init_obj.tps_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&tps_init_obj.tps_attr_md.write_perm);

    err_code = ble_tps_init(&m_tps, &tps_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Immediate Alert Service.
 */
void ias_init(void)
{
    uint32_t       err_code;
    ble_ias_init_t ias_init_obj;

    memset(&ias_init_obj, 0, sizeof(ias_init_obj));
    ias_init_obj.evt_handler = on_ias_evt;
    
    err_code = ble_ias_init(&m_ias, &ias_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Link Loss Service.
 */
void lls_init(void)
{
    uint32_t       err_code;
    ble_lls_init_t lls_init_obj;

    // Initialize Link Loss Service
    memset(&lls_init_obj, 0, sizeof(lls_init_obj));
    
    lls_init_obj.evt_handler         = on_lls_evt;
    lls_init_obj.error_handler       = service_error_handler;
    lls_init_obj.initial_alert_level = INITIAL_LLS_ALERT_LEVEL;
    
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&lls_init_obj.lls_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&lls_init_obj.lls_attr_md.write_perm);

    err_code = ble_lls_init(&m_lls, &lls_init_obj);
    APP_ERROR_CHECK(err_code);
    m_is_link_loss_alerting = false;
}


/**@brief Function for initializing the Battery Service.
 */
void bas_init(void)
{
    uint32_t       err_code;
    ble_bas_init_t bas_init_obj;
    
    memset(&bas_init_obj, 0, sizeof(bas_init_obj));
    
    bas_init_obj.evt_handler          = on_bas_evt;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_report_read_perm);
    
    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing the immediate alert service client.
 * @details This will initialize the client side functionality of the Find Me profile.
 */
void ias_client_init(void)
{
    uint32_t            err_code;
    ble_ias_c_init_t    ias_c_init_obj;

    memset(&ias_c_init_obj, 0, sizeof(ias_c_init_obj));

    m_is_high_alert_signalled = false;

    ias_c_init_obj.evt_handler    = on_ias_c_evt;
    ias_c_init_obj.error_handler  = service_error_handler;

    err_code = ble_ias_c_init(&m_ias_c, &ias_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


//insert by su
void nus_receive_data_handler(ble_nus_t * p_nus, uint8_t * data, uint16_t length)
{
		receive_handle(data);
#ifdef  DEBUG_LOG
 		printf("%d-%d-%d,%d:%d:%d,nus_receive_data_handler\r\n",timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);
#endif

}

/**@brief Initialize services that will be used by the application.
 */
void nus_init(void)
{
    uint32_t err_code;
    static ble_nus_init_t nus_init;
    
    memset(&nus_init, 0, sizeof nus_init);
    nus_init.data_handler = nus_receive_data_handler;

		err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Connection Parameters Module handler.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    APP_ERROR_CHECK_BOOL(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED);
    
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
}
//insert by su

/**@brief Function for initializing the services that will be used by the application.
 */
void services_init(void)
{
    tps_init();
    ias_init();
    lls_init();
    bas_init();
    ias_client_init();
		nus_init();
}


/**@brief Function for initializing the security parameters.
 */
void sec_params_init(void)
{
    m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;  
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}




/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Signals alert event from Immediate Alert or Link Loss services.
 *
 * @param[in]   alert_level  Requested alert level.
 */
void alert_signal(uint8_t alert_level)
{
    switch (alert_level)
    {
        case BLE_CHAR_ALERT_LEVEL_NO_ALERT:
            m_is_link_loss_alerting = false;
            alert_led_blink_stop();
            break;

        case BLE_CHAR_ALERT_LEVEL_MILD_ALERT:
            alert_led_blink_start();
            break;

        case BLE_CHAR_ALERT_LEVEL_HIGH_ALERT:
            alert_led_blink_stop();
            nrf_gpio_pin_set(ALERT_PIN_NO);
            break;
            
        default:
            break;
    }
}


/**@brief Function for handling Immediate Alert events.
 *
 * @details This function will be called for all Immediate Alert events which are passed to the
 *          application.
 *
 * @param[in]   p_ias  Immediate Alert structure.
 * @param[in]   p_evt  Event received from the Immediate Alert service.
 */
void on_ias_evt(ble_ias_t * p_ias, ble_ias_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_IAS_EVT_ALERT_LEVEL_UPDATED:
            alert_signal(p_evt->params.alert_level);
            break;
            
        default:
            break;
    }
}


/**@brief Function for handling Link Loss events.
 *
 * @details This function will be called for all Link Loss events which are passed to the
 *          application.
 *
 * @param[in]   p_lls  Link Loss stucture.
 * @param[in]   p_evt  Event received from the Link Loss service.
 */
void on_lls_evt(ble_lls_t * p_lls, ble_lls_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_LLS_EVT_LINK_LOSS_ALERT:
            m_is_link_loss_alerting = true;
            alert_signal(p_evt->params.alert_level);
            break;
            
        default:
            break;
    }
}


/**@brief Function for handling IAS Client events.
 *
 * @details This function will be called for all IAS Client events which are passed to the
 *          application.
 *
 * @param[in]   p_ias_c  IAS Client stucture.
 * @param[in]   p_evt    Event received.
 */
void on_ias_c_evt(ble_ias_c_t * p_ias_c, ble_ias_c_evt_t * p_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_evt->evt_type)
    {
        case BLE_IAS_C_EVT_SRV_DISCOVERED:
            // IAS is found on peer. The Find Me Locator functionality of this app will work.
            break;

        case BLE_IAS_C_EVT_SRV_NOT_FOUND:
            // IAS is not found on peer. The Find Me Locator functionality of this app will NOT work.
            break;

        case BLE_IAS_C_EVT_DISCONN_COMPLETE:
            // Stop detecting button presses when not connected
            err_code = app_button_disable();
            break;

        default:
            break;
    }

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Battery Service events.
 *
 * @details This function will be called for all Battery Service events which are passed to the
 |          application.
 *
 * @param[in]   p_bas  Battery Service stucture.
 * @param[in]   p_evt  Event received from the Battery Service.
 */
void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t *p_evt)
{
    uint32_t err_code;

    switch (p_evt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            // Start battery timer
            err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
				
            break;

        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            err_code = app_timer_stop(m_battery_timer_id);
            APP_ERROR_CHECK(err_code);
				
            break;

        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t        err_code      = NRF_SUCCESS;
    static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
	
    //insert by su
    static ble_gap_evt_auth_status_t m_auth_status;
    ble_gap_enc_info_t *             p_enc_info;		
		//insert by su
	
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
						//insert by su      
						mconnected = true;
						//insert by su      
            adv_led_blink_stop();

            if (m_is_link_loss_alerting)
            {
                alert_led_blink_stop();
            }
            
            m_advertising_mode = BLE_NO_ADV;
            m_conn_handle      = p_ble_evt->evt.gap_evt.conn_handle;
            
//comment by su for program run forever

            // Start handling button presses
//            err_code = app_button_enable();
						
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
						//insert by su      
						mconnected = false;
						//insert by su 
            if (!m_is_link_loss_alerting)
            {
                alert_led_blink_stop();
            }

            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            // Since we are not in a connection and have not started advertising, store bonds
            err_code = ble_bondmngr_bonded_masters_store();
            APP_ERROR_CHECK(err_code);
						
            advertising_start();
            break;
            
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, 
                                                   BLE_GAP_SEC_STATUS_SUCCESS, 
                                                   &m_sec_params);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
            { 
//comment by su for program run forever
							
//                if (m_advertising_mode == BLE_SLEEP)
//                {
//                    m_advertising_mode = BLE_NO_ADV;
//                    adv_led_blink_stop();
//                    alert_led_blink_stop();
//                    GPIO_WAKEUP_BUTTON_CONFIG(SIGNAL_ALERT_BUTTON);
//                    GPIO_WAKEUP_BUTTON_CONFIG(BONDMNGR_DELETE_BUTTON_PIN_NO);
//                                    
//                    // Go to system-off mode
//                    // (this function will not return; wakeup will cause a reset)
//                    err_code = sd_power_system_off();    
//                }
//                else
//                {
//                    advertising_start();
//                }
							
//comment by su
							
							//insert by su    for adviod stop advertising
							m_advertising_mode = BLE_NO_ADV;
							advertising_start();
							//insert by su    
            }
            break;
						


			
        case BLE_GATTC_EVT_TIMEOUT:
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            break;
				
				//insert by su		
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0);
            break;
        case BLE_GAP_EVT_AUTH_STATUS:
            m_auth_status = p_ble_evt->evt.gap_evt.params.auth_status;
            break;
        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            p_enc_info = &m_auth_status.periph_keys.enc_info;
            if (p_enc_info->div == p_ble_evt->evt.gap_evt.params.sec_info_request.div)
            {
                err_code = sd_ble_gap_sec_info_reply(m_conn_handle, p_enc_info, NULL);
            }
            else
            {
                // No keys found for this device
                err_code = sd_ble_gap_sec_info_reply(m_conn_handle, NULL, NULL);
            }
            break;
				//insert by su
        default:
            break;
    }

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_bondmngr_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_ias_on_ble_evt(&m_ias, p_ble_evt);
    ble_lls_on_ble_evt(&m_lls, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
    ble_ias_c_on_ble_evt(&m_ias_c, p_ble_evt);
		//insert by su
	  ble_nus_on_ble_evt(&m_nus, p_ble_evt);
		//insert by su
    on_ble_evt(p_ble_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    BLE_STACK_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM,
                           BLE_L2CAP_MTU_DEF,
                           ble_evt_dispatch,
                           false);
}


/**@brief Function for handling a Bond Manager error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
void bond_manager_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for the Bond Manager initialization.
 */
void bond_manager_init(void)
{
    uint32_t            err_code;
    ble_bondmngr_init_t bond_init_data;
    bool                bonds_delete;

    // Clear all bonded masters if the Bonds Delete button is pushed
    err_code = app_button_is_pushed(BONDMNGR_DELETE_BUTTON_PIN_NO, &bonds_delete);
    APP_ERROR_CHECK(err_code);
    
    // Initialize the Bond Manager
    bond_init_data.flash_page_num_bond     = FLASH_PAGE_BOND;
    bond_init_data.flash_page_num_sys_attr = FLASH_PAGE_SYS_ATTR;
    bond_init_data.evt_handler             = NULL;
    bond_init_data.error_handler           = bond_manager_error_handler;
    bond_init_data.bonds_delete            = bonds_delete;

    err_code = ble_bondmngr_init(&bond_init_data);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Radio Notification event.
 */
void radio_notification_init(void)
{
    uint32_t err_code;

    err_code = ble_radio_notification_init(NRF_APP_PRIORITY_HIGH,
                                           NRF_RADIO_NOTIFICATION_DISTANCE_4560US,
                                           ble_flash_on_radio_active_evt);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
void button_event_handler(uint8_t pin_no)
{
    uint32_t err_code;
		printf("button_event_handler\r\n");
    switch (pin_no)
    {
        case SIGNAL_ALERT_BUTTON:
            if (!m_is_high_alert_signalled)
            {
                err_code = ble_ias_c_send_alert_level(&m_ias_c, BLE_CHAR_ALERT_LEVEL_HIGH_ALERT);
            }
            else
            {
                err_code = ble_ias_c_send_alert_level(&m_ias_c, BLE_CHAR_ALERT_LEVEL_NO_ALERT);
            }

            if (err_code == NRF_SUCCESS)
            {
                m_is_high_alert_signalled = !m_is_high_alert_signalled;
            }
            else if (
                (err_code != BLE_ERROR_NO_TX_BUFFERS)
                &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                &&
                (err_code != NRF_ERROR_NOT_FOUND)
            )
            {
                APP_ERROR_HANDLER(err_code);
            }
            break;

        case STOP_ALERTING_BUTTON:
            alert_led_blink_stop();
            break;

				
				case MY_BUTTON:
					if(oled_statu==0)
					{
						OLED_POWER_CONTROL(1);
						oled_statu = 1;
						err_code = app_timer_start(m_oled_timer_id, OLED_TIMER_INTERVAL, NULL);
						APP_ERROR_CHECK(err_code);

						
						m_advertising_mode = BLE_FAST_ADV;
						advertising_start();
						m_adv_statu = 1;	
						now_min = timer.min;
					}
					
//					send_cry_signal(CRY_COMMAND, 1);
					break;
        default:
            APP_ERROR_HANDLER(pin_no);
    }
}


/**@brief Function for initializing the GPIOTE handler module.
 */
void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


/**@brief Function for initializing the button handler module.
 */
void buttons_init(void)
{
    static app_button_cfg_t buttons[] =
    {
        {SIGNAL_ALERT_BUTTON,  false, NRF_GPIO_PIN_PULLUP, button_event_handler},
        {STOP_ALERTING_BUTTON, false, NRF_GPIO_PIN_PULLUP, button_event_handler},
				{MY_BUTTON, false, NRF_GPIO_PIN_PULLUP, button_event_handler}
    };
    
    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);
}


/**@brief Function for the Power manager.
 */
void power_manage(void)
{
    uint32_t err_code;
    err_code = sd_app_event_wait();
    APP_ERROR_CHECK(err_code);
}

//insert by su
void uart_init(void)
{
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, false);
    
    NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;
    
    NVIC_SetPriority(UART0_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(UART0_IRQn);
}

void UART0_IRQHandler(void)
{
    static uint8_t data_array[NUS_MAX_DATA_LENGTH];
    static uint8_t index = 0;
    uint32_t err_code;
    
    data_array[index] = simple_uart_get();
    if (data_array[index] == '\n' || index >= NUS_MAX_DATA_LENGTH)
    {
        err_code = ble_nus_send_string(&m_nus, data_array, index);
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        
        index = 0;
    }
    else
    {
        index++;
    }
}


//insert by su
void updata_by_min(void)
{
		uint8_t static min = 0;
		if(timer.min == (now_min+2) && m_adv_statu==1)
		{
			m_adv_statu = 0;
			m_advertising_mode = BLE_NO_ADV;
			advertising_start();
		}
	
		if(min!=timer.min)
		{
			wdt_feed();
			min = timer.min;
//			htu21d_data_has_updata = Updata_HTU();
//			if(htu21d_data_has_updata)
//			{
//#ifdef  DEBUG_LOG
//				printf("%d-%d-%d,%d:%d:%d\r\n",timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);
//				printf("temp:%.1f,rh:%.1f\r\n",HTU.T,HTU.RH);
//#endif				

//				htu21d_data_has_updata = false;
//				if(mconnected)
//				{
//						send_temp_rh_data(TEMPERATURE_COMMAND, HTU.T);
//						nrf_delay_ms(1);
//						send_temp_rh_data(RH_COMMAND, HTU.RH);
//				}
//			}
		}
}

void updata_by_hour(void)
{
		uint8_t static hour = 0;
		if(hour!=timer.hour)
		{
			hour = timer.hour;
//			adc_start();		//获取电池电量百分比，并在adc中断中显示电量值
		}
}

void my_timer_init(void)
{
		uint32_t    err_code;
		//insert by su Start RTC
		err_code = app_timer_start(m_rtc_timer_timer_id, RTC_TIMER_INTERVAL, NULL);
		APP_ERROR_CHECK(err_code);
		
		err_code = app_timer_start(m_cry_sound_timer_timer_id, CRY_SOUND_TIMER_INTERVAL, NULL);
		APP_ERROR_CHECK(err_code);

}
