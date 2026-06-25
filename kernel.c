#include "kernel.h"

#define KEY_ENTER 0x1C
#define KEY_SPACE 0x39

const char* MSG = "Ruto must go";

static void delay(volatile int n) {
    for (volatile int i = 0; i < n; i++);
}

static void clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA[i] = vga_entry(' ', vga_attr(WHITE, BLACK));
}

static void writestr_at(int row, int col, const char* str, uint8_t fg, uint8_t bg) {
    uint8_t attr = vga_attr(fg, bg);
    int pos = row * VGA_WIDTH + col;
    for (int i = 0; str[i]; i++)
        VGA[pos + i] = vga_entry(str[i], attr);
}

static void writechar_at(int row, int col, char c, uint8_t fg, uint8_t bg) {
    VGA[row * VGA_WIDTH + col] = vga_entry(c, vga_attr(fg, bg));
}

static void draw_horiz(int row, int col, int len, int ch, uint8_t fg) {
    uint8_t attr = vga_attr(fg, BLACK);
    int pos = row * VGA_WIDTH + col;
    for (int i = 0; i < len; i++)
        VGA[pos + i] = vga_entry(ch, attr);
}

static void draw_box(int top, int left, int height, int width,
                     int tl, int tr, int bl, int br,
                     int horiz, int vert, uint8_t fg) {
    writechar_at(top, left, tl, fg, BLACK);
    writechar_at(top, left + width - 1, tr, fg, BLACK);
    writechar_at(top + height - 1, left, bl, fg, BLACK);
    writechar_at(top + height - 1, left + width - 1, br, fg, BLACK);
    draw_horiz(top, left + 1, width - 2, horiz, fg);
    draw_horiz(top + height - 1, left + 1, width - 2, horiz, fg);
    for (int i = 1; i < height - 1; i++) {
        writechar_at(top + i, left, vert, fg, BLACK);
        writechar_at(top + i, left + width - 1, vert, fg, BLACK);
    }
}

static void draw_screen_border(void) {
    writechar_at(0, 0, BOX_TTL, WHITE, BLACK);
    writechar_at(0, VGA_WIDTH - 1, BOX_TTR, WHITE, BLACK);
    writechar_at(VGA_HEIGHT - 1, 0, BOX_TBL, WHITE, BLACK);
    writechar_at(VGA_HEIGHT - 1, VGA_WIDTH - 1, BOX_TBR, WHITE, BLACK);
    draw_horiz(0, 1, VGA_WIDTH - 2, BOX_THORIZ, WHITE);
    draw_horiz(VGA_HEIGHT - 1, 1, VGA_WIDTH - 2, BOX_THORIZ, WHITE);
    for (int i = 1; i < VGA_HEIGHT - 1; i++) {
        writechar_at(i, 0, BOX_TVERT, WHITE, BLACK);
        writechar_at(i, VGA_WIDTH - 1, BOX_TVERT, WHITE, BLACK);
    }
}

static void draw_wantam(void) {
    writestr_at(4, 25, "  W A N T A M   O S", WHITE, BLACK);
}

static void draw_yes_button(void) {
    int btn_width = 15;
    int btn_left = (VGA_WIDTH - btn_width) / 2;
    int btn_top = 14;
    draw_box(btn_top, btn_left, 3, btn_width,
             BOX_TL, BOX_TR, BOX_BL, BOX_BR,
             BOX_HORIZ, BOX_VERT, WHITE);
    writestr_at(btn_top + 1, btn_left + 5, "YES!", WHITE, BLACK);
    writestr_at(btn_top + 5, btn_left - 4, "Press SPACE or ENTER to confirm", WHITE, BLACK);
}

static void show_acknowledged(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA[i] = vga_entry(' ', vga_attr(WHITE, BLACK));
    draw_screen_border();
    int width = 43;
    int left = (VGA_WIDTH - width) / 2;
    int top = 10;
    draw_box(top, left, 3, width,
             BOX_TTL, BOX_TTR, BOX_TBL, BOX_TBR,
             BOX_THORIZ, BOX_TVERT, WHITE);
    writestr_at(top + 1, left + 5, "Patriotism Acknowledged!", WHITE, BLACK);
    while (1) {
        delay(8000000);
    }
}

static int kb_wait_press(void) {
    while (1) {
        if (inb(PS2_STATUS) & 1) {
            uint8_t scan = inb(PS2_DATA);
            if (!(scan & 0x80)) {
                if (scan == KEY_ENTER || scan == KEY_SPACE)
                    return 1;
            }
        }
    }
}

static void type_text(void) {
    int msg_len = strlen(MSG);
    int row = 9;
    int col = (VGA_WIDTH - msg_len) / 2;
    for (int i = 0; i < msg_len; i++) {
        writechar_at(row, col + i, MSG[i], WHITE, BLACK);
        delay(8000000);
    }
}

__attribute__((section(".text.entry"))) void kernel_main(void) {
    clear_screen();
    draw_screen_border();
    draw_wantam();
    delay(25000000);
    type_text();
    draw_yes_button();
    kb_wait_press();
    show_acknowledged();
}