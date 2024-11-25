BITS 16
ORG 0x7C00

boot:
    ; Set up the stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Print "Ruto Must Go"
    mov si, message
    call print_string

    ; Infinite loop to halt the system
    jmp $

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string

done:
    ret

message db "Ruto Must Go", 0

times 510-($-$$) db 0
dw 0xAA55
