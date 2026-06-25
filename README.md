# WANTAM OS

A minimal x86 operating system that boots to display "Ruto must go"
with an interactive button. White text on black, double-line border.

## Features

- Boot screen: "WANTAM OS" centred on a black screen with white double-line border
- Typing animation: "Ruto must go" types out character by character
- Interactive YES button — press Space or Enter to confirm
- Acknowledgement screen: "Patriotism Acknowledged!" in a bordered box

## Files

| File | Purpose |
|------|---------|
| `boot.asm` | 16-bit bootloader (LBA disk read, 32-bit protected mode switch) |
| `kernel.c` | 32-bit kernel (VGA text mode, keyboard input, animation) |
| `kernel.h` | VGA constants, colours, I/O port helpers |
| `linker.ld` | Links kernel at address 0x10000 |
| `Makefile` | Build automation |

## Requirements

- `nasm` — x86 assembler
- `gcc` (multilib) — 32-bit C compiler
- `ld` (elf_i386) — linker
- `make`
- `qemu-system-x86_64` — emulator
- `vncviewer` (tigervnc) — to view the display

## Build & Run

```
make clean && make
make run
```

In another terminal:
```
vncviewer localhost:5901
```

## OS Flow

1. SeaBIOS boots → loads kernel from disk
2. WANTAM OS banner appears (centred, double-line border)
3. "Ruto must go" types out character by character
4. YES button appears below
5. Press SPACE or ENTER → "Patriotism Acknowledged!" displayed

## Technical Notes

- Raw disk image (10 MB), kernel loaded at 0x10000
- VGA text mode 80×25, writes directly to 0xB8000
- PS/2 keyboard polling via ports 0x60 / 0x64
- All visual output is white-on-black with box-drawing characters (code page 437)
