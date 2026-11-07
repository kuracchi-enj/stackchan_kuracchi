#!/bin/bash
set -eu

STACKCHAN_PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [ "$(uname -s)" != Darwin ] || [ "$(uname -m)" != arm64 ]; then
  printf '%s\n' 'このセットアップはApple SiliconのMac用。別の環境ではdocs/toolchain.mdを確認してほしい。' >&2
  exit 1
fi

mkdir -p "$STACKCHAN_PROJECT_ROOT/.tools" "$STACKCHAN_PROJECT_ROOT/private/downloads"
STACKCHAN_CLI_ARCHIVE="$STACKCHAN_PROJECT_ROOT/private/downloads/arduino-cli_1.5.1_macOS_ARM64.tar.gz"
if [ ! -f "$STACKCHAN_CLI_ARCHIVE" ]; then
  curl -fL --proto '=https' --connect-timeout 10 --max-time 180 \
    --output "$STACKCHAN_CLI_ARCHIVE.download" \
    https://downloads.arduino.cc/arduino-cli/arduino-cli_1.5.1_macOS_ARM64.tar.gz
  mv "$STACKCHAN_CLI_ARCHIVE.download" "$STACKCHAN_CLI_ARCHIVE"
fi

python3 - "$STACKCHAN_CLI_ARCHIVE" "$STACKCHAN_PROJECT_ROOT/.tools/arduino-cli" <<'PY'
import hashlib
import sys
import tarfile
from pathlib import Path

archive, target = map(Path, sys.argv[1:])
expected = 'cb952e8c1621c95ef5f1d17831c945e3d0ec5973f89c557a7ec8feb9c4f7d4c9'
if hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
    raise SystemExit('Arduino CLIのチェックサムが一致しない。取得元とファイルを確認してほしい。')
with tarfile.open(archive) as source:
    binaries = [entry for entry in source.getmembers()
                if entry.isfile() and Path(entry.name).name == 'arduino-cli']
    if len(binaries) != 1:
        raise SystemExit('Arduino CLIのアーカイブ構成が想定と違う。')
    target.write_bytes(source.extractfile(binaries[0]).read())
target.chmod(0o755)
PY

"$STACKCHAN_PROJECT_ROOT/scripts/arduino.sh" version
"$STACKCHAN_PROJECT_ROOT/scripts/arduino.sh" core update-index
"$STACKCHAN_PROJECT_ROOT/scripts/arduino.sh" core install m5stack:esp32@3.2.2
"$STACKCHAN_PROJECT_ROOT/scripts/arduino.sh" lib install --no-deps \
  M5GFX@0.2.28 M5Unified@0.2.21 M5StackChan@1.1.1
