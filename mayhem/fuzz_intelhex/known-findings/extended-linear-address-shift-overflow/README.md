# Extended linear address record shifts a promoted `int` into the sign bit

`repro.hex` is an extended linear address record (type `04`) with the upper address `0x8000`, then the end-of-file record. It is the second `fuzz_intelhex` defect of the first Mayhem run (3591186, "Incorrect Calculation").

- **Where:** `source/daplink/drag-n-drop/intelhex.c:239`, `parse_hex_blob()`, `(line.data[0] << 24) | (line.data[1] << 16)`.
- **Cause:** `line.data[0]` is a `uint8_t`, promoted to `int`, so a value of `0x80` or more (a flash address of `0x80000000` and above) shifts into the sign bit. That is undefined behaviour in C (`shift-base` in UBSan). Compilers for the probe's Cortex-M cores produce the intended wrapped value, so it is benign in practice.
- **Impact:** none observed on the probe; it only matters to a compiler that exploits the UB. With halting UBSan it aborts every HEX input that has such a record, before the parser's later logic runs.
- **Handling:** `mayhem/build.sh` compiles `intelhex.c` with `-fno-sanitize=shift-base`, as it already does for `DAP.c`, `SW_DP.c` and `JTAG_DP.c`. The fuzz target no longer aborts on this input, and every other ASan and UBSan check on `intelhex.c` stays on. Rebuilding `intelhex.c` without that flag brings the abort back on `repro.hex`.
- **Fix:** `((uint32_t)line.data[0] << 24) | ((uint32_t)line.data[1] << 16)`.
