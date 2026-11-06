# How hudname works

Build `26451734` (PE timestamp `0x6AE07392`). All addresses are RVAs from a
live, decrypted image of that build.

## Where the text comes from

The life bar label is not a texture or a localized string. It is an inline
text record in the HUD layout file,
`common/stage/stage00/s10a30l/cache/00b66172.la2` in
`common/stage/stage_data_compressed.1.pak`, with per-stage copies in the `ww`
paks:

```
0D 00 14 00   tag 0x0D, record size 0x14
09 00 00 00   length 9
"OLD SNAKE\0" padded to the record size
```

The game byte-swaps the layout to little endian after loading it. "STRESS :"
sits in the next record. The record has room for 11 characters, so patching
the file would limit the replacement's length.

## How it reaches the screen

When the layout loads, its loader (`0x308EE3`) walks the text nodes and
copies each record's string into the node:

- nodes flagged `0x1000000`, the HUD label among them, go through `0x30FCA0`,
  which expands `\\`, `\n`, `\r` and `\x..` escapes;
- the rest go through `0x30F6B0`, a plain copy that menus also call at
  runtime.

Both take `(ctx, node, text)`, allocate `strlen + 1` bytes from the layout's
allocator at `ctx+0x20`, copy, and store the pointer at `node+0x38`. The
renderer draws from that buffer. A gdb hardware read watch on the layout's
copy of "OLD SNAKE" during a checkpoint reload stopped in `0x30FCA0`.

## The hook

`hudname` hooks both setters with MinHook. When the incoming text equals
`match` exactly, it passes `name` instead. The game then allocates a buffer
sized for the new string, so the replacement is not bound by the record's
size; the life bar fits about 15 characters on screen. The swap happens once
per layout load, with no per-frame cost.

Only one other layout carries the string: the options screen's "DISPLAY /
Life / Psyche Gauge" preview (`ww/stage/slot/00003ad4/cache/0090329a.la2`,
two nodes), which loads the same way and shows the new name too. The
stage scripts (`00180720.gcx` in every stage) hold one "OLD SNAKE" string
resource each; what uses it is not known. The retro HUDs of the final
Act 5 fight label the bar LIFE or SNAKE, so the hook leaves them alone.

`name` and `match` must be 1 to 32 printable ASCII characters with no
backslash; otherwise the mod logs an error and installs nothing. The
backslash rule matters: `0x30FCA0` skips four bytes after `\x` without
looking for the terminator, so a `name` ending in `\x` would make it read
past the string.

Each hook site carries its expected prologue bytes. Install waits for the
decrypted `.text` to match them and enables both hooks or neither, so on any
other build the mod logs an error and changes nothing.
