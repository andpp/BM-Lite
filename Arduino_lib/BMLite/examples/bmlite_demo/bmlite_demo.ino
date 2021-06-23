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

#include <SPI.h>
#include <BMLite.h>

#define BMLITE_PIN_RESET    2
#define BMLITE_PIN_STATUS   4

#define LED1 14
#define LED2 15
#define LED3 16
#define LED4 17

#define BMLITE_BUTTON 3

#define BMLITE_CS_PIN   8
//#define BMLITE_MISO_PIN ARDUINO_12_PIN
//#define BMLITE_MOSI_PIN ARDUINO_11_PIN
//#define BMLITE_CLK_PIN  ARDUINO_13_PIN


int leds[4] = {LED1, LED2, LED3, LED4};


#define DATA_BUFFER_SIZE (512)
static uint8_t hcp_txrx_buffer[MTU];
static uint8_t hcp_data_buffer[DATA_BUFFER_SIZE];

static HCP_comm_t hcp_chain;

void setup() {

  char *version;

  hcp_chain.write = platform_bmlite_spi_send;
  hcp_chain.read = platform_bmlite_spi_receive;
  hcp_chain.phy_rx_timeout = 2000;
  hcp_chain.pkt_buffer = hcp_data_buffer;
  hcp_chain.pkt_size_max = sizeof(hcp_data_buffer);
  hcp_chain.pkt_size = 0;
  hcp_chain.txrx_buffer = hcp_txrx_buffer;
  

  // put your setup code here, to run once:
  pinMode(BMLITE_PIN_RESET,  OUTPUT);
  pinMode(BMLITE_PIN_STATUS, INPUT);

  for(int i=0; i<4; i++) {
    pinMode(leds[i],  OUTPUT);
  }
  set_leds(0);

  pinMode(BMLITE_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BMLITE_BUTTON), check_buttons, CHANGE);

  SPI.begin();
  pinMode(BMLITE_CS_PIN, OUTPUT);
  
  platform_init(NULL);

  // These lines for debug purpose only 
  {
    version = (char *)malloc(50);
    memset(version, 0, 50);
    fpc_bep_result_t res = bep_version(&hcp_chain, version, 99);
    free(version);
  }

}

uint16_t template_id;
uint32_t current_id = 0;
bool match;


void loop() {
  // put your main code here, to run repeatedly:
    int res;

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
        return;
    }
    hal_set_leds(BMLITE_LED_STATUS_MATCH, match);
    res = sensor_wait_finger_not_present(&hcp_chain, 0);
}

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


/* 
 * Arduino HAL Implementation
 */

fpc_bep_result_t hal_board_init(void *params)
{
    (void)params;
    
    return FPC_BEP_RESULT_OK;
}

void hal_bmlite_reset(bool state)
{
    if(!state) {
      digitalWrite(BMLITE_PIN_RESET, HIGH);
    } else {
      digitalWrite(BMLITE_PIN_RESET, LOW);
    }
}

bool hal_bmlite_get_status(void)
{
    return digitalRead(BMLITE_PIN_STATUS);
}


/** LED ON time in ms */
#define LED_SOLID_ON_TIME_MS 700

/** LED blink time in ms */
#define LED_BLINK_TIME_MS    200

static void set_leds(uint8_t color)
{
  uint32_t i;

  for(i=0; i<4; i++) {
    digitalWrite(leds[i], color & 1);
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


fpc_bep_result_t hal_bmlite_spi_write_read(uint8_t *buff, size_t size)
{
  
  digitalWrite(BMLITE_CS_PIN, LOW);
  SPI.transfer(buff,size);
  digitalWrite(BMLITE_CS_PIN, HIGH);
  
  return FPC_BEP_RESULT_OK;
}


volatile uint32_t button_pressed_time = 0;

static uint32_t btn_press_start;
static uint32_t btn_pressed = 0;

void hal_timebase_init(void)
{
}

void hal_timebase_busy_wait(uint32_t delay)
{
    uint32_t start;
    uint32_t delay_internal = 0;

    /* Ensure minimum delay or skip if delay is zero*/
    if (delay) {
        delay_internal = delay + 1;
        start = millis();
        while ((millis() - start) < delay_internal) {
        }
    }
}

hal_tick_t hal_timebase_get_tick(void)
{
    return millis();
}

static void check_buttons()
{
    if (digitalRead(BMLITE_BUTTON)) {
        if (btn_pressed == 0) {
            btn_press_start = micros();
            btn_pressed = 1;
        }
    } else { // Btn released
        if (btn_pressed) {
            if (micros() > btn_press_start) {
                button_pressed_time = micros() - btn_press_start;
            } else {
                button_pressed_time = micros() + ~btn_press_start + 1;
            }
            btn_pressed = 0;
        }
    }
}

uint32_t hal_get_button_press_time()
{
    uint32_t time = button_pressed_time;
    button_pressed_time = 0;
    return time;
}

uint32_t hal_check_button_pressed()
{
    uint32_t time = button_pressed_time;
    return time;
}
