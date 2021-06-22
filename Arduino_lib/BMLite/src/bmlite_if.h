/*
 * Copyright (c) 2020 Andrey Perminov <andrey.ppp@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BMLITE_IF_H
#define BMLITE_IF_H

#include "hcp_tiny.h"
#include "bmlite_if_callbacks.h"

/**
 * @brief Enroll finger. Created template must be saved to FLASH storage
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_enroll_finger(HCP_comm_t *chain);

/**
 * @brief Capture and identify finger against existing templates in Flash storage
 *
 * @param[in] chain   - HCP com chain
 * @param[in] timeout - timeout (msec). Maximum timeout 65535 msec
 *                      set to 0 for waiting indefinitely
 * 
 * @param[out] template_id - pointer for matched template ID
 * @param[out] match       - pointer to match result
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_identify_finger(HCP_comm_t *chain, uint32_t timeout, 
                uint16_t *template_id, bool *match);

/**
 * @brief Wait for finger present on sensor"
 *
 * @param[in] chain   - HCP com chain
 * @param[in] timeout - timeout (msec). Maximum timeout 65535 msec
 *                      set to 0 for waiting indefinitely
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t sensor_wait_finger_present(HCP_comm_t *chain, uint16_t timeout);

/**
 * @brief Wait for finger not present on sensor"
 *
 * @param[in] chain   - HCP com chain
 * @param[in] timeout - timeout (msec). Maximum timeout 65535 msec
 *                      set to 0 for waiting indefinitely
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t sensor_wait_finger_not_present(HCP_comm_t *chain, uint16_t timeout);

/**
 * @brief Wait for finger present on sensor and capture image"
 *
 * @param[in] chain   - HCP com chain
 * @param[in] timeout - timeout (msec). Maximum timeout 65535 msec
 *                      set to 0 for waiting indefinitely
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_capture(HCP_comm_t *chain, uint16_t timeout);

/**
 * @brief Get size of captured image
 *
 * @param[in] chain  - HCP com chain
 * 
 * @param[out] size  
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_image_get_size(HCP_comm_t *chain, uint32_t *size);

/**
 * @brief Allocates image buffer on FPC BM-LIte
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t image_create(HCP_comm_t *chain);

/**
 * @brief Deletes image buffer on FPC BM-LIte
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t image_delete(HCP_comm_t *chain);

/**
 * @brief Pull captured image from FPC BM-Lite
 *
 * @param[in] chain  - HCP com chain
 * 
 * @param[in] data   - pointer to image buffer
 * @param[in] size   - size of the image buffer
 *                     if buffer size is not enough the image
 *                     will be truncated
 *                     chain->arg.size will contain real size of the image
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_image_get(HCP_comm_t *chain, uint8_t *data, uint32_t size);

/**
 * @brief Push image to FPC BM-Lite
 *
 * @param[in] chain  - HCP com chain
 * 
 * @param[in] data   - pointer to image buffer
 * @param[in] size   - size of the image buffer
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_image_put(HCP_comm_t *chain, uint8_t *data, uint32_t size);

/**
 * @brief Extract image features to prepare image for enrolling or matching
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_image_extract(HCP_comm_t *chain);

/**
 * @brief Identify prepared image against existing templates in Flash storage
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_identify(HCP_comm_t *chain);

/**
 * @brief Save template after enroll is finished to FLASH storage
 *
 * @param[in] chain        - HCP com chain
 * @param[out] template_id - template ID
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_save(HCP_comm_t *chain, uint16_t template_id);

/**
 * @brief Remove template from RAM
 *
 * @param[in] chain        - HCP com chain
 * @param[in] template_id - template ID
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_remove_ram(HCP_comm_t *chain);

/**
 * @brief Pull template stored in RAM from FPC BM-Lite
 *
 * @param[in] chain  - HCP com chain
 * 
 * @param[in] data   - pointer to template buffer
 * @param[in] size   - size of the template buffer
 *                     if buffer size is not enough the template
 *                     will be truncated
 *                     chain->arg.size will contain real size of the template
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_get(HCP_comm_t *chain, uint8_t *data, uint32_t size);

/**
 * @brief Push template to FPC BM-Lite and stored it to RAM 
 *
 * @param[in] chain  - HCP com chain
 * 
 * @param[in] data   - pointer to template buffer
 * @param[in] size   - size of the template buffer
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_put(HCP_comm_t *chain, uint8_t *data, uint16_t length);

/**
 * @brief Remove template from FLASH storage
 *
 * @param[in] chain       - HCP com chain
 * @param[in] template_id - template ID
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_remove(HCP_comm_t *chain, uint16_t template_id);

/**
 * @brief Remove all templates from FLASH storage
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_remove_all(HCP_comm_t *chain);

/**
 * @brief Copy template from FLASH storage to RAM
 *
 * @param[in] chain       - HCP com chain
 * @param[in] template_id - template ID
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_load_storage(HCP_comm_t *chain, uint16_t template_id);

/**
 * @brief Remove template from FLASH storage
 *
 * @param[in] chain        - HCP com chain
 * @param[out] template_id - template ID
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_template_get_count(HCP_comm_t *chain, uint16_t *count);

/**
 * @brief Get array of template ID stored on FPC BM-LIte
 *
 * @param[in] chain          - HCP com chain
 * @return ::fpc_bep_result_t
 *           chain->arg.data - pointer to array of uint16_t of IDs
 *           chain->arg.size - length of the array (in bytes). For calculating
 *                             number of templates divide the arg.size by 2
 */
fpc_bep_result_t bep_template_get_ids(HCP_comm_t *chain);

/**
 * @brief Software reset of FCP BM-Lite
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_sw_reset(HCP_comm_t *chain);

/**
 * @brief Calibrate FPC BM-LIte sensor and store calibration data to FLASH storage
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 * 
 * FPC BM-Lite must be restarted to activate new calibration data
 */
fpc_bep_result_t bep_sensor_calibrate(HCP_comm_t *chain);

/**
 * @brief Remove FPC BM-LIte sensor calibration data from FLASH storage
 *
 * @param[in] chain  - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 * 
 * FPC BM-Lite must be restarted to activate
 */
fpc_bep_result_t bep_sensor_calibrate_remove(HCP_comm_t *chain);

/**
 * @brief Get version of FPC BM-LIte firmware
 *
 * @param[in] chain   - HCP com chain
 * @param[in] version - pointer to data buffer
 * @param[in] size    - size of the data buffer
 *                      if buffer size is not enough the data
 *                      will be truncated
 *                      chain->arg.size will contain real size of the data
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_version(HCP_comm_t *chain, char *version, int len);

/**
 * @brief Get version of FPC BM-LIte firmware
 *
 * @param[in] chain     - HCP com chain
 * @param[in] unique_id - pointer to data buffer
 *                        chain->arg.size will contain real size of the data
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_unique_id_get(HCP_comm_t *chain, uint8_t *unique_id);

/**
 * @brief Set requested UART communication speed
 *
 * @param[in] chain  - HCP com chain
 * @param[in] speed  - UART speed
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_uart_speed_set(HCP_comm_t *chain, uint32_t speed);

/**
 * @brief Get current UART communication speed
 *
 * @param[in] chain   - HCP com chain
 * @param[out] speed  - UART speed
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_uart_speed_get(HCP_comm_t *chain, uint32_t *speed);

/**
 * @brief Reset FPC BM-Lite fingerprint sensor
 *
 * @param[in] chain   - HCP com chain
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bep_sensor_reset(HCP_comm_t *chain);

/**
 * @brief Build and send command to FPC BM-Lite and receive answer
 *
 * @param[in] chain    - HCP com chain
 * @param[in] cmd      - BM-Lite command
 * @param[in] arg_type - Argument without parameters
 *                       set to ARG_NONE if the command has no argument
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_send_cmd(HCP_comm_t *chain, uint16_t cmd, uint16_t arg_type);

/**
 * @brief Build and send command with additiona argument with parameters
 *
 * @param[in] chain       - HCP com chain
 * @param[in] cmd         - BM-Lite command
 * @param[in] arg1_type   - argument 1 without parameters
 *                          set to ARG_NONE if the command has no argument without paramener
 * @param[in] arg2_type   - argument 2
 * @param[in] arg2_data   - data pointer for argument 2
 *                          set to 0 if argument 2 has no parameter
 * @param[in] arg2_length - length of data for argument 2
 *                          set to 0 if argument 2 has no parameter
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_send_cmd_arg(HCP_comm_t *chain, uint16_t cmd, uint16_t arg1_type, uint16_t arg2_type, void *arg2_data, uint16_t arg2_length);



#endif