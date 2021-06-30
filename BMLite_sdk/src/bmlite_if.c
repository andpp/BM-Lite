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

#include "hcp_tiny.h"
#include "bmlite_if.h"
#include "bmlite_hal.h"
#include <stdio.h>

#include "bmlite_if_callbacks.h"

#define MAX_CAPTURE_ATTEMPTS 15
#define MAX_SINGLE_CAPTURE_ATTEMPTS 3
#define CAPTURE_TIMEOUT 3000

#define exit_if_err(c) { bep_result = c; if(bep_result || chain->bep_result) goto exit; }

#define assert(c)  { fpc_bep_result_t res = c; if(res) return res; }

#ifdef BMLITE_USE_CALLBACK
/**
 * @brief Mock callback functions
 */
__attribute__((weak)) void bmlite_on_error(bmlite_error_t error, int32_t value) { (void)error; (void)value; }

__attribute__((weak)) void bmlite_on_start_capture() {}
__attribute__((weak)) void bmlite_on_finish_capture() {}

__attribute__((weak)) void bmlite_on_start_enroll() {}
__attribute__((weak)) void bmlite_on_finish_enroll() {}

__attribute__((weak)) void bmlite_on_start_enrollcapture() {}
__attribute__((weak)) void bmlite_on_finish_enrollcapture() {}

__attribute__((weak)) void bmlite_on_identify_start() {}
__attribute__((weak)) void bmlite_on_identify_finish() {}
#endif

fpc_bep_result_t bep_enroll_finger(HCP_comm_t *chain)
{
    uint32_t samples_remaining = 0;
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;
    bool enroll_done = false;

    bmlite_on_start_enroll();
    /* Enroll start */
    exit_if_err(bmlite_send_cmd(chain, CMD_ENROLL, ARG_START));
    
    for (uint8_t i = 0; i < MAX_CAPTURE_ATTEMPTS; ++i) {

        bmlite_on_start_enrollcapture();
        bep_result = bep_capture(chain, CAPTURE_TIMEOUT);
        bmlite_on_finish_enrollcapture();

        if (bep_result != FPC_BEP_RESULT_OK) {
            continue;
        }

        /* Enroll add */
        bep_result = bmlite_send_cmd(chain, CMD_ENROLL, ARG_ADD);
        if (bep_result != FPC_BEP_RESULT_OK) {
            continue;
        }

        bmlite_get_arg(chain, ARG_COUNT);
        samples_remaining = *(uint32_t *)chain->arg.data;
//      DEBUG("Enroll samples remaining: %d\n", samples_remaining);

        /* Break enrolling if we can't collect enough correct images for enroll*/
        if (samples_remaining == 0U) {
            enroll_done = true;
            break;
        }

        sensor_wait_finger_not_present(chain, 0);
    }

    bep_result = bmlite_send_cmd(chain, CMD_ENROLL, ARG_FINISH);

exit:
    bmlite_on_finish_enroll();
    return (!enroll_done) ? FPC_BEP_RESULT_GENERAL_ERROR : bep_result;
}

fpc_bep_result_t bep_identify_finger(HCP_comm_t *chain, uint32_t timeout, uint16_t *template_id, bool *match)
{
    fpc_bep_result_t bep_result;
    *match = false;

    bmlite_on_identify_start();

    exit_if_err(bep_capture(chain, timeout));
    exit_if_err(bep_image_extract(chain));
    exit_if_err(bep_identify(chain));
    exit_if_err(bmlite_get_arg(chain, ARG_MATCH));
    *match = *(bool *)chain->arg.data;
    if(*match) {
        bmlite_get_arg(chain, ARG_ID);
        *template_id = *(uint16_t *)chain->arg.data;
        // Delay for possible updating template on BM-Lite
        hal_timebase_busy_wait(50);
    }
exit:
    bmlite_on_identify_finish();
    return bep_result;    
}

fpc_bep_result_t sensor_wait_finger_present(HCP_comm_t *chain, uint16_t timeout)
{
    fpc_bep_result_t bep_result;
    uint32_t prev_timeout = chain->phy_rx_timeout;

    bmlite_on_start_capture();
    chain->phy_rx_timeout = timeout;
    bep_result = bmlite_send_cmd_arg(chain, CMD_WAIT, ARG_FINGER_DOWN, ARG_TIMEOUT, &timeout, sizeof(timeout));
    chain->phy_rx_timeout = prev_timeout;
    bmlite_on_finish_capture();

    return bep_result;
}

fpc_bep_result_t sensor_wait_finger_not_present(HCP_comm_t *chain, uint16_t timeout)
{
    fpc_bep_result_t bep_result;
    uint32_t prev_timeout = chain->phy_rx_timeout;

    chain->phy_rx_timeout = timeout;
    bep_result = bmlite_send_cmd_arg(chain, CMD_WAIT, ARG_FINGER_UP, ARG_TIMEOUT, &timeout, sizeof(timeout));
    chain->phy_rx_timeout = prev_timeout;

    return bep_result;
}

fpc_bep_result_t bep_capture(HCP_comm_t *chain, uint16_t timeout)
{
    fpc_bep_result_t bep_result;
    uint32_t prev_timeout = chain->phy_rx_timeout;

    bmlite_on_start_capture();
    chain->phy_rx_timeout = timeout;
    for(int i=0; i< MAX_SINGLE_CAPTURE_ATTEMPTS; i++) {
        bep_result = bmlite_send_cmd_arg(chain, CMD_CAPTURE, ARG_NONE, ARG_TIMEOUT, &timeout, sizeof(timeout));
        if(bep_result == FPC_BEP_RESULT_IO_ERROR ||
           bep_result == FPC_BEP_RESULT_TIMEOUT) {
            break;
        }
        if( !(bep_result || chain->bep_result))
            break;
    }
    chain->phy_rx_timeout = prev_timeout;
    bmlite_on_finish_capture();

    return bep_result;
}

fpc_bep_result_t bep_image_get_size(HCP_comm_t *chain, uint32_t *size)
{
    assert(bmlite_send_cmd(chain, CMD_IMAGE, ARG_SIZE));
    assert(bmlite_get_arg(chain, ARG_SIZE));

    *size = *(uint32_t*)chain->arg.data;

    return FPC_BEP_RESULT_OK;
}

fpc_bep_result_t image_create(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_IMAGE, ARG_CREATE);
}

fpc_bep_result_t image_delete(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_IMAGE, ARG_DELETE);
}

fpc_bep_result_t bep_image_get(HCP_comm_t *chain, uint8_t *data, uint32_t size)
{
    assert(bmlite_send_cmd(chain, CMD_IMAGE, ARG_UPLOAD));
    return bmlite_copy_arg(chain, ARG_DATA, data, size);
}

fpc_bep_result_t bep_image_put(HCP_comm_t *chain, uint8_t *data, uint32_t size)
{
    return bmlite_send_cmd_arg(chain, CMD_IMAGE, ARG_DOWNLOAD, ARG_DATA, data, size);
}

fpc_bep_result_t bep_image_extract(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_IMAGE, ARG_EXTRACT);
}

fpc_bep_result_t bep_identify(HCP_comm_t *chain)
{
   return bmlite_send_cmd(chain, CMD_IDENTIFY, ARG_NONE);
}

fpc_bep_result_t bep_template_save(HCP_comm_t *chain, uint16_t template_id)
{
    return bmlite_send_cmd_arg(chain, CMD_TEMPLATE, ARG_SAVE, ARG_ID, &template_id, sizeof(template_id));
}

fpc_bep_result_t bep_template_remove_ram(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_TEMPLATE, ARG_DELETE);
}

fpc_bep_result_t bep_template_get(HCP_comm_t *chain, uint8_t *data, uint32_t size)
{
    assert(bmlite_send_cmd(chain, CMD_TEMPLATE, ARG_UPLOAD));
    return bmlite_copy_arg(chain, ARG_DATA, data, size);
}

fpc_bep_result_t bep_template_put(HCP_comm_t *chain, uint8_t *data, uint16_t length)
{
    return bmlite_send_cmd_arg(chain, CMD_TEMPLATE, ARG_DOWNLOAD,
           ARG_DATA, data, length);
}

fpc_bep_result_t bep_template_remove(HCP_comm_t *chain, uint16_t template_id)
{
    return bmlite_send_cmd_arg(chain, CMD_STORAGE_TEMPLATE, ARG_DELETE, 
            ARG_ID, &template_id, sizeof(template_id));
}

fpc_bep_result_t bep_template_remove_all(HCP_comm_t *chain)
{
    return bmlite_send_cmd_arg(chain, CMD_STORAGE_TEMPLATE, ARG_DELETE,
             ARG_ALL, 0, 0);
}

fpc_bep_result_t bep_template_load_storage(HCP_comm_t *chain, uint16_t template_id)
{
   return bmlite_send_cmd_arg(chain, CMD_STORAGE_TEMPLATE, ARG_UPLOAD,   
            ARG_ID, &template_id, sizeof(template_id));
}

fpc_bep_result_t bep_template_get_count(HCP_comm_t *chain, uint16_t *count)
{
    assert(bmlite_send_cmd(chain, CMD_STORAGE_TEMPLATE, ARG_COUNT));
    assert(bmlite_get_arg(chain, ARG_COUNT));
    *count = *(uint16_t*)chain->arg.data;
    return FPC_BEP_RESULT_OK;
}

fpc_bep_result_t bep_template_get_ids(HCP_comm_t *chain)
{
    assert(bmlite_send_cmd(chain, CMD_STORAGE_TEMPLATE, ARG_ID));
    return bmlite_get_arg(chain, ARG_DATA);
}

fpc_bep_result_t bep_sw_reset(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_RESET, ARG_NONE);
}

fpc_bep_result_t bep_sensor_calibrate(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_STORAGE_CALIBRATION, ARG_NONE);
}

fpc_bep_result_t bep_sensor_calibrate_remove(HCP_comm_t *chain)
{
    return bmlite_send_cmd(chain, CMD_STORAGE_CALIBRATION, ARG_DELETE);    
}

fpc_bep_result_t bep_version(HCP_comm_t *chain, char *version, int len)
{
    assert(bmlite_send_cmd_arg(chain, CMD_INFO, ARG_GET, ARG_VERSION, 0, 0));
    return bmlite_copy_arg(chain, ARG_VERSION, version, len);
}

fpc_bep_result_t bep_unique_id_get(HCP_comm_t *chain, uint8_t *unique_id)
{
    assert(bmlite_send_cmd_arg(chain, CMD_INFO, ARG_GET, ARG_UNIQUE_ID, 0, 0));
    return bmlite_copy_arg(chain, ARG_UNIQUE_ID, unique_id, 12);
}

fpc_bep_result_t bep_uart_speed_set(HCP_comm_t *chain, uint32_t speed)
{
    assert(bmlite_init_cmd(chain, CMD_COMMUNICATION, ARG_SPEED));
    assert(bmlite_add_arg(chain, ARG_SET, 0, 0));
    assert(bmlite_add_arg(chain, ARG_DATA, (uint8_t*)&speed, sizeof(speed)));
    return bmlite_tranceive(chain);

}

fpc_bep_result_t bep_uart_speed_get(HCP_comm_t *chain, uint32_t *speed)
{
    assert(bmlite_init_cmd(chain, CMD_COMMUNICATION, ARG_SPEED));
    assert(bmlite_add_arg(chain, ARG_GET, 0, 0));
    assert(bmlite_tranceive(chain));
    return bmlite_copy_arg(chain, ARG_DATA, speed, sizeof(speed));

}

fpc_bep_result_t bep_sensor_reset(HCP_comm_t *chain)
{
    // Delay for possible updating template on BM-Lite
    hal_timebase_busy_wait(50);

    return bmlite_send_cmd(chain, CMD_SENSOR, ARG_RESET);    
}

fpc_bep_result_t bmlite_send_cmd(HCP_comm_t *chain, uint16_t cmd, uint16_t arg_type)
{
    assert(bmlite_init_cmd(chain, cmd, arg_type));
    return bmlite_tranceive(chain);
}

fpc_bep_result_t bmlite_send_cmd_arg(HCP_comm_t *chain, uint16_t cmd, uint16_t arg1_type, uint16_t arg2_type, void *arg2_data, uint16_t arg2_length)
{
    assert(bmlite_init_cmd(chain, cmd, arg1_type));
    assert(bmlite_add_arg(chain, arg2_type, arg2_data, arg2_length));

    return bmlite_tranceive(chain);
}
