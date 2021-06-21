/* Define size of stack. Size must be multiple of 4. */
#define __STARTUP_CONFIG_STACK_SIZE   0x2000

/* Define alignement of stack. Alignment will be 2 to the power of __STARTUP_CONFIG_STACK_ALIGNEMENT. Since calling convention requires that the stack is aligned to 8-bytes when a function is called, the minimum __STARTUP_CONFIG_STACK_ALIGNEMENT is therefore 3. */
#define __STARTUP_CONFIG_STACK_ALIGNEMENT 3

/* Define size of heap. Size must be multiple of 4. */
#define __STARTUP_CONFIG_HEAP_SIZE   0x12000

/* Define entry point */
#define __START main

/* Clear BSS at startup */
#define __STARTUP_CLEAR_BSS