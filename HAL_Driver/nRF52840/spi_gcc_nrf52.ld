/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x80000
  FLASH_STORAGE (rx) : ORIGIN = 0x80000, LENGTH = 0x80000
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x40000
}

GROUP (
    libc_nano.a
    libm.a
    libgcc.a
)

/* The stack size used by the application. NOTE: you need to adjust according to your application. */
__stack_size__ = DEFINED(__stack_size__) ? __stack_size__ : 0x3000;
_eheap = ORIGIN(RAM) + LENGTH(RAM) - 4;

_fs_storage_start = ORIGIN(FLASH_STORAGE);
_fs_storage_size  = LENGTH(FLASH_STORAGE);


SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

SECTIONS
{
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > FLASH
  
   .rodata.host_cmd_handlers : ALIGN(4)
   {
    __host_cmd_handlers_start = .;
    KEEP(*(.host_cmd_handlers))   /* Host command handlers section */
    __host_cmd_handlers_end = .;
   } >FLASH
  
} INSERT AFTER .text

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH
    .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH

} INSERT AFTER .text

INCLUDE "nrf52_common.ld"
