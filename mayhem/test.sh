#!/usr/bin/env bash
#
# mayhem/test.sh — run the pre-built known-answer tools (mayhem/build.sh step 3) and assert their exact output.
# hexkat: upstream intelhex.c decoding Intel HEX files (addresses, lengths, byte sums, parser status).
# dapkat: upstream CMSIS-DAP command processor (DAP.c + SW_DP.c) answering a scripted host session.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

KAT=mayhem/build/kat
for t in hexkat dapkat; do
  [ -x "$KAT/$t" ] || { echo "missing $KAT/$t: mayhem/build.sh did not build it" >&2; emit_ctrf "daplink-kat" 0 1; exit 1; }
done

passed=0 failed=0
check() {
  local name="$1" expected="$2"; shift 2
  local actual rc
  actual="$("$@" 2>&1)"; rc=$?
  if [ "$rc" -eq 0 ] && [ "$actual" == "$expected" ]; then
    passed=$((passed + 1)); echo "PASS $name"
  else
    failed=$((failed + 1)); echo "FAIL $name (rc=$rc)"; echo "--- expected"; echo "$expected"; echo "--- actual"; echo "$actual"
  fi
}

check intelhex-contiguous "WRITE addr=0x08000000 len=32 nonff=32 sum=496 first=00010203
STATUS 1" "$KAT/hexkat" mayhem/kat/contig.hex

check intelhex-gap-splits-writes "WRITE addr=0x00000100 len=8 nonff=8 sum=1320 first=a5a5a5a5
WRITE addr=0x00000200 len=4 nonff=4 sum=360 first=5a5a5a5a
STATUS 1" "$KAT/hexkat" mayhem/kat/gap.hex

check intelhex-bad-checksum-rejected "STATUS 4" "$KAT/hexkat" mayhem/kat/badsum.hex

check intelhex-extended-segment-address "WRITE addr=0x00010000 len=4 nonff=4 sum=824 first=deadbeef
STATUS 1" "$KAT/hexkat" mayhem/kat/extseg.hex

check cmsis-dap-session "RESP 00 12 44 41 50 4c 69 6e 6b 20 43 4d 53 49 53 2d 44 41 50 00
RESP 00 06 32 2e 31 2e 30 00
RESP 00 01 33
RESP 00 04 40 42 0f 00
RESP 00 02 00 02
RESP 00 01 08
RESP 02 01
RESP 05 01 01 ef be ad de
RESP 05 01 01
RESP 00 05 30 32 35 34 00" "$KAT/dapkat" mayhem/kat/session1.bin

emit_ctrf "daplink-kat" "$passed" "$failed"
