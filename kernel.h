#ifndef KERNEL_H
#define KERNEL_H

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA ((uint16_t*)VGA_BUFFER)

#define PS2_DATA   0x60
#define PS2_STATUS 0x64

#define BOX_HORIZ  0xC4
#define BOX_VERT   0xB3
#define BOX_TL     0xDA
#define BOX_TR     0xBF
#define BOX_BL     0xC0
#define BOX_BR     0xD9
#define BOX_THORIZ 0xCD
#define BOX_TVERT  0xBA
#define BOX_TTL    0xC9
#define BOX_TTR    0xBB
#define BOX_TBL    0xC8
#define BOX_TBR    0xBC

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef int size_t;

enum vga_color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GREY = 7,
    DARK_GREY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

static inline uint8_t vga_attr(uint8_t fg, uint8_t bg) {
    return (bg << 4) | fg;
}

static inline uint16_t vga_entry(char c, uint8_t attr) {
    return (uint16_t)attr << 8 | (uint16_t)c;
}

static inline size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void kernel_main(void);

#endif