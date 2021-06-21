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

#ifndef BMLITE_IF_CALLBACKS_H
#define BMLITE_IF_CALLBACKS_H
#include <stdint.h>

#ifndef BMLITE_USE_CALLBACK
  #define bmlite_on_error(error, value) 
  #define bmlite_on_start_capture() 
  #define bmlite_on_finish_capture() 
  #define bmlite_on_finish_enroll() 
  #define bmlite_on_start_enroll() 
  #define bmlite_on_start_enrollcapture() 
  #define bmlite_on_finish_enrollcapture() 
  #define bmlite_on_identify_start() 
  #define bmlite_on_identify_finish() 

#else

typedef enum {
    BMLITE_ERROR_OK = 0,
    BMLITE_ERROR_CAPTURE,
    BMLITE_ERROR_CAPTURE_START,
    BMLITE_ERROR_ENROLL_START,
    BMLITE_ERROR_ENROLL_ADD,
    BMLITE_ERROR_ENROLL_FINISH,
    BMLITE_ERROR_WRONG_ANSWER,
    BMLITE_ERROR_FINGER_WAIT,
    BMLITE_ERROR_IDENTYFY,
    BMLITE_ERROR_TEMPLATE_SAVE,
    BMLITE_ERROR_TEMPLATE_DELETE,
    BMLITE_ERROR_TEMPLATE_COUNT,
    BMLITE_ERROR_TEMPLATE_GETIDS,
    BMLITE_ERROR_IMAGE_EXTRACT,
    BMLITE_ERROR_IMAGE_GETSIZE,
    BMLITE_ERROR_IMAGE_GET,
    BMLITE_ERROR_GETVERSION,
    BMLITE_ERROR_SW_RESET,
    BMLITE_ERROR_CALIBRATE,
    BMLITE_ERROR_CALIBRATE_DELETE,
    BMLITE_ERROR_SEND_CMD,
    BMLITE_ERROR_GET_ARG,
} bmlite_error_t;

/**
 * @brief Error Callback function
 *
 * @param[in] Callback Error Code
 * @param[in] BEP result code
 */
void bmlite_on_error(bmlite_error_t error, int32_t value);

/**
 * @brief Starting Capture Callback function
 */
void bmlite_on_start_capture();

/**
 * @brief Finishing Capture Callback function
 */
void bmlite_on_finish_capture();

/**
 * @brief Starting Enroll Callback function
 */
void bmlite_on_start_enroll();

/**
 * @brief Finishing Enroll Callback function
 */
void bmlite_on_finish_enroll();

/**
 * @brief Starting Capture for Enroll Callback function
 */
void bmlite_on_start_enrollcapture();

/**
 * @brief Finishing Capture for Enroll Callback function
 */
void bmlite_on_finish_enrollcapture();

/**
 * @brief Starting Identify Callback function
 */
void bmlite_on_identify_start();

/**
 * @brief Finishing Identify Callback function
 */
void bmlite_on_identify_finish();
#endif    // BMLITE_USE_CALLBACK

#endif