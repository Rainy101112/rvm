#!/bin/bash
# Quick VM benchmark: runs the given bytecode program N times on a pinned
# CPU and reports the best/median wall-clock time (seconds).
# Usage: bench.sh <bytecode-file> [runs]
set -e

BIN="$1"
RUNS="${2:-7}"

if [ ! -x ./build/rvm ]; then
    echo "build/rvm not found; build it first" >&2
    exit 1
fi

TIMES=()
for _ in $(seq "$RUNS"); do
    start=$EPOCHREALTIME
    RVM_LOGGER_LEVEL=error taskset -c 2 ./build/rvm "$BIN" 0xffff 0 >/dev/null 2>&1
    end=$EPOCHREALTIME
    TIMES+=("$(awk "BEGIN { printf \"%.4f\", $end - $start }")")
done

sorted=$(printf '%s\n' "${TIMES[@]}" | sort -n)
best=$(echo "$sorted" | head -1)
median=$(echo "$sorted" | sed -n "$(( (RUNS + 1) / 2 ))p")

echo "best:   ${best}s"
echo "median: ${median}s"
