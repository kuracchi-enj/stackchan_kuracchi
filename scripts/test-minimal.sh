#!/bin/bash
set -eu

STACKCHAN_PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$STACKCHAN_PROJECT_ROOT/build/host-tests"

c++ -std=c++17 -Wall -Wextra -Werror \
  -I "$STACKCHAN_PROJECT_ROOT/tests/minimal_offline/fakes" \
  "$STACKCHAN_PROJECT_ROOT/tests/minimal_offline/initialization_test.cpp" \
  -o "$STACKCHAN_PROJECT_ROOT/build/host-tests/initialization_test"

"$STACKCHAN_PROJECT_ROOT/build/host-tests/initialization_test"
