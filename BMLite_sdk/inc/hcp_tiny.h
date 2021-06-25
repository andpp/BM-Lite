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

#ifndef HCP_H
#define HCP_H

#include "fpc_bep_types.h"
#include "fpc_hcp_common.h"

/** MTU for HCP physical layer */
#define MTU 256

/** Communication acknowledge definition */
#define FPC_BEP_ACK 0x7f01ff7f

typedef struct {
    uint32_t size;
    uint8_t *data;
} HCP_arg_t;

typedef struct {
    /** Send data to BM-Lite */
    fpc_bep_result_t (*write) (uint16_t, const uint8_t *, uint32_t);  
    /** Receive data from BM-Lite */
    fpc_bep_result_t (*read)(uint16_t, uint8_t *, uint32_t);
    /** Receive timeout (msec). Applys ONLY to receiving packet from BM-Lite on physical layer */
    uint32_t phy_rx_timeout;
    /** Data buffer for application layer */
    uint8_t *pkt_buffer;
    /** Size of data buffer */
    uint32_t pkt_size_max;
    /** Current size of incoming or outcoming command packet */
    uint32_t pkt_size;
    /** Buffer of MTU size for transport layer */
    uint8_t *txrx_buffer;
    /** Values of last argument pulled by bmlite_get_arg 
        Values are valid only right after bmlite_get_arg() call */
    HCP_arg_t arg;
    /** Result of execution command on BM-Lite */
    fpc_bep_result_t bep_result;
} HCP_comm_t;

/**
 * @brief Send prepared command packet to FPC BM-LIte
 * 
 * @param[in] hcp_comm - pointer to HCP_comm struct
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_send(HCP_comm_t *hcp_comm);

/**
 * @brief Receive answer from FPC BM-LIte
 * 
 * @param[in] hcp_comm - pointer to HCP_comm struct
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_receive(HCP_comm_t *hcp_comm);

/**
 * @brief Send prepared command packet to FPC BM-LIte and receive answer
 * 
 * @param[in] hcp_comm - pointer to HCP_comm struct
 * 
 *   Returns result of executing command in BM-LIte in hcp_comm->bep_result
 *   if communication with BM-Lite was successful.
 *   Please not that some BM-Lite command does not return result in ARG_RESULT.
 *   They return data with some other argument instead.
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_tranceive(HCP_comm_t *hcp_comm);

/**
 * @brief Initialize new command for BM-Lite
 *
 * @param[in] hcp_comm     - pointer to HCP_comm struct
 * @param[in] cmd          - command to send
 * @param[in] arg          - Argument for the command without parameterd
 *                           Use ARG_NONE and add arguments by bmlite_add_arg() if 
     *                       you need to add argument with parameter
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_init_cmd(HCP_comm_t *hcp_comm, uint16_t cmd, uint16_t arg);

/**
 * @brief  Add argument to command. 
 *         Must be used only after command buffer is initialized by bmlite_init_cmd()
 * 
 * @param[in] hcp_comm     - pointer to HCP_comm struct
 * @param[in] arg_type     - argument key
 * @param[in] arg_data     - argument data
 * @param[in] arg_size     - argument data length
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_add_arg(HCP_comm_t *hcp_comm, uint16_t arg_type, void *arg_data, uint16_t arg_size);

/**
 * @brief  Search for argument in received answer. 
 * 
 * @param[in] hcp_comm     - pointer to HCP_comm struct
 * @param[in] arg_type     - argument key
 * 
 *  If found, place pointer to argument data in receiving buffer to hcp_comm->arg.data
 *  and size of the argument in hcp_comm->arg.size
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_get_arg(HCP_comm_t *hcp_comm, uint16_t arg_type);

/**
 * @brief  Search for argument in received answer and copy argument's data
 *         to arg_data 
 * 
 * @param[in] hcp_comm        - pointer to HCP_comm struct
 * @param[in] arg_type        - argument key
 * @param[out] arg_data       - pointer for memory to copy argument value
 * @param[out] arg_data_size  - size of data area for copying argument value
 * 
 *  If found, argument's data will be copyed to arg_data
 *  If received argument's size greater that arg_data_size, the copyed data will be
 *  truncated to arg_data_size. 
 *  Still hcp_comm->arg.data will be pointed to argument's data in receiving buffer
 *  and real size of the argument will be hcp_comm->arg.size
 * 
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t bmlite_copy_arg(HCP_comm_t *hcp_comm, uint16_t arg_key, void *arg_data, uint16_t arg_data_size);

#endif 