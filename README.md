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
PORT - if defined as UART, BM-Lite HAL will use UART interface. Else will use SPI interface

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

<table>
<tr><th> Platform function <th> Description</tr>
<tr><td> fpc_bep_result_t <b>platform_init</b>(void *params) <td>  Initilalizes hardware </tr>
<tr><td> void <b>platform_bmlite_reset</b>(void) <td> Implements BM-Lite HW Reset </tr>
<tr><td> fpc_bep_result_t <b>platform_bmlite_spi_send</b>(uint16_t size, const uint8_t *data, uint32_t timeout) <br> fpc_bep_result_t <b>platform_bmlite_uart_send</b>(uint16_t size, const uint8_t *data, uint32_t timeout) <td> Send data packet to FPC BM-Lite </tr>
<tr><td> fpc_bep_result_t <b>platform_bmlite_spi_receive</b>(uint16_t size, uint8_t *data, uint32_t timeout) <br> fpc_bep_result_t <b>platform_bmlite_uart_receive</b>(uint16_t size, uint8_t *data, uint32_t timeout)<td> Receive data packet from FPC BM-Lite. If timeout = <b>0</b>, the function will wait for data from BM-Lite indefinitely. The waiting loop will be breaked if <b>hal_check_button_pressed()</b> returns non-zero value. It is recommended to do HW or SW reset of BM-Lite if <b>platform_bmlite_spi_receive()</b> returns **FPC_BEP_RESULT_TIMEOUT** in order to return is into known state. </tr>
</table>


For UART interface **IRQ/READY** pin is not used, so in UART mode there is no signal from FPC-BM-Lite indicating that BM-Lite is going to send data. To avoid data loss, all data received from UART must be stored into an incoming circular buffer. I would recommend to use UART interrupt or DMA to receive data from UART and store it to the buffer.
**platform_bmlite_uart_receive()** will read from that buffer when HCP protocol requests to read a data block. Activation UART data reading only inside **platform_bmlite_uart_receive()** will lead to loosing incoming data and causing HCP protocol errors.

------------

### HAL implementation

For porting the project to a new microcontroller, all functions from [bmlite_hal.h](BMLite_sdk/inc/bmlite_hal.h) should be implemented.

#### Functions must be implemented: 

|  HAL Function |  Description |
| :------------ | :------------ |
| fpc_bep_result_t **hal_board_init**(void *params) |  Initialize GPIO, System timer, SPI  |
| void **hal_bmlite_reset**(bool state) |  Activate/Deactivate BM-Lite **RST_N** pin (***Active Low***) |
| fpc_bep_result_t **hal_bmlite_spi_write_read**(uint8_t *write, uint8_t *read, size_t size, bool leave_cs_asserted) |  SPI data exchange |
| size_t **hal_bmlite_uart_write**(const uint8_t *data, size_t size); | Write data to UART interface |
| size_t **hal_bmlite_uart_read**(uint8_t *buff, size_t size); |  Read data from UART interface |
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

