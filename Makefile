NASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
DD = dd

NASMFLAGS = -f bin
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-pie -fno-pic -mno-sse -mno-sse2 -Wall -Wextra -O2
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

BUILD_DIR = build

all: $(BUILD_DIR)/ruto.img

$(BUILD_DIR)/boot.bin: boot.asm
	@mkdir -p $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.o: kernel.c kernel.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel.o linker.ld
	$(LD) $(LDFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/ruto.img: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	$(DD) if=/dev/zero of=$@ bs=512 count=20480
	$(DD) if=$(BUILD_DIR)/boot.bin of=$@ conv=notrunc
	$(DD) if=$(BUILD_DIR)/kernel.bin of=$@ bs=512 seek=1 conv=notrunc

run: $(BUILD_DIR)/ruto.img
	qemu-system-x86_64 -drive file=$<,format=raw,if=ide -boot c -vnc :1

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean