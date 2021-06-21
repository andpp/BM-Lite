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
 */


/**
 * @file   board_leds.c
 * @brief  Leds control functions.
 */

#include "boards.h"
#include "bmlite_hal.h"

/** LED ON time in ms */
#define LED_SOLID_ON_TIME_MS 700

/** LED blink time in ms */
#define LED_BLINK_TIME_MS    200

static void set_leds(uint8_t color)
{
	uint32_t i;

	for(i=0; i<4; i++) {
		if (color & 1) {
             bsp_board_led_on(i);
        } else {
             bsp_board_led_off(i);
        }
        color = color >> 1;
	}
}

void hal_set_leds(platform_led_status_t status, uint16_t mode)
{
     switch(status) {
          case BMLITE_LED_STATUS_READY:
               set_leds(0);
               break;
          case BMLITE_LED_STATUS_MATCH:
               if (mode) {
                    set_leds(1);
               } else {
                    set_leds(2);
               }
               hal_timebase_busy_wait(500);
               break;
          case BMLITE_LED_STATUS_WAITTOUCH:
               if (mode) {
                    set_leds(3);
               }
               break;
          case BMLITE_LED_STATUS_ENROLL:
               if (mode) { 
                    // Start enroll
                    set_leds(1);
                    hal_timebase_busy_wait(500);
                    set_leds(2);
                    hal_timebase_busy_wait(500);
               } else {
                    // Finish enroll
                    set_leds(1);
                    hal_timebase_busy_wait(100);
                    set_leds(0);
                    hal_timebase_busy_wait(100);
                    set_leds(2);
                    hal_timebase_busy_wait(100);
               }
               break;
          case BMLITE_LED_STATUS_DELETE_TEMPLATES:
                    set_leds(4);
                    hal_timebase_busy_wait(100);
                    set_leds(0);
                    hal_timebase_busy_wait(100);
                    set_leds(4);
                    hal_timebase_busy_wait(100);
               break;
          case BMLITE_LED_STATUS_ERROR:
               if (mode) {
                    set_leds(3);
                    hal_timebase_busy_wait(70);
               } else {
                    set_leds(3);
                    hal_timebase_busy_wait(500);
                    set_leds(0);
                    hal_timebase_busy_wait(500);
                    set_leds(3);
                    hal_timebase_busy_wait(500);
               }
               break;
    }
}
