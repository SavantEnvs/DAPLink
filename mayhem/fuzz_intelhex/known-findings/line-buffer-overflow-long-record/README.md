# Intel HEX record longer than 32 bytes overruns the line buffer

`repro.hex` is a data record whose byte count is `0x26` (38). Reproduce with `./fuzz_intelhex-standalone repro.hex`.

- **Where:** `source/daplink/drag-n-drop/intelhex.c`, `parse_hex_blob()`, the low-nibble branch (`line.buf[idx] |= ...`).
- **Cause:** `hex_line_t.buf` is 0x25 bytes, but `byte_count` comes from the file and `idx` runs to `byte_count + 5` (up to 260). The high-nibble branch guards with `idx < sizeof(hex_line_t)`; the low-nibble branch does not, so the write lands past the static `line` buffer. `validate_checksum()` reads `record->buf[i]` up to the same bound.
- **Impact:** a crafted `.hex` file dragged onto the DAPLink MSC drive makes the bootloader/interface write and read up to ~220 bytes past the static buffer (adjacent parser state in RAM).
- **Fix:** reject a record when `line.byte_count > sizeof(line.data)` (HEX_PARSE_LINE_OVERRUN exists for this) and bound both nibble branches by `sizeof(hex_line_t)`.
