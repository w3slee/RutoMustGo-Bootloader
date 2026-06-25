bits 16
org 0x7c00

KERNEL_SEG equ 0x1000
KERNEL_OFFSET equ 0
KERNEL_SECTORS equ 32

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    mov [BOOT_DRIVE], dl

    mov si, MSG_LOADING
    call print_string

    call load_kernel

    call switch_to_pm

    jmp $

load_kernel:
    mov ah, 0x00
    mov dl, [BOOT_DRIVE]
    int 0x13

    mov si, DAP
    mov word [si], 0x10
    mov word [si+2], KERNEL_SECTORS
    mov word [si+4], KERNEL_OFFSET
    mov word [si+6], KERNEL_SEG
    mov dword [si+8], 1
    mov dword [si+12], 0

    mov dl, [BOOT_DRIVE]
    mov ah, 0x42
    int 0x13
    jc disk_error
    ret

disk_error:
    mov si, MSG_DISK_ERROR
    call print_string
    mov al, ah
    call print_hex
    jmp $

print_hex:
    pusha
    mov cx, 4
.hex_loop:
    rol al, 4
    mov bl, al
    and bl, 0xf
    cmp bl, 10
    jl .digit
    add bl, 7
.digit:
    add bl, '0'
    mov ah, 0xe
    mov al, bl
    int 0x10
    loop .hex_loop
    popa
    ret

print_string:
    pusha
    mov ah, 0x0e
    .loop:
        lodsb
        test al, al
        jz .done
        int 0x10
        jmp .loop
    .done:
        popa
        ret

switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

bits 32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    call 0x10000
    jmp $

gdt_start:
    dq 0
gdt_code:
    dw 0xffff
    dw 0
    db 0
    db 0x9a
    db 0xcf
    db 0
gdt_data:
    dw 0xffff
    dw 0
    db 0
    db 0x92
    db 0xcf
    db 0
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

BOOT_DRIVE db 0
MSG_LOADING db "Loading WANTAM OS...", 0x0d, 0x0a, 0
MSG_DISK_ERROR db "Disk read error!", 0x0d, 0x0a, 0

DAP:
    db 0x10
    db 0
    dw KERNEL_SECTORS
    dw KERNEL_OFFSET
    dw KERNEL_SEG
    dd 1
    dd 0

times 510 - ($ - $$) db 0
dw 0xaa55