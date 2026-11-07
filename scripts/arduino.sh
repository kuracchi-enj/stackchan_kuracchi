#!/bin/bash
set -eu

STACKCHAN_PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
STACKCHAN_ARDUINO_CLI="$STACKCHAN_PROJECT_ROOT/.tools/arduino-cli"

if [ ! -x "$STACKCHAN_ARDUINO_CLI" ]; then
  printf '%s\n' 'Arduino CLIがまだない。docs/toolchain.mdの準備手順を確認してほしい。' >&2
  exit 1
fi

export ARDUINO_DIRECTORIES_DATA="$STACKCHAN_PROJECT_ROOT/private/arduino/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="$STACKCHAN_PROJECT_ROOT/private/arduino/downloads"
export ARDUINO_DIRECTORIES_USER="$STACKCHAN_PROJECT_ROOT/private/arduino/user"
export ARDUINO_BUILD_CACHE_PATH="$STACKCHAN_PROJECT_ROOT/private/arduino/cache"
export ARDUINO_UPDATER_ENABLE_NOTIFICATION=false

exec "$STACKCHAN_ARDUINO_CLI" --config-file "$STACKCHAN_PROJECT_ROOT/config/arduino-cli.yaml" "$@"
