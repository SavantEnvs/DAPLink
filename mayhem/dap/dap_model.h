/* Host-side model of the debug probe's pins and of a (fuzzer-driven) SWD/JTAG target. */
#ifndef DAP_MODEL_H
#define DAP_MODEL_H

#include <stddef.h>
#include <stdint.h>

void     dap_model_reset(const uint8_t *stream, size_t len);
uint32_t dap_model_swdio_in(void);
void     dap_model_swdio_out_enable(void);
void     dap_model_swdio_out_disable(void);
uint32_t dap_model_raw_bit(void);
uint32_t dap_model_timestamp(void);

#endif
