/* Drives upstream's Intel HEX parser the way source/daplink/drag-n-drop/file_stream.c write_hex() does:
 * 512-byte MSC sectors in, 256-byte bin_buffer out, flash_decoder_write() replaced by a callback. */
#ifndef HEX_DRIVE_H
#define HEX_DRIVE_H

#include <stdint.h>
#include <stddef.h>
#include "intelhex.h"

#define HEX_SECTOR 512
#define HEX_BIN_BUF 256

typedef void (*hex_sink_t)(void *ctx, uint32_t addr, const uint8_t *buf, uint32_t len);

/* Returns the final parse status of the file, or -1 when the first sector is not a hex file. */
static int hex_drive(const uint8_t *data, size_t size, hex_sink_t sink, void *ctx)
{
    uint8_t first[HEX_SECTOR] = {0};
    uint8_t bin_buffer[HEX_BIN_BUF];
    size_t off, n;
    int result = HEX_PARSE_OK;

    if (size == 0) {
        return -1;
    }
    n = size < HEX_SECTOR ? size : HEX_SECTOR;
    for (off = 0; off < n; off++) {
        first[off] = data[off];
    }
    /* validation.c validate_hexfile() default branch */
    if (!((first[0] == ':') && (first[8] == '0' || first[8] == '2' || first[8] == '3' ||
                                first[8] == '4' || first[8] == '5'))) {
        return -1;
    }

    reset_hex_parser();
    for (off = 0; off < size; off += HEX_SECTOR) {
        const uint8_t *blk = data + off;
        uint32_t blk_size = (uint32_t)((size - off) < HEX_SECTOR ? (size - off) : HEX_SECTOR);
        for (;;) {
            uint32_t start = 0, written = 0, parsed = 0;
            hexfile_parse_status_t st = parse_hex_blob(blk, blk_size, &parsed, bin_buffer, sizeof(bin_buffer), &start, &written);
            result = (int)st;
            if (st == HEX_PARSE_OK) {
                if (written > 0) {
                    sink(ctx, start, bin_buffer, written);
                }
                break;
            } else if (st == HEX_PARSE_UNALIGNED) {
                if (written > 0) {
                    sink(ctx, start, bin_buffer, written);
                }
                blk += parsed;
                blk_size -= parsed;
                if (blk_size == 0) {
                    break;
                }
            } else if (st == HEX_PARSE_EOF) {
                if (written > 0) {
                    sink(ctx, start, bin_buffer, written);
                }
                return result;
            } else {
                return result;
            }
        }
    }
    return result;
}

#endif
