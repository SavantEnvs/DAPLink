/* CMSIS-DAP command processor, as driven by the USB host. Input layout:
 *   { u8 n; n bytes of DAP request } ...   a session of command packets (n == 0 ends the list)
 *   the rest                                what the SWD/JTAG target answers on the wire (dap_model.c) */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "DAP_config.h"
#include "DAP.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    size_t pos = 0;
    int packets = 0;

    if (size < 2) {
        return -1;
    }

    {
        size_t scan = 0;
        while (scan < size && data[scan] != 0) {
            scan += 1u + data[scan];
        }
        dap_model_reset(data + (scan < size ? scan + 1 : size), scan < size ? size - scan - 1 : 0);
    }

    memset(&DAP_Data, 0, sizeof(DAP_Data));
    DAP_TransferAbort = 0;
    DAP_Setup();

    while (pos < size && data[pos] != 0) {
        uint8_t request[DAP_PACKET_SIZE];
        uint8_t response[DAP_PACKET_SIZE];
        size_t n = data[pos++];
        if (n > size - pos) {
            n = size - pos;
        }
        memset(request, 0, sizeof(request));
        memcpy(request, data + pos, n);
        pos += n;
        DAP_ExecuteCommand(request, response);
        packets++;
    }
    return packets ? 0 : -1;
}
