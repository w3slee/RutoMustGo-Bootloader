#include <stdint.h>

void print_string(const char* str) {
    while (*str) {
        __asm__ __volatile__ (
            "movb $0x0E, %%ah\n"
            "int $0x10"
            :
            : "a" (*str)
        );
        str++;
    }
}

void kernel_main() {
    print_string("Ruto Must Go");

    // Halt the system
    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}
