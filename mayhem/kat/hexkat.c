/* Known-answer tool: decodes an Intel HEX file with upstream's parser and prints each flash write. */
#include <stdio.h>
#include <stdlib.h>
#include "hex_drive.h"

static void sink(void *ctx, uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t i, nff = 0, sum = 0;
    (void)ctx;
    for (i = 0; i < len; i++) {
        sum += buf[i];
        nff += buf[i] != 0xff;
    }
    printf("WRITE addr=0x%08x len=%u nonff=%u sum=%u first=%02x%02x%02x%02x\n", addr, len, nff, sum,
           buf[0], len > 1 ? buf[1] : 0, len > 2 ? buf[2] : 0, len > 3 ? buf[3] : 0);
}

int main(int argc, char **argv)
{
    FILE *f;
    uint8_t *data;
    long n;
    int rc;
    if (argc != 2 || !(f = fopen(argv[1], "rb"))) {
        fprintf(stderr, "usage: hexkat file.hex\n");
        return 2;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)n + 1);
    if (fread(data, 1, (size_t)n, f) != (size_t)n) {
        return 2;
    }
    fclose(f);
    rc = hex_drive(data, (size_t)n, sink, NULL);
    printf("STATUS %d\n", rc);
    free(data);
    return 0;
}
