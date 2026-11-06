# hudname

MGS4 (Master Collection) ASI mod: replace a HUD layout string, by default the
life bar's "OLD SNAKE" with "GOLD SNAKE".

## Config (`hudname.ini`)

```ini
; hudname config. All keys optional.
enabled = 1
; Log every layout text set (to find other strings to replace).
verbose = 0
; HUD text to replace, exact match.
match = OLD SNAKE
; Replacement. The HUD font is uppercase; keep it short enough for the life bar.
name = GOLD SNAKE
```

## How

Hooks the layout text setters `0x30FCA0` and `0x30F6B0` and swaps `match` for
`name` before the game copies it into the node. See
[docs/MECHANISM.md](docs/MECHANISM.md).

Build `26451734` (PE timestamp `0x6AE07392`) only; on any other build nothing
is hooked.

## Build

```sh
nix build      # result/hudname.asi + result/hudname.ini
./deploy.sh    # installs into METAL GEAR SOLID 4/MGS4
```

Needs Ultimate ASI Loader (`winmm.dll`) in the game directory.
