#!/usr/bin/env bash
# Build, then install next to the ASI loader. The old ASI is backed up and the new one
# swapped in with a rename, so a running game's mapped image stays intact; the game
# picks up the new build on its next launch.
set -euo pipefail

STEAMAPPS="${STEAMAPPS:-$HOME/.local/share/Steam/steamapps/common}"
# MGS2/MGS3: <game> | MGS4: "METAL GEAR SOLID 4/MGS4" | PW: MGS_PW/mgspw
DEST="${DEST:-$STEAMAPPS/METAL GEAR SOLID 4/MGS4}"
NAME=hudname

cd "$(dirname "$0")"
nix build -L

if [[ ! -d "$DEST" ]]; then
  echo "$DEST not found (override with DEST=... or STEAMAPPS=...)" >&2
  exit 1
fi

[[ -f "$DEST/$NAME.asi" ]] && cp -p "$DEST/$NAME.asi" "$DEST/$NAME.asi.bak"
install -m 644 "result/$NAME.asi" "$DEST/.$NAME.asi.new"
mv -f "$DEST/.$NAME.asi.new" "$DEST/$NAME.asi"
[[ -f "$DEST/$NAME.ini" ]] || install -m 644 "result/$NAME.ini" "$DEST/"
cmp "result/$NAME.asi" "$DEST/$NAME.asi"
echo "deployed $DEST/$NAME.asi, log: $DEST/$NAME.log"
