## FPC BM-Lite example application

There are 3 main folders in the project:
- [BMLite_examples](BMLite_examples) - HW-independed application examples
- [BMLite_sdk](BMLite_sdk) - FPC BM-Lite SDK
- [HAL_Driver](HAL_Driver) - implementation of HW-dependent functions

------------

## Build system

There are some variables controlling application building:

APP - defines application
PLATFORM - defines HW plaform with HAL implementation
DEBUG - if defined as 'y' will produce debug output and add debug information

Not all applications can be built for a particular platform. To build embedded application for nRF52840 platform with additional debug info use:

`make APP=embedded_app PLATFORM=nRF52840 DEBUG=y`

There are some useful makefile targets:

- Show all available applications:
  `make list_apps`
- Show all available platforms:
  `make list_hals`
- Clean output for particular application and platform:
  `make PLATFORM=nRF52840 APP=embedded_app clean`
- Clean output for all applications and platforms:
  `make clean_all`


### Platform-independent interface functions

Platform-independent interface implemented in [platform.c](BMLite_sdk/src/platform.c)

| Platform function | Description  |
| :-------- | :-------- |
| fpc_bep_result_t **platform_init**(void *params) |  Initilalizes hardware |
| void **platform_bmlite_reset**(void) | Implements BM-Lite HW Reset |
| fpc_bep_result_t **platform_bmlite_spi_send**(uint16_t size, const uint8_t *data, uint32_t timeout) | Send data packet to FPC BM-Lite |
| fpc_bep_result_t **platform_bmlite_spi_receive**(uint16_t size, uint8_t *data, uint32_t timeout) | Receive data packet from FPC BM-Lite. If timeout = **0**, the function will wait for data from BM-Lite indefinitely. The waiting loop will be breaked if **hal_check_button_pressed()** returns non-zero value. It is recommended to do HW or SW reset of BM-Lite if **platform_bmlite_spi_receive()** returns **FPC_BEP_RESULT_TIMEOUT** in order to return is into known state. |

Currently **platform_bmlite_uart_send()** and **platform_bmlite_uart_receive()** are not implemented. For UART interface there is no need to wait for **IRQ** pin ready. However because in UART mode there is no signal from FPC-BM-LIte that it will send data, I would recommend to use UART interrupt or DMA to receive data from UART and store it to a separate buffer and read data in **platform_bmlite_uart_receive()** from that buffer. Activation UART data reading only inside **platform_bmlite_uart_receive()** could lead to loosing some incoming data and causing HCP protocol errors.

------------

### HAL implementation

For porting the project to a new microcontroller, all functions from [bmlite_hal.h](BMLite_sdk/inc/bmlite_hal.h) should be implemented.

#### Functions must be implemented: 

|  HAL Function |  Description |
| :------------ | :------------ |
| fpc_bep_result_t **hal_board_init**(void *params) |  Initialize GPIO, System timer, SPI  |
| void **hal_bmlite_reset**(bool state) |  Activate/Deactivate BM-Lite **RST_N** pin (***Active Low***) |
| fpc_bep_result_t **hal_bmlite_spi_write_read**(uint8_t *write, uint8_t *read, size_t size, bool leave_cs_asserted) |  SPI data exchange |
| bool **hal_bmlite_get_status**(void) | Return status of BM-Lite **IRQ** pin (***Active High***) |
| void **hal_timebase_init**(void) |  Initialize system clock with 1 msec tick |
| uint32_t **hal_timebase_get_tick**(void) | Read currect system clock value |
| void **hal_timebase_busy_wait**(uint32_t ms) | Delay for **ms** msec |

#### Optional functions for  implementing user interface in BM-Lite example application:

|  HAL Function |  Description |
| :------------ | :------------ |
| uint32_t **hal_get_button_press_time**(void) | How long UI button was pressed last time. Also it should reset button pressed time counter |
| uint32_t **hal_check_button_pressed**(void) | Used for breaking waiting in **platform_bmlite_spi_receive()** if returns non-zero |
| void **hal_set_leds**(platform_led_status_t status, uint16_t mode) | Set LED(s) state according to status and mode. |

------------

### Some notes about FPC BM-Lite HW interface

- BM-Lite support both UART and SPI communication interface. BM-Lite automatically detects the specific communication interface in use. However, it is not possible to use both interfaces at the same time! 
- **IRQ** pin is used in SPI mode only as FPC BM-Lite Ready-to-Transmit signal. The module set it to **High** when it has some data to send and set it to **Low** when data packet is sent.

