/* Host build of the CMSIS-DAP hardware abstraction (the role of source/hic_hal/<mcu>/DAP_config.h).
 * An SWD+JTAG probe as in the Bambino/lpc11u35 builds (8 JTAG devices), pins routed to dap_model.c. */
#ifndef __DAP_CONFIG_H__
#define __DAP_CONFIG_H__

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_compiler.h"
#include "dap_model.h"

typedef unsigned int BOOL;
#ifndef __TRUE
#define __TRUE 1
#endif
#ifndef __FALSE
#define __FALSE 0
#endif

#define CPU_CLOCK               96000000U
#define IO_PORT_WRITE_CYCLES    2U
#define DAP_SWD                 1
#define DAP_JTAG                1
#define DAP_JTAG_DEV_CNT        8
#define DAP_DEFAULT_PORT        1
#define DAP_DEFAULT_SWJ_CLOCK   5000000
#define DAP_PACKET_SIZE         512
#define DAP_PACKET_COUNT        8
#define SWO_UART                0
#define SWO_UART_DRIVER         1
#define SWO_UART_MAX_BAUDRATE   10000000U
#define SWO_MANCHESTER          0
#define SWO_BUFFER_SIZE         4096U
#define SWO_STREAM              0
#define TIMESTAMP_CLOCK         1000000U
#define DAP_UART                0
#define DAP_UART_DRIVER         1
#define DAP_UART_RX_BUFFER_SIZE 1024U
#define DAP_UART_TX_BUFFER_SIZE 1024U
#define DAP_UART_USB_COM_PORT   1
#define TARGET_FIXED            0

/* DAP.h's PIN_DELAY_SLOW is ARM-only inline asm; clang on x86 takes the portable branch it keeps for armcc.
 * Only the DAP.h include sees the macro: cmsis_compiler.h is already included above. */
#define __CC_ARM 1

__STATIC_INLINE void PORT_JTAG_SETUP(void) {}
__STATIC_INLINE void PORT_SWD_SETUP(void) {}
__STATIC_INLINE void PORT_OFF(void) {}

__STATIC_FORCEINLINE uint32_t PIN_SWCLK_TCK_IN(void) { return 0; }
__STATIC_FORCEINLINE void     PIN_SWCLK_TCK_SET(void) {}
__STATIC_FORCEINLINE void     PIN_SWCLK_TCK_CLR(void) {}
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_TMS_IN(void) { return dap_model_raw_bit(); }
__STATIC_FORCEINLINE void     PIN_SWDIO_TMS_SET(void) {}
__STATIC_FORCEINLINE void     PIN_SWDIO_TMS_CLR(void) {}
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN(void) { return dap_model_swdio_in(); }
__STATIC_FORCEINLINE void     PIN_SWDIO_OUT(uint32_t bit) { (void)bit; }
__STATIC_FORCEINLINE void     PIN_SWDIO_OUT_ENABLE(void) { dap_model_swdio_out_enable(); }
__STATIC_FORCEINLINE void     PIN_SWDIO_OUT_DISABLE(void) { dap_model_swdio_out_disable(); }
__STATIC_FORCEINLINE uint32_t PIN_TDI_IN(void) { return 0; }
__STATIC_FORCEINLINE void     PIN_TDI_OUT(uint32_t bit) { (void)bit; }
__STATIC_FORCEINLINE uint32_t PIN_TDO_IN(void) { return dap_model_raw_bit(); }
__STATIC_FORCEINLINE uint32_t PIN_nTRST_IN(void) { return 0; }
__STATIC_FORCEINLINE void     PIN_nTRST_OUT(uint32_t bit) { (void)bit; }
__STATIC_FORCEINLINE uint32_t PIN_nRESET_IN(void) { return dap_model_raw_bit(); }
__STATIC_FORCEINLINE void     PIN_nRESET_OUT(uint32_t bit) { (void)bit; }

__STATIC_INLINE void LED_CONNECTED_OUT(uint32_t bit) { (void)bit; }
__STATIC_INLINE void LED_RUNNING_OUT(uint32_t bit) { (void)bit; }

__STATIC_INLINE uint32_t TIMESTAMP_GET(void) { return dap_model_timestamp(); }
__STATIC_INLINE void DAP_SETUP(void) {}
__STATIC_INLINE uint32_t RESET_TARGET(void) { return 0; }

#endif
