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

#include <string.h>

#include "fpc_crc.h"
#include "hcp_tiny.h"
#include "bmlite_if_callbacks.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#define LOG_DEBUG(...) printf(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

static uint32_t fpc_com_ack = FPC_BEP_ACK;

static fpc_bep_result_t _rx_link(HCP_comm_t *hcp_comm);
static fpc_bep_result_t _tx_link(HCP_comm_t *hcp_comm);

typedef struct {
    uint16_t cmd;
    uint16_t args_nr;
    uint8_t args[];
} _HCP_cmd_t;

typedef struct {
    uint16_t arg;
    uint16_t size;
    uint8_t pld[];
} _CMD_arg_t;

typedef struct {
    uint16_t lnk_chn;
    uint16_t lnk_size;
    uint16_t t_size;
    uint16_t t_seq_nr;
    uint16_t t_seq_len;
   _HCP_cmd_t t_pld;
} _HPC_pkt_t;

fpc_bep_result_t bmlite_init_cmd(HCP_comm_t *hcp_comm, uint16_t cmd, uint16_t arg_key)
{
    fpc_bep_result_t bep_result;

    _HCP_cmd_t *out = (_HCP_cmd_t *)hcp_comm->pkt_buffer;
    out->cmd = cmd;
    out->args_nr = 0;
    hcp_comm->pkt_size = 4;

    if(arg_key != ARG_NONE) {
        bep_result = bmlite_add_arg(hcp_comm, arg_key, NULL, 0);
        if(bep_result) {
            bmlite_on_error(BMLITE_ERROR_SEND_CMD, bep_result);
            return bep_result;
        }
    }    

    return FPC_BEP_RESULT_OK;
}

fpc_bep_result_t bmlite_add_arg(HCP_comm_t *hcp_comm, uint16_t arg_type, void *arg_data, uint16_t arg_size)
{
    if(hcp_comm->pkt_size + 4 + arg_size > hcp_comm->pkt_size_max) {
        bmlite_on_error(BMLITE_ERROR_SEND_CMD, FPC_BEP_RESULT_NO_MEMORY);
        return FPC_BEP_RESULT_NO_MEMORY;
    }

    ((_HCP_cmd_t *)hcp_comm->pkt_buffer)->args_nr++;
    _CMD_arg_t *args = (_CMD_arg_t *)(&hcp_comm->pkt_buffer[hcp_comm->pkt_size]);
    args->arg = arg_type;
    args->size = arg_size;
    if(arg_size) {
        memcpy(&args->pld, arg_data, arg_size);
    }
    hcp_comm->pkt_size += 4 + arg_size;
    return FPC_BEP_RESULT_OK;
}

fpc_bep_result_t bmlite_get_arg(HCP_comm_t *hcp_comm, uint16_t arg_type)
{
    uint16_t i = 0;
    uint8_t *buffer = hcp_comm->pkt_buffer;
    uint16_t args_nr = ((_HCP_cmd_t *)(buffer))->args_nr;
    uint8_t *pdata = (uint8_t *)&((_HCP_cmd_t *)(buffer))->args;
    while (i < args_nr && (uint32_t)(pdata - buffer) <= hcp_comm->pkt_size) {
        _CMD_arg_t *parg = (_CMD_arg_t *)pdata;
        if(parg->arg == arg_type) {
            hcp_comm->arg.size = parg->size;
            hcp_comm->arg.data = parg->pld;
            return FPC_BEP_RESULT_OK;
        } else {
            i++;
            pdata += 4 + parg->size;
        }
    }

    // Ignore missing ARG_RESULT because some command return result other way
    // if (arg_type != ARG_RESULT) {
        bmlite_on_error(BMLITE_ERROR_GET_ARG, FPC_BEP_RESULT_INVALID_ARGUMENT);
    // }
    return FPC_BEP_RESULT_INVALID_ARGUMENT;
}

fpc_bep_result_t bmlite_copy_arg(HCP_comm_t *hcp_comm, uint16_t arg_key, void *arg_data, uint16_t arg_data_size)
{
    fpc_bep_result_t bep_result;
    bep_result = bmlite_get_arg(hcp_comm, arg_key);
    if(bep_result == FPC_BEP_RESULT_OK) {
        if(arg_data == NULL) {
            bmlite_on_error(BMLITE_ERROR_GET_ARG, FPC_BEP_RESULT_NO_MEMORY);
            return FPC_BEP_RESULT_NO_MEMORY;
        }
        memcpy(arg_data, hcp_comm->arg.data, HCP_MIN(arg_data_size, hcp_comm->arg.size));
    } else {
        bmlite_on_error(BMLITE_ERROR_GET_ARG, FPC_BEP_RESULT_INVALID_ARGUMENT);
        return FPC_BEP_RESULT_INVALID_ARGUMENT;
    }

    return bep_result;
}

fpc_bep_result_t bmlite_tranceive(HCP_comm_t *hcp_comm)
{
    fpc_bep_result_t bep_result;

    bep_result = bmlite_send(hcp_comm);
    if (bep_result == FPC_BEP_RESULT_OK) {
        bep_result = bmlite_receive(hcp_comm);

        if (bmlite_get_arg(hcp_comm, ARG_RESULT) == FPC_BEP_RESULT_OK) {
            hcp_comm->bep_result = (fpc_bep_result_t)*(int8_t*)hcp_comm->arg.data;
        } else {
            hcp_comm->bep_result = FPC_BEP_RESULT_OK;
        }
    }

    return bep_result;
}

fpc_bep_result_t bmlite_receive(HCP_comm_t *hcp_comm)
{
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;
    fpc_bep_result_t com_result = FPC_BEP_RESULT_OK;
    uint16_t seq_nr = 0;
    uint16_t seq_len = 1;
    uint8_t *p = hcp_comm->pkt_buffer;
    _HPC_pkt_t *pkt = (_HPC_pkt_t *)hcp_comm->txrx_buffer;
    uint16_t buf_len = 0;

    while(seq_nr < seq_len) {
        bep_result = _rx_link(hcp_comm);

        if (!bep_result) {
            seq_nr = pkt->t_seq_nr;
            seq_len = pkt->t_seq_len;
            if(pkt->t_size != pkt->lnk_size - 6) {
                com_result = FPC_BEP_RESULT_IO_ERROR;
                continue;
            }
            if(buf_len + pkt->t_size < hcp_comm->pkt_size_max) {
                memcpy(p, &pkt->t_pld, pkt->t_size);
                p += pkt->t_size;
                buf_len += pkt->t_size;
            } else {
                com_result = FPC_BEP_RESULT_NO_MEMORY;
            }
#ifdef DEBUG            
            if (seq_len > 1)
                LOG_DEBUG("Received data chunk %d of %d\n", seq_nr, seq_len);
#endif
        } else {
            bmlite_on_error(BMLITE_ERROR_SEND_CMD, bep_result);
            return bep_result;
        }
    }

    hcp_comm->pkt_size = buf_len;
    if(com_result != FPC_BEP_RESULT_OK) {
        bmlite_on_error(BMLITE_ERROR_SEND_CMD, com_result);
    }
    return com_result;
}

static fpc_bep_result_t _rx_link(HCP_comm_t *hcp_comm)
{
    // Get size, msg and CRC
    fpc_bep_result_t result = hcp_comm->read(4, hcp_comm->txrx_buffer, hcp_comm->phy_rx_timeout);
    _HPC_pkt_t *pkt = (_HPC_pkt_t *)hcp_comm->txrx_buffer;
    uint16_t size;

    if (result) {
        LOG_DEBUG("Timed out waiting for response.\n");
        return result;
    }

    size = pkt->lnk_size;

    // Check if size plus header and crc is larger than max package size.
    if (MTU < size + 8) {
        // LOG_DEBUG("S: Invalid size %d, larger than MTU %d.\n", size, MTU);
        bmlite_on_error(BMLITE_ERROR_SEND_CMD, FPC_BEP_RESULT_IO_ERROR);
        return FPC_BEP_RESULT_IO_ERROR;
    }
        
    hcp_comm->read(size + 4, hcp_comm->txrx_buffer + 4, 100);

    uint32_t crc = *(uint32_t *)(hcp_comm->txrx_buffer + 4 + size);
    uint32_t crc_calc = fpc_crc(0, hcp_comm->txrx_buffer+4, size);

    if (crc_calc != crc) {
        LOG_DEBUG("CRC mismatch. Calculated %04X, received %04X\n", 
                               (unsigned int)crc_calc, (unsigned int)crc);
        bmlite_on_error(BMLITE_ERROR_SEND_CMD, FPC_BEP_RESULT_IO_ERROR);
        return FPC_BEP_RESULT_IO_ERROR;
    }

    // Send Ack
    hcp_comm->write(4, (uint8_t *)&fpc_com_ack, 0);

    return FPC_BEP_RESULT_OK;
}

fpc_bep_result_t bmlite_send(HCP_comm_t *hcp_comm)
{
    uint16_t seq_nr = 1;
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;
    uint16_t data_left = hcp_comm->pkt_size;
    uint8_t *p = hcp_comm->pkt_buffer;

    _HPC_pkt_t *phy_frm = (_HPC_pkt_t *)hcp_comm->txrx_buffer;

    // Application MTU size is PHY MTU - (Transport and Link overhead)
    uint16_t app_mtu = MTU - 6 - 8;

    // Calculate sequence length
    uint16_t seq_len = (data_left / app_mtu) + 1;

    phy_frm->lnk_chn = 0;
    phy_frm->t_seq_len = seq_len;

    for (seq_nr = 1; seq_nr <= seq_len && !bep_result; seq_nr++) {
        phy_frm->t_seq_nr = seq_nr;
        if (data_left < app_mtu) {
            phy_frm->t_size = data_left;
        } else {
            phy_frm->t_size = app_mtu;
        }
        memcpy(&phy_frm->t_pld, p, phy_frm->t_size);
        phy_frm->lnk_size = phy_frm->t_size + 6;
        p += phy_frm->t_size;
        data_left -= phy_frm->t_size;

        bep_result = _tx_link(hcp_comm);
    }

    if(bep_result) {
        bmlite_on_error(BMLITE_ERROR_SEND_CMD, bep_result);
    }
    return bep_result;
}

static fpc_bep_result_t _tx_link(HCP_comm_t *hcp_comm)
{
    fpc_bep_result_t bep_result;

    _HPC_pkt_t *pkt = (_HPC_pkt_t *)hcp_comm->txrx_buffer;

    uint32_t crc_calc = fpc_crc(0, &pkt->t_size, pkt->lnk_size);
    *(uint32_t *)(hcp_comm->txrx_buffer + pkt->lnk_size + 4) = crc_calc;
    uint16_t size = pkt->lnk_size + 8;

    bep_result = hcp_comm->write(size, hcp_comm->txrx_buffer, 0);

    // Wait for ACK
    uint32_t ack;
    bep_result = hcp_comm->read(4, (uint8_t *)&ack, 500);
    if (bep_result == FPC_BEP_RESULT_TIMEOUT) {
        LOG_DEBUG("ASK read timeout\n");
        bmlite_on_error(BMLITE_ERROR_SEND_CMD, FPC_BEP_RESULT_TIMEOUT);
        return FPC_BEP_RESULT_IO_ERROR;
    }

    if(ack != fpc_com_ack) {
        return FPC_BEP_RESULT_IO_ERROR;
    }

    return FPC_BEP_RESULT_OK;
}

