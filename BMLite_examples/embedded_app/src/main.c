/*
 * Copyright (c) 2020 Fingerprint Cards AB
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
 *
 * Modified by Andrey Perminov <andrey.ppp@gmail.com> 
 * for running on microcontrollers
 */


/**
 * @file    main.c
 * @brief   Main file for FPC BM-Lite Communication example.
 */

#include <unistd.h>
#include <string.h>

#include "bmlite_if.h"
#include "hcp_tiny.h"
#include "platform.h"
#include "bmlite_hal.h"

#define DATA_BUFFER_SIZE (1024*5)
static uint8_t hcp_txrx_buffer[MTU];
static uint8_t hcp_data_buffer[DATA_BUFFER_SIZE];

static HCP_comm_t hcp_chain = {
    .read = platform_bmlite_receive,
    .write = platform_bmlite_send,
    .pkt_buffer = hcp_data_buffer,
    .txrx_buffer = hcp_txrx_buffer,
    .pkt_size = 0,
    .pkt_size_max = sizeof(hcp_data_buffer),
    .phy_rx_timeout = 2000,
};

void bmlite_on_error(bmlite_error_t error, int32_t value)
{
    if(value != FPC_BEP_RESULT_TIMEOUT) {
        hal_set_leds(BMLITE_LED_STATUS_ERROR, false);
    } else {
        // Timeout - not really an error here
        hal_set_leds(BMLITE_LED_STATUS_ERROR, true);
    }
}

// void bmlite_on_start_capture();
// void bmlite_on_finish_capture();

void bmlite_on_start_enroll()
{
    hal_set_leds(BMLITE_LED_STATUS_ENROLL, true);
}

void bmlite_on_finish_enroll()
{
    hal_set_leds(BMLITE_LED_STATUS_ENROLL, false);
}

void bmlite_on_start_enrollcapture()
{
    hal_set_leds(BMLITE_LED_STATUS_WAITTOUCH, true);
}

void bmlite_on_finish_enrollcapture()
{
    hal_set_leds(BMLITE_LED_STATUS_READY, false);
}

void bmlite_on_identify_start()
{
    hal_set_leds(BMLITE_LED_STATUS_READY, true);
}

// void bmlite_on_identify_finish();


int main (int argc, char **argv)
{
    platform_init(NULL);

    {
        char version[100];
        uint16_t template_id;
        uint32_t current_id = 0;
        bool match;

        // These two lines for debug purpose only 
        memset(version, 0, 100);
        fpc_bep_result_t res = bep_version(&hcp_chain, version, 99);

        while (1)
        {
            uint32_t btn_time = hal_get_button_press_time();
            hal_set_leds(BMLITE_LED_STATUS_READY,0);
            if (btn_time < 200) {
                // nothing hapened
            } else if (btn_time < 5000) {
                // Enroll
                res = bep_enroll_finger(&hcp_chain);
                res = bep_template_save(&hcp_chain, current_id++);
            } else {
                // Erase All templates
                hal_set_leds(BMLITE_LED_STATUS_DELETE_TEMPLATES, true);
                res = bep_template_remove_all(&hcp_chain);
                current_id = 0;
            }
            res = bep_identify_finger(&hcp_chain, 0, &template_id, &match);
            if (res == FPC_BEP_RESULT_TIMEOUT) {
                platform_bmlite_reset();
            } else if (res != FPC_BEP_RESULT_OK) {
                continue;
            }
            hal_set_leds(BMLITE_LED_STATUS_MATCH, match);
            res = sensor_wait_finger_not_present(&hcp_chain, 0);

        }
    }
}
