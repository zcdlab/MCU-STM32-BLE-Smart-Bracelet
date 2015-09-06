#ifndef __BLE_APP_MAIN_H
#define __BLE_APP_MAIN_H
#include "ble_includes.h"

#define DEBUG_LOG

#define MY_BUTTON               					30                           /**< Button used for send or cancel High Alert to the peer. */
#define SIGNAL_ALERT_BUTTON               EVAL_BOARD_BUTTON_0                               /**< Button used for send or cancel High Alert to the peer. */
#define STOP_ALERTING_BUTTON              EVAL_BOARD_BUTTON_1                               /**< Button used for clearing the Alert LED that may be blinking or turned ON because of alerts from the master. */
#define BONDMNGR_DELETE_BUTTON_PIN_NO     EVAL_BOARD_BUTTON_1                               /**< Button used for deleting all bonded masters during startup. */

#define ALERT_PIN_NO                      EVAL_BOARD_LED_1                                  /**< Pin used as LED to signal an alert. */

#define DEVICE_NAME                       "Child Watch"                                     /**< Name of device. Will be included in the advertising data. */
#define APP_ADV_INTERVAL_FAST             0x0028                                            /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_INTERVAL_SLOW             0x0C80                                            /**< Slow advertising interval (in units of 0.625 ms. This value corresponds to 2 seconds). */
#define APP_SLOW_ADV_TIMEOUT              180                                               /**< The duration of the slow advertising period (in seconds). */
#define APP_FAST_ADV_TIMEOUT              30                                                /**< The duration of the fast advertising period (in seconds). */

#define APP_TIMER_PRESCALER               0                                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS              10 //5+2                                           /**< Maximum number of simultaneously created timers. 1 for Battery measurement, 2 for flashing Advertising LED and Alert LED, 1 for connection parameters module, 1 for button polling timer needed by the app_button module,  */
#define APP_TIMER_OP_QUEUE_SIZE           10 //6+4                                          /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL       APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)      /**< Battery level measurement interval (ticks). This value corresponds to 120 seconds. */
#define ADV_LED_ON_TIME                   APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)         /**< Advertisement LED ON period when in blinking state. */
#define ADV_LED_OFF_TIME                  APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)         /**< Advertisement LED OFF period when in blinking state. */

#define MILD_ALERT_LED_ON_TIME            APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)         /**< Alert LED ON period when in blinking state. */
#define MILD_ALERT_LED_OFF_TIME           APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)         /**< Alert LED OFF period when in blinking state. */

#define MIN_CONN_INTERVAL                 MSEC_TO_UNITS(500, UNIT_1_25_MS)                  /**< Minimum acceptable connection interval (0.5 seconds).  */
#define MAX_CONN_INTERVAL                 MSEC_TO_UNITS(1000, UNIT_1_25_MS)                 /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                     0                                                 /**< Slave latency. */
#define CONN_SUP_TIMEOUT                  MSEC_TO_UNITS(4000, UNIT_10_MS)                   /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(5 * 1000, APP_TIMER_PRESCALER)    /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY     APP_TIMER_TICKS(5 * 1000, APP_TIMER_PRESCALER)    /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT      3                                                 /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_GPIOTE_MAX_USERS              1                                                 /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY            APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)          /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_TIMEOUT                 30                                                /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                    1                                                 /**< Perform bonding. */
#define SEC_PARAM_MITM                    0                                                 /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES         BLE_GAP_IO_CAPS_NONE                              /**< No I/O capabilities. */
#define SEC_PARAM_OOB                     0                                                 /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE            7                                                 /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE            16                                                /**< Maximum encryption key size. */

#define INITIAL_LLS_ALERT_LEVEL           BLE_CHAR_ALERT_LEVEL_NO_ALERT                     /**< Initial value for the Alert Level characteristic in the Link Loss service. */
#define TX_POWER_LEVEL                    (0)                                              /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */

#define FLASH_PAGE_SYS_ATTR              (BLE_FLASH_PAGE_END - 3)                           /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                  (BLE_FLASH_PAGE_END - 1)                           /**< Flash page used for bond manager bonding information. */

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS     1200                                              /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION      3                                                 /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS    270                                               /**< Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) that is connected in series with the voltage supply. This is the voltage drop when the forward current is 1mA. Source: Data sheet of 'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com. */

#define DEAD_BEEF                         0xDEADBEEF                                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */



//insert by su
#define RTC_TIMER_INTERVAL       					APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)      /**< Battery level measurement interval (ticks). This value corresponds to 120 seconds. */
#define CRY_SOUND_TIMER_INTERVAL   				APP_TIMER_TICKS(3, APP_TIMER_PRESCALER)      	/**< Battery level measurement interval (ticks). This value corresponds to 120 seconds. */
#define OLED_TIMER_INTERVAL   						APP_TIMER_TICKS(10000, APP_TIMER_PRESCALER)      	/**< Battery level measurement interval (ticks). This value corresponds to 120 seconds. */
//insert by su


void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);
void service_error_handler(uint32_t nrf_error);

void uart_init(void);
void leds_init(void);
void timers_init(void);
void gpiote_init(void);
void buttons_init(void);
void my_timer_init(void);


void tps_init(void);
void ias_init(void);
void lls_init(void);
void bas_init(void);
void ias_client_init(void);
void nus_init(void);
void services_init(void);


void radio_notification_init(void);
void advertising_init(uint8_t adv_flags);
void gap_params_init(void);
void sec_params_init(void);
void conn_params_init(void);
void ble_stack_init(void);
void bond_manager_init(void);


void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
void on_ias_evt(ble_ias_t * p_ias, ble_ias_evt_t * p_evt);
void on_lls_evt(ble_lls_t * p_lls, ble_lls_evt_t * p_evt);
void on_ias_c_evt(ble_ias_c_t * p_ias_c, ble_ias_c_evt_t * p_evt);
void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t *p_evt);
void on_ble_evt(ble_evt_t * p_ble_evt);
void ble_evt_dispatch(ble_evt_t * p_ble_evt);


void power_manage(void);
void advertising_start(void);
void alert_signal(uint8_t alert_level);
void adv_led_blink_start(void);
void adv_led_blink_stop(void);
void alert_led_blink_start(void);
void alert_led_blink_stop(void);


void adc_start(void);
void cry_sound_adc_start(void);
void ADC_IRQHandler(void);
void UART0_IRQHandler(void);


void rtc_timer_timeout_handler(void * p_context);
void cry_sound_timer_timeout_handler(void * p_context);
void oled_timer_timeout_handler(void * p_context);
void battery_level_meas_timeout_handler(void * p_context);
void button_event_handler(uint8_t pin_no);
void nus_receive_data_handler(ble_nus_t * p_nus, uint8_t * data, uint16_t length);


void mild_alert_timeout_handler(void * p_context);
void adv_led_blink_timeout_handler(void * p_context);
void conn_params_error_handler(uint32_t nrf_error);
void bond_manager_error_handler(uint32_t nrf_error);


void updata_by_min(void);
void updata_by_hour(void);
#endif
