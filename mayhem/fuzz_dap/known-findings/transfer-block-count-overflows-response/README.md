# DAP_TransferBlock writes `count` words into a packet-sized response

`repro-transfer-block-1358.bin` is `DAP_Connect(SWD)` followed by a `DAP_TransferBlock` (`0x06`) read of 256 words. Reproduce with `./fuzz_dap-standalone repro-transfer-block-1358.bin` (defect 10 in [`../README.md`](../README.md)). The same unbounded response growth is the response half of [`../execute-commands-overruns-packet/`](../execute-commands-overruns-packet/).

- **Where:** `source/daplink/cmsis-dap/DAP.c`, `DAP_SWD_TransferBlock()` (`*response++ = (uint8_t) data;`), and `DAP_JTAG_TransferBlock()`.
- **Cause:** the 16-bit block count is host-controlled and each read appends 4 response bytes, with no check against `DAP_PACKET_SIZE` (512 in this build).
- **Impact:** up to 65535 words (256 KB) are written past the response buffer. `DAP_queue_execute_buf()` uses a slot of `USB_Request` as the response, so this overwrites the following slots and whatever lies after the queue.
- **Fix:** clamp the number of words returned to what fits in `DAP_PACKET_SIZE - 4` (the CMSIS-DAP spec says the host must not request more, so a conforming probe should report `DAP_ERROR`).
