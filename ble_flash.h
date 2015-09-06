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
 * @defgroup ble_flash_module Flash Manager
 * @{
 * @ingroup ble_sdk_lib
 * @brief Module for accessing flash memory.
 *
 * @details It contains functions for reading, writing and erasing one page in flash.
 *
 *          The module uses the first 32 bits of the flash page to write a magic number in order to
 *          determine if the page has been written or not.
 *
 * @note Be careful not to use a page number in the SoftDevice area (which currently occupies the
 *       range 0 to 127), or in your application space! In both cases, this would end up
 *       with a hard fault.
 */

#ifndef BLE_FLASH_H__
#define BLE_FLASH_H__

#include <stdint.h>
#include <stdbool.h>
#include <nrf51.h>

/*flash´æ´¢¸ñÊ½
*   12byte		12byte		 2byte      2byte
*  bondnum	parent  areacode  	cellid
*            num   	  hex				hex
*/
#define USERPROFILE_LEN  1

typedef union {
    uint32_t data[USERPROFILE_LEN];
} userprofile_union_t;
//insert by su  copy from baidu band

#define BLE_FLASH_PAGE_SIZE     ((uint16_t)NRF_FICR->CODEPAGESIZE)  /**< Size of one flash page. */
#define BLE_FLASH_MAGIC_NUMBER  0x45DE0000                          /**< Magic value to identify if flash contains valid data. */
#define BLE_FLASH_EMPTY_MASK    0xFFFFFFFF                          /**< Bit mask that defines an empty address in flash. */


//insert by su  copy from baidu band
#define FLASH_PAGE_HEADER_LEN   (2)
#define PAGE_NUM_FOR_DATA         (32)
#define PAGE_NUM_FOR_SWAP         (60)
#define PAGE_NUM_FOR_CONFIG       (10)

#define OTA_BLOCK_START_PAGE      (NRF_FICR->CODESIZE-14)
#define CONFIG_BLOCK_START_PAGE   (NRF_FICR->CODESIZE-24)
#define DATA_BLOCK_START_PAGE     (NRF_FICR->CODESIZE-56)
#define SWAP_BLOCK_STAET_PAGE     (NRF_FICR->CODESIZE-116)

//half close half open area just like "[)"
#define ERASABLE_BLOCK_START       (DATA_BLOCK_START_PAGE)
#define ERASABLE_BLOCK_END         (CONFIG_BLOCK_START_PAGE + PAGE_NUM_FOR_CONFIG)
#define FLASH_PAGE_STORE_TIME      (NRF_FICR->CODESIZE-23)                           /**< used to store */
#define FLASH_PAGE_DAILY_TARGET    (NRF_FICR->CODESIZE-22)           /**< Flash page used for SN and factory test flag*/
//#define FLASH_PAGE_ERROR_LOG       (NRF_FICR->CODESIZE-21)                           /**< Address in flash where stack trace can be stored. */
//#define FLASH_PAGE_SYS_ATTR        (NRF_FICR->CODESIZE-20)                           /**< Flash page used for bond manager system attribute information. */
//#define FLASH_PAGE_BOND            (NRF_FICR->CODESIZE-19)                           /**< 232 Flash page used for bond manager bonding information. */
#define FLASH_PAGE_PRIVATE_BOND    (NRF_FICR->CODESIZE-18)
#define FLASH_PAGE_USER_PROFILE    (NRF_FICR->CODESIZE-17)           /**< Flash page used for user profile*/
#define FLASH_PAGE_ALARM_SETTINGS  (NRF_FICR->CODESIZE-16)           /**< Flash page used for alarm settings*/
#define FLASH_PAGE_SN_FACTORY_FLAG (NRF_FICR->CODESIZE-15)           /**< Flash page used for SN and factory test flag*/
#define FLASH_PAGE_HEALTH_DATA      DATA_BLOCK_START_PAGE                             /**< start page used to store sports data > */
#define FLASH_PAGE_SLEEP_SETTINGS  (NRF_FICR->CODESIZE-25)                           /**< used to store sleep setting data*/


#define FLASH_PAGE_SIZE            (NRF_FICR->CODEPAGESIZE)



/**@brief Macro for getting the end of the flash available for application.
 * 
 * @details    The result flash page number indicates the end boundary of the flash available 
 *             to the application. If a bootloader is used, the end will be the start of the 
 *             bootloader region. Otherwise, the end will be the size of the flash. 
 */
#define BLE_FLASH_PAGE_END \
    ((NRF_UICR->BOOTLOADERADDR != BLE_FLASH_EMPTY_MASK) \
        ? (NRF_UICR->BOOTLOADERADDR / BLE_FLASH_PAGE_SIZE) \
        : NRF_FICR->CODESIZE)

/**@brief Function for erasing the specified flash page, and then writes the given data to this page.
 *
 * @warning This operation blocks the CPU. DO NOT use while in a connection!
 *
 * @param[in]  page_num     Page number to update.
 * @param[in]  p_in_array   Pointer to a RAM area containing the elements to write in flash.
 *                          This area has to be 32 bits aligned.
 * @param[in]  word_count   Number of 32 bits words to write in flash.
 *
 * @return     NRF_SUCCESS on successful flash write, otherwise an error code.
 */
uint32_t ble_flash_page_write(uint8_t page_num, uint32_t * p_in_array, uint8_t word_count);

/**@brief Function for reading data from flash to RAM.
 *
 * @param[in]  page_num       Page number to read.
 * @param[out] p_out_array    Pointer to a RAM area where the found data will be written. 
 *                            This area has to be 32 bits aligned.
 * @param[out] p_word_count   Number of 32 bits words read.
 *
 * @return     NRF_SUCCESS on successful upload, NRF_ERROR_NOT_FOUND if no valid data has been found
 *             in flash (first 32 bits not equal to the MAGIC_NUMBER+CRC).
 */
uint32_t ble_flash_page_read(uint8_t page_num, uint32_t * p_out_array, uint8_t * p_word_count);

/**@brief Function for erasing a flash page.
 *
 * @note This operation blocks the CPU, so it should not be done while the radio is running!
 *
 * @param[in]  page_num   Page number to erase.
 *
 * @return     NRF_SUCCESS on success, an error_code otherwise.
 */
uint32_t ble_flash_page_erase(uint8_t page_num);

/**@brief Function for writing one word to flash.
 *
 * @note Flash location to be written must have been erased previously.
 *
 * @param[in]  p_address   Pointer to flash location to be written.
 * @param[in]  value       Value to write to flash.
 *
 * @return     NRF_SUCCESS.
 */
uint32_t ble_flash_word_write(uint32_t * p_address, uint32_t value);

/**@brief Function for writing a data block to flash.
 *
 * @note Flash locations to be written must have been erased previously.
 *
 * @param[in]  p_address    Pointer to start of flash location to be written.
 * @param[in]  p_in_array   Pointer to start of flash block to be written.
 * @param[in]  word_count   Number of words to be written.
 *
 * @return     NRF_SUCCESS.
 */
uint32_t ble_flash_block_write(uint32_t * p_address, uint32_t * p_in_array, uint16_t word_count);

/**@brief Function for computing pointer to start of specified flash page.
 *
 * @param[in]  page_num       Page number.
 * @param[out] pp_page_addr   Pointer to start of flash page.
 *
 * @return     NRF_SUCCESS.
 */
uint32_t ble_flash_page_addr(uint8_t page_num, uint32_t ** pp_page_addr);

/**@brief Function for calculating a 16 bit CRC using the CRC-16-CCITT scheme.
 * 
 * @param[in]  p_data   Pointer to data on which the CRC is to be calulated.
 * @param[in]  size     Number of bytes on which the CRC is to be calulated.
 * @param[in]  p_crc    Initial CRC value (if NULL, a preset value is used as the initial value).
 *
 * @return     Calculated CRC.
 */
uint16_t ble_flash_crc16_compute(uint8_t * p_data, uint16_t size, uint16_t * p_crc);

/**@brief Function for handling flashing module Radio Notification event.
 *
 * @note For flash writing to work safely while in a connection or while advertising, this function
 *       MUST be called from the Radio Notification module's event handler (see
 *       @ref ble_radio_notification for details).
 *
 * @param[in]  radio_active   TRUE if radio is active (or about to become active), FALSE otherwise.
 */
void ble_flash_on_radio_active_evt(bool radio_active);

void flash_page_erase(uint32_t * p_page);

void save_user_profile(void );
void load_user_profile(void);
#endif // BLE_FLASH_H__

/** @} */
