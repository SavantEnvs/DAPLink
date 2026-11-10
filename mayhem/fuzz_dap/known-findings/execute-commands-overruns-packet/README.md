# DAP_ExecuteCommands walks past the request packet and the response buffer

Two defects (rows 9 and 11 of [`../README.md`](../README.md)), both in `DAP_ProcessCommand` as called from `DAP_ExecuteCommand`. Reproduce with `./fuzz_dap-standalone <repro>`.

- **Where:** `source/daplink/cmsis-dap/DAP.c`, `DAP_ExecuteCommand()` (the `ID_DAP_ExecuteCommands` branch, lines 1802-1812). It reads `cnt` (up to 255) from the packet and calls `DAP_ProcessCommand(request, response)` that many times, advancing `request` by the upper 16 bits and `response` by the lower 16 bits of each return value.
- **Cause:** neither pointer is checked against `DAP_PACKET_SIZE`. A packet of many one-byte commands, or of commands that report a large consumed length (such as the handlers in [`../transfer-count-overreads-request/`](../transfer-count-overreads-request/)), moves `request` beyond the packet, and the next sub-command then reads its ID (`*request`, line 1667) from outside it. Each sub-command advances `response` by at least its ID byte (usually 2 or more, and a transfer or info result many more), so 255 of them leave the buffer, and the next sub-command's `*response++ = *request` (line 1675) writes outside the buffer.
- **Impact:** the sub-command ID is read from, and the response is written into, memory beyond the packet and response slot: the next queue slot, or the memory after `DAP_queue` for the last one. The sub-commands also keep running with whatever was read.
- **Fix:** pass the remaining request and response lengths to `DAP_ProcessCommand` and stop with `DAP_ERROR` (as the spec does for an invalid command) when either would be exceeded.

| Reproducer | Line | |
|---|---|---|
| `repro-request-1667.bin` | `DAP_ProcessCommand` 1667 | read of the sub-command ID past the request (`cnt` = 255) |
| `repro-response-1675.bin` | `DAP_ProcessCommand` 1675 | write of the response ID past the response buffer |
