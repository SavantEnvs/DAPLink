/* Known-answer tool: runs a session of CMSIS-DAP packets (the fuzz input layout) and prints each response. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DAP_config.h"
#include "DAP.h"

int main(int argc, char **argv)
{
    FILE *f;
    uint8_t *data;
    size_t size, pos = 0, scan = 0, i;

    if (argc != 2 || !(f = fopen(argv[1], "rb"))) {
        fprintf(stderr, "usage: dapkat session.bin\n");
        return 2;
    }
    data = malloc(65536);
    size = fread(data, 1, 65536, f);
    fclose(f);

    while (scan < size && data[scan] != 0) {
        scan += 1u + data[scan];
    }
    dap_model_reset(data + (scan < size ? scan + 1 : size), scan < size ? size - scan - 1 : 0);
    memset(&DAP_Data, 0, sizeof(DAP_Data));
    DAP_Setup();

    while (pos < size && data[pos] != 0) {
        uint8_t request[DAP_PACKET_SIZE] = {0};
        uint8_t response[DAP_PACKET_SIZE] = {0};
        size_t n = data[pos++];
        uint32_t r;
        if (n > size - pos) {
            n = size - pos;
        }
        memcpy(request, data + pos, n);
        pos += n;
        r = DAP_ExecuteCommand(request, response);
        printf("RESP");
        for (i = 0; i < (r & 0xFFFFu); i++) {
            printf(" %02x", response[i]);
        }
        printf("\n");
    }
    return 0;
}
