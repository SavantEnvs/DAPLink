# DAP_Transfer / DAP_TransferBlock trust the host's count and read past the request packet

Six defects (rows 1-6 of [`../README.md`](../README.md)), one root cause. Reproduce each with `./fuzz_dap-standalone <repro>`.

- **Cause:** the handlers loop `request_count` times (an 8-bit count for `DAP_Transfer`, a 16-bit count for `DAP_TransferBlock`) and advance `request` by 1 to 5 bytes per entry. Nothing compares the running total with `DAP_PACKET_SIZE`, and nothing returns an error when the packet is too short for its own count.
- **Impact:** a host-chosen count reads up to ~1.3 KB (`255 * 5`) for `DAP_Transfer`, and up to 256 KB for `DAP_TransferBlock`, beyond the packet. On the probe that is the neighbouring USB buffer or queue slot, so the command acts on stale bytes. A write transfer would send them to the target.
- **Fix:** carry the remaining request length through the handlers and answer `DAP_ERROR` or an invalid response once `count` entries would not fit in `DAP_PACKET_SIZE`.

| Reproducer | Line | What it reaches |
|---|---|---|
| `repro-swd-transfer-731.bin` | `DAP_SWD_Transfer` 731 | per-entry header byte `*request++` (count 137 of 5-byte write entries) |
| `repro-swd-transfer-876.bin` | `DAP_SWD_Transfer` 876 | third byte of a write's data word, `*(request+2)` |
| `repro-swd-transfer-877.bin` | `DAP_SWD_Transfer` 877 | fourth byte of a write's data word, `*(request+3)` |
| `repro-swd-transfer-913.bin` | `DAP_SWD_Transfer` 913 | the "process cancelled requests" loop: after a transfer fails, the rest are still parsed to compute the request length |
| `repro-dummy-transfer-1253.bin` | `DAP_Dummy_Transfer` 1253 | the same skip loop when the port is neither SWD nor JTAG (count 255, 3-byte body) |
| `repro-transfer-block-1371.bin` | `DAP_SWD_TransferBlock` 1371 | write-block data words, 4 bytes per entry |

Lines 731/876/877 are the same loop and differ only in the byte at which the request runs out of packet (they are separate Mayhem defects because the faulting line differs). `DAP_JTAG_Transfer` and `DAP_JTAG_TransferBlock` have the same pattern.
