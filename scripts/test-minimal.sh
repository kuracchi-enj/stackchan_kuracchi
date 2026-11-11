#!/bin/bash
set -eu

STACKCHAN_PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$STACKCHAN_PROJECT_ROOT/build/host-tests"

for test_name in initialization_test touch_led_test; do
  c++ -std=c++17 -Wall -Wextra -Werror \
    -I "$STACKCHAN_PROJECT_ROOT/tests/minimal_offline/fakes" \
    "$STACKCHAN_PROJECT_ROOT/tests/minimal_offline/$test_name.cpp" \
    -o "$STACKCHAN_PROJECT_ROOT/build/host-tests/$test_name"
  "$STACKCHAN_PROJECT_ROOT/build/host-tests/$test_name"
done
