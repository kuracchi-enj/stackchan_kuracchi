#!/bin/bash
set -eu

STACKCHAN_PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$STACKCHAN_PROJECT_ROOT/scripts/arduino.sh" compile \
  --fqbn m5stack:esp32:m5stack_cores3 \
  --warnings all \
  --build-path "$STACKCHAN_PROJECT_ROOT/build/minimal_offline" \
  "$STACKCHAN_PROJECT_ROOT/firmware/arduino/minimal_offline"
