# DAP_SWD_Sequence / DAP_JTAG_Sequence trust the sequence count and read past the request packet

Two defects (rows 7-8 of [`../README.md`](../README.md)). Reproduce with `./fuzz_dap-standalone <repro>`.

- **Where:** `source/daplink/cmsis-dap/DAP.c`, `DAP_SWD_Sequence()` (`sequence_info = *request++`, line 516) and `DAP_JTAG_Sequence()` (line 571).
- **Cause:** `sequence_count` is the second request byte (up to 255). Each sequence reads an info byte, and an output sequence also consumes up to 8 data bytes (`(count + 7) / 8`, with a count of 0 meaning 64 bits). The total is never compared with `DAP_PACKET_SIZE`, so `255 * 9` bytes (~2.3 KB) can be consumed from a 512-byte packet. The response side has the same problem for input sequences: each one appends up to 8 bytes to the response.
- **Impact:** the sequences clock out whatever follows the packet in memory (stale USB data or the next buffer) on SWDIO/TMS/TDI, and the response can grow past its buffer. `DAP_JTAG_Sequence` is only built where `DAP_JTAG=1`.
- **Fix:** track the remaining request and response bytes and answer `DAP_ERROR` when a sequence would not fit.

| Reproducer | Line |
|---|---|
| `repro-swd-sequence-516.bin` | `DAP_SWD_Sequence` 516 |
| `repro-jtag-sequence-571.bin` | `DAP_JTAG_Sequence` 571 |
