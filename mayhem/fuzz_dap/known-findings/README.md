# fuzz_dap: known findings

The first 300 s Mayhem run of `fuzz_dap` reported 12 defects. All 12 have their first application frame in `source/daplink/cmsis-dap/DAP.c` and none is in the harness. They come from four root causes, each with its own directory. Every `repro-*.bin` replays one defect: `./fuzz_dap-standalone <file>` aborts at the line shown.

| # | Defect (Mayhem) | Site in `DAP.c` | Kind | Root cause | Reproducer |
|---|---|---|---|---|---|
| 1 | 3591226 | `DAP_SWD_Transfer` 731 | read past request | [request count](transfer-count-overreads-request/) | `transfer-count-overreads-request/repro-swd-transfer-731.bin` |
| 2 | 3591172 | `DAP_SWD_Transfer` 876 | read past request | [request count](transfer-count-overreads-request/) | `transfer-count-overreads-request/repro-swd-transfer-876.bin` |
| 3 | 3591153 | `DAP_SWD_Transfer` 877 | read past request | [request count](transfer-count-overreads-request/) | `transfer-count-overreads-request/repro-swd-transfer-877.bin` |
| 4 | 3591167 | `DAP_SWD_Transfer` 913 | read past request | [request count](transfer-count-overreads-request/) | `transfer-count-overreads-request/repro-swd-transfer-913.bin` |
| 5 | 3591143 | `DAP_Dummy_Transfer` 1253 | read past request | [request count](transfer-count-overreads-request/) | `transfer-count-overreads-request/repro-dummy-transfer-1253.bin` |
| 6 | 3591147 | `DAP_SWD_TransferBlock` 1371 | read past request | [request count](transfer-count-overreads-request/) | `transfer-count-overreads-request/repro-transfer-block-1371.bin` |
| 7 | 3591168 | `DAP_SWD_Sequence` 516 | read past request | [request count](sequence-count-overreads-request/) | `sequence-count-overreads-request/repro-swd-sequence-516.bin` |
| 8 | 3591152 | `DAP_JTAG_Sequence` 571 | read past request | [request count](sequence-count-overreads-request/) | `sequence-count-overreads-request/repro-jtag-sequence-571.bin` |
| 9 | 3591223 | `DAP_ProcessCommand` 1667 | read past request | [`ExecuteCommands` queue](execute-commands-overruns-packet/) | `execute-commands-overruns-packet/repro-request-1667.bin` |
| 10 | 3591149 | `DAP_SWD_TransferBlock` 1358 | write past response | [response size](transfer-block-count-overflows-response/) | `transfer-block-count-overflows-response/repro-transfer-block-1358.bin` |
| 11 | 3591222 | `DAP_ProcessCommand` 1675 | write past response | [`ExecuteCommands` queue](execute-commands-overruns-packet/) | `execute-commands-overruns-packet/repro-response-1675.bin` |
| 12 | 3591156 | `DAP_JTAG_Configure` 612 | index past array | [device count](jtag-configure-count-overflow/) | `jtag-configure-count-overflow/repro-jtag-configure-612.bin` |

By root cause: 9 request over-reads (rows 1-9, three directories), 2 response overflows (10, 11) and 1 unbounded device count (12).

## Are any of these artefacts of the harness?

None is a harness bug: every site is an upstream loop or store whose bound comes from the host packet and is never compared with `DAP_PACKET_SIZE` (or `DAP_JTAG_DEV_CNT`). Two properties of the harness decide how they show up:

- **Buffer size and placement.** `fuzz_dap.c` gives each command a request and a response buffer of exactly `DAP_PACKET_SIZE` (512 here, the size of the HS `hic_hal` configs; the FS ones use 64 and trip the same bugs at smaller counts). On the probe, the response is a slot of `DAP_queue.USB_Request` and the request is the USB endpoint buffer (`DAP_queue_execute_buf`), so an overrun reads or clobbers the neighbouring slot or buffer and does not fault. ASan turns that silent corruption into an abort. The read overruns also return stale bytes on the probe where the harness returns the zero padding; that changes which bytes are used, not whether the read is out of bounds.
- **JTAG configuration.** `mayhem/dap/DAP_config.h` builds with `DAP_JTAG=1` and `DAP_JTAG_DEV_CNT=8`, to reach `JTAG_DP.c`. Only some boards do (`lpc11u35` with 8 devices, `lpc55xx` with 4, where a count of 5 already overflows), so rows 8 and 12 affect those builds only. The SWD rows affect every build.
