#include "dap_model.h"

/* The stream is the target's behaviour. For every SWD read phase (entered by PIN_SWDIO_OUT_DISABLE) one
 * byte picks the ACK, four bytes are the data word and one byte decides whether the parity bit is right.
 * Bits read outside a read phase (sequences, JTAG TDO, nRESET) are raw stream bits. When the stream runs
 * out the target answers ACK OK with zero data. */
static struct {
    const uint8_t *stream;
    size_t len, pos;
    uint32_t raw_byte, raw_left;
    int reading;
    uint32_t rd_pos, ack, data, parity;
    uint32_t ts;
} m;

static int more(void)
{
    return m.pos < m.len;
}

static uint8_t next_byte(void)
{
    return m.stream[m.pos++];
}

void dap_model_reset(const uint8_t *stream, size_t len)
{
    m.stream = stream;
    m.len = len;
    m.pos = 0;
    m.raw_left = 0;
    m.reading = 0;
    m.rd_pos = 0;
    m.ts = 0;
}

uint32_t dap_model_raw_bit(void)
{
    uint32_t bit;
    if (m.raw_left == 0) {
        m.raw_byte = more() ? next_byte() : 0;
        m.raw_left = 8;
    }
    bit = m.raw_byte & 1u;
    m.raw_byte >>= 1;
    m.raw_left--;
    return bit;
}

void dap_model_swdio_out_disable(void)
{
    m.reading = 1;
    m.rd_pos = 0;
}

void dap_model_swdio_out_enable(void)
{
    m.reading = 0;
}

static void start_read_phase(void)
{
    uint8_t b;
    int i;
    m.ack = 1;
    m.data = 0;
    m.parity = 0;
    if (!more()) {
        return;
    }
    b = next_byte();
    switch (b & 7u) {
        case 5: m.ack = 2; break;
        case 6: m.ack = 4; break;
        case 7: m.ack = more() ? (next_byte() & 7u) : 1; break;
        default: m.ack = 1; break;
    }
    for (i = 0; i < 4; i++) {
        m.data |= (uint32_t)(more() ? next_byte() : 0) << (8 * i);
    }
    b = more() ? next_byte() : 1;
    m.parity = (uint32_t)__builtin_parity(m.data) ^ ((b & 7u) == 0);
}

uint32_t dap_model_swdio_in(void)
{
    uint32_t p;
    if (!m.reading) {
        return dap_model_raw_bit();
    }
    p = m.rd_pos++;
    if (p == 0) {
        start_read_phase();
    }
    if (p < 3) {
        return (m.ack >> p) & 1u;
    }
    if (p < 35) {
        return (m.data >> (p - 3)) & 1u;
    }
    if (p == 35) {
        return m.parity;
    }
    return 0;
}

uint32_t dap_model_timestamp(void)
{
    return m.ts += 7;
}
