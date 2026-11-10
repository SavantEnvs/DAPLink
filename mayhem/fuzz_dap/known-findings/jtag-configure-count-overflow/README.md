# DAP_JTAG_Configure does not bound the device count

`repro-jtag-configure-612.bin` is one `DAP_JTAG_Configure` packet (`0x15`) announcing 9 devices. Reproduce with `./fuzz_dap-standalone repro-jtag-configure-612.bin` (defect 12 in [`../README.md`](../README.md)); it is reported by UBSan (`index 8 out of bounds for type 'uint8_t[8]'`), which halts before the store.

- **Where:** `source/daplink/cmsis-dap/DAP.c`, `DAP_JTAG_Configure()`, `DAP_Data.jtag_dev.ir_length[n]` and the `ir_before`/`ir_after` stores.
- **Cause:** `count` is taken from the request and used as the loop bound, but the arrays hold `DAP_JTAG_DEV_CNT` (8 on the JTAG builds) entries.
- **Impact:** a USB host chooses `count` up to 255 and writes up to ~1 KB past `DAP_Data` (`ir_length` is `uint8_t[8]`, `ir_before`/`ir_after` are `uint16_t[8]`), corrupting adjacent globals. Only builds with `DAP_JTAG=1` are affected.
- **Fix:** answer `DAP_ERROR` when `count > DAP_JTAG_DEV_CNT`.
