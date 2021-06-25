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
 * for FPC BM-Lite applications 
 *
 * 04/14/2020: Added SPI interface support
 */


/**
 * @file    main.c
 * @brief   Main file for FPC BM-Lite Communication example.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "bmlite_if.h"
#include "hcp_tiny.h"
#include "platform.h"
#include "bmlite_hal.h"
#include "platform_rpi.h"

#define DATA_BUFFER_SIZE 102400
static uint8_t hcp_txrx_buffer[MTU];
static uint8_t hcp_data_buffer[DATA_BUFFER_SIZE];

static HCP_comm_t hcp_chain = {
    .write = platform_bmlite_spi_send,
    .read = platform_bmlite_spi_receive,
    .phy_rx_timeout = 2000,
    .pkt_buffer = hcp_data_buffer,
    .pkt_size_max = sizeof(hcp_data_buffer),
    .pkt_size = 0,
    .txrx_buffer = hcp_txrx_buffer,
};

static void help(void)
{
    fprintf(stderr, "BEP Host Communication Application\n");
    fprintf(stderr, "Syntax: bep_host_com [-s] [-p port] [-b baudrate] [-t timeout]\n");
}

void bmlite_on_error(bmlite_error_t error, int32_t value) 
{ 
    printf("Error: %d, return code %d\n", error, (int16_t)value); 
}

void bmlite_on_start_capture() 
{
    printf("Put finger on the sensor\n");
}
void bmlite_on_finish_capture() 
{
    printf("Remove finger from the sensor\n");
}

void bmlite_on_start_enroll() 
{
    printf("Start enrolling\n");
}

void bmlite_on_finish_enroll() 
{
    printf("Finish enrolling\n");
}

void bmlite_on_start_enrollcapture() {}
void bmlite_on_finish_enrollcapture() {}

void bmlite_on_identify_start() 
{
    printf("Start Identifying\n");
}
void bmlite_on_identify_finish() 
{
    printf("Finish Identifying\n");
}

void save_to_pgm(FILE *f, uint8_t *image, int res_x, int res_y)
{
        /* Print 8-bpp PGM ASCII header */
        fprintf(f, "P2\n%d %d\n255\n", res_x, res_y);
        for (int y = 0; y < res_y; y++) {
                for (int x = 0; x < res_x; x++, image++)
                        fprintf(f,"%d ", *image);
                fprintf(f,"\n");
        }

        fprintf(f,"\x04"); /* End Of Transmission */
}


int main (int argc, char **argv)
{
    int index;
    int c;
    rpi_initparams_t rpi_params;
    
    rpi_params.iface = SPI_INTERFACE;
    rpi_params.hcp_comm = &hcp_chain;
    rpi_params.baudrate = 921600;
    rpi_params.timeout = 5;
    rpi_params.port = NULL;

    opterr = 0;

    while ((c = getopt (argc, argv, "sb:p:t:")) != -1) {
        switch (c) {
            case 's':
                rpi_params.iface = SPI_INTERFACE;
                if(rpi_params.baudrate == 921600)
                    rpi_params.baudrate = 1000000;
                break;
            case 'b':
                rpi_params.baudrate = atoi(optarg);
                break;
            case 'p':
                rpi_params.iface = COM_INTERFACE;
                rpi_params.port = optarg;
                break;
            case 't':
                rpi_params.timeout = atoi(optarg);
                break;
            case '?':
                if (optopt == 'b')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                help();
                exit(1);
            }
        }

    if (rpi_params.iface == COM_INTERFACE && rpi_params.port == NULL) {
        printf("port must be specified\n");
        help();
        exit(1);
    }

    for (index = optind; index < argc; index++) {
        printf ("Non-option argument %s\n", argv[index]);
    }

    if(platform_init(&rpi_params) != FPC_BEP_RESULT_OK) {
        help();
        exit(1);
    }

    while(1) {
        char cmd[100];
        fpc_bep_result_t res = FPC_BEP_RESULT_OK;
        uint16_t template_id;
        bool match;

        rpi_clear_screen();
        printf("BM-Lite Interface\n");
        if (rpi_params.iface == SPI_INTERFACE)
        	printf("SPI port: speed %d Hz\n", rpi_params.baudrate);
        else
            printf("Com port: %s [speed: %d]\n", rpi_params.port, rpi_params.baudrate);
        printf("Timeout: %ds\n", rpi_params.timeout);
        printf("-------------------\n\n");
        printf("Possible options:\n");
        printf("a: Enroll finger\n");
        printf("b: Capture and identify finger\n");
        printf("c: Remove all templates\n");
        printf("d: Save template\n");
        printf("e: Remove template\n");
        printf("l: List of templates\n");
        printf("t: Save template to file\n");
        printf("T: Push template from file\n");
        printf("f: Capture image\n");
        printf("g: Pull captured image\n");
        printf("h: Get version\n");
        printf("r: SW Reset\n");
        printf("q: Exit program\n");
        printf("\nOption>> ");
        fgets(cmd, sizeof(cmd), stdin);
        switch (cmd[0]) {
            case 'a':
                res = bep_enroll_finger(&hcp_chain);
                break;
            case 'b':
                res = bep_identify_finger(&hcp_chain, 0, &template_id, &match);
                if (res == FPC_BEP_RESULT_OK) {
                    if (match) {
                        printf("Match with template id: %d\n", template_id);
                    } else {
                        printf("No match\n");
                    }
                }
                break;
            case 'c':
                res = bep_template_remove_all(&hcp_chain);
                break;
            case 'd':
                printf("Template id: ");
                fgets(cmd, sizeof(cmd), stdin);
                template_id = atoi(cmd);
                res = bep_template_save(&hcp_chain, template_id);
                // res = bep_template_remove_ram(&hcp_chain);
                break;
            case 'e':
                printf("Template id: ");
                fgets(cmd, sizeof(cmd), stdin);
                template_id = atoi(cmd);
                res = bep_template_remove(&hcp_chain, template_id);
                break;
            case 'l':
                res = bep_template_get_ids(&hcp_chain);
                if (hcp_chain.bep_result == FPC_BEP_RESULT_OK) {
                    printf("Template list\n");
                    for(uint i=0; i < hcp_chain.arg.size/2; i++) {
                        printf("%d ", *(uint16_t *)(hcp_chain.arg.data+i*2));
                    }
                    printf("\n");
                }
                break;
            case 'f': {
                printf("Timeout (ms): ");
                fgets(cmd, sizeof(cmd), stdin);
                uint32_t prev_timeout = hcp_chain.phy_rx_timeout;
                hcp_chain.phy_rx_timeout = atoi(cmd);
                res = bep_capture(&hcp_chain, atoi(cmd));
                hcp_chain.phy_rx_timeout = prev_timeout;
                break;
            }
            case 'g': {
                uint32_t size;
                res = bep_image_get_size(&hcp_chain, &size);
                if (res == FPC_BEP_RESULT_OK) {
                    uint8_t *buf = (uint8_t *)malloc(size);
                    if (buf) {
                      res = bep_image_get(&hcp_chain, buf, size);
                      if (res == FPC_BEP_RESULT_OK) {
                        //   if(size != hcp_chain.arg.size) {
                              printf("Image size: %d. Received %d bytes\n", size, hcp_chain.arg.size);
                        //   }
                        FILE *f = fopen("image.raw", "wb");
                        if (f) {
                            fwrite(buf, 1, size, f);
                            fclose(f);
                            printf("Image saved as image.raw\n");
                        }
                        f = fopen("image.pgm", "wb");
                        if(f) {
                            save_to_pgm(f, buf, 160, 160);
                            fclose(f);
                            printf("Image saved as image.pgm\n");
                        }
                        free(buf);
                      }
                    }
                }
                break;
            }
            case 'T': {
                uint32_t size;
                printf("Read template from file: ");
                fscanf(stdin, "%s", cmd);
                uint8_t *buf = (uint8_t *)malloc(102400);
                FILE *f = fopen(cmd, "rb");
                if (f) {
                    size = fread(buf, 1, 102400, f);
                    fclose(f);
                    if(size > 0) {
                        printf("Pushing template size %d\n", size);
                        res = bep_template_put(&hcp_chain, buf, size);
                        if (res != FPC_BEP_RESULT_OK) {
                            printf("Pushing template error: %d\n", res);
                        }
                    }
                } else {
                    printf("Can't open %s\n", cmd);
                }
                free(buf);

                break;
            }
            case 't': {
                    uint8_t *buf = (uint8_t *)malloc(102400);
                    printf("Save template to file: ");
                    fscanf(stdin, "%s", cmd);
                    if (buf) {
                      res = bep_template_get(&hcp_chain, buf, 102400);
                      if (res == FPC_BEP_RESULT_OK) {
                        //   if(size != hcp_chain.arg.size) {
                              printf("Template size received %d bytes\n", hcp_chain.arg.size);
                        //   }
                          FILE *f = fopen(cmd, "wb");
                          if (f) {
                              fwrite(buf, 1, hcp_chain.arg.size, f);
                              fclose(f);
                              printf("Image saved as %s\n", cmd);
                          }
                        }
                    }
                    free(buf);
                break;
            }
            case 'h': {
                char version[100];

                memset(version, 0, 100);
                res = bep_version(&hcp_chain, version, 99);
                if (res == FPC_BEP_RESULT_OK) {
                  printf("%s\n", version);
                }
                break;
            }
            case 'r':
                bep_sw_reset(&hcp_chain);
                break;
            case 'q':
                return 0;
            default:
                printf("\nUnknown command\n");
        }
        if (hcp_chain.bep_result == FPC_BEP_RESULT_OK) {
            printf("\nCommand succeded\n");
        } else {
            printf("\nCommand failed with error code %d\n", hcp_chain.bep_result);
        }

        if (res == FPC_BEP_RESULT_OK) {
            printf("Transfer succeded\n");
        } else {
            printf("Transfer failed with error code %d\n", res);
        }

        printf("Press any key to continue...");
        fgets(cmd, sizeof(cmd), stdin);
    }

    return 0;
}
