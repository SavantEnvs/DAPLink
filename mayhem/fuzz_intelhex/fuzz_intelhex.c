#include <stdint.h>
#include <stddef.h>
#include "hex_drive.h"

static volatile uint32_t g_sink;

static void sink(void *ctx, uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t i, acc = addr;
    (void)ctx;
    for (i = 0; i < len; i++) {
        acc = acc * 31u + buf[i];
    }
    g_sink = acc;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return hex_drive(data, size, sink, NULL) < 0 ? -1 : 0;
}
