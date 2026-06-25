#include "kernel.h"

#define KEY_ENTER 0x1C
#define KEY_SPACE 0x39
#define KEY_ESC   0x01
#define KEY_M1    0x32
#define KEY_M2    0x3A

const char* MSG = "Ruto must go";

// -------- Existing utilities --------
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

// -------- Write integer at position --------
static void write_int_at(int row, int col, int n, uint8_t fg, uint8_t bg) {
    char buf[12];
    int i = 11;
    buf[11] = 0;
    if (n == 0) {
        buf[--i] = '0';
    } else {
        while (n > 0 && i > 0) {
            buf[--i] = '0' + (n % 10);
            n /= 10;
        }
    }
    writestr_at(row, col, buf + i, fg, bg);
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
    writestr_at(btn_top + 3, btn_left - 4, "Press SPACE or ENTER to confirm", WHITE, BLACK);
}

static void draw_mpigs_button(void) {
    int btn_width = 15;
    int btn_left = (VGA_WIDTH - btn_width) / 2;
    int btn_top = 19;
    draw_box(btn_top, btn_left, 3, btn_width,
             BOX_TL, BOX_TR, BOX_BL, BOX_BR,
             BOX_HORIZ, BOX_VERT, WHITE);
    writestr_at(btn_top + 1, btn_left + 3, "mpigs", WHITE, BLACK);
    writestr_at(btn_top + 3, btn_left + 1, "Press M to view", WHITE, BLACK);
}

static void drain_kbd(void);
static uint8_t read_key(void);

static void show_acknowledged(void) {
    clear_screen();
    draw_screen_border();

    // Kenyan flag (rows 2-9, columns 8-71 on white background)
    for (int r = 2; r <= 9; r++)
        for (int c = 8; c <= 71; c++)
            VGA[r * VGA_WIDTH + c] = vga_entry(' ', vga_attr(BLACK, WHITE));

    // Black stripe (rows 2-3)
    for (int c = 8; c <= 71; c++) {
        VGA[2 * VGA_WIDTH + c] = vga_entry(0xDB, vga_attr(BLACK, WHITE));
        VGA[3 * VGA_WIDTH + c] = vga_entry(0xDB, vga_attr(BLACK, WHITE));
    }
    // White fimbriation is row 4 (already white bg)
    // Red stripe (rows 5-6)
    for (int c = 8; c <= 71; c++) {
        VGA[5 * VGA_WIDTH + c] = vga_entry(0xDB, vga_attr(RED, WHITE));
        VGA[6 * VGA_WIDTH + c] = vga_entry(0xDB, vga_attr(RED, WHITE));
    }
    // White fimbriation is row 7
    // Green stripe (rows 8-9)
    for (int c = 8; c <= 71; c++) {
        VGA[8 * VGA_WIDTH + c] = vga_entry(0xDB, vga_attr(GREEN, WHITE));
        VGA[9 * VGA_WIDTH + c] = vga_entry(0xDB, vga_attr(GREEN, WHITE));
    }

    // Text
    writestr_at(12, 26, "Patriotism Acknowledged!", WHITE, BLACK);
    writestr_at(23, 20, "Press ESC to return to main menu", WHITE, BLACK);

    // Wait for ESC or Q to go back
    drain_kbd();
    while (1) {
        uint8_t k = read_key();
        if (k == KEY_ESC) return;
    }
}

// -------- Drain PS/2 buffer (keyboard + mouse) --------
static void drain_kbd(void) {
    for (int i = 0; i < 256; i++) {
        if (!(inb(PS2_STATUS) & 1)) break;
        inb(PS2_DATA);
    }
}

// -------- Read a single key press (blocking, filters mouse) --------
static uint8_t read_key(void) {
    while (1) {
        uint8_t st = inb(PS2_STATUS);
        if (st & 1) {
            if (st & 0x20) {
                inb(PS2_DATA);
                continue;
            }
            uint8_t scan = inb(PS2_DATA);
            if (!(scan & 0x80))
                return scan;
        }
    }
}

// -------- MPIGS Data --------
#define MPIGS_N 71
static const char* MPIGS[MPIGS_N] = {
    "Abdi Ali (NAP-K, Ijara, Garissa)",
    "Abdikadir Hussein (ODM, Lagdera, Garissa)",
    "Abdirahman Mohamed Abdi (JP, Lafey, Mandera)",
    "Abdul Rahim Dawood (IND, North Imenti, Meru)",
    "Adagala Beatrice Kahai (ANC, Vihiga)",
    "Adan Haji Yussuf (UDM, Mandera West, Mandera)",
    "Adhe Ali Wario Guyo (KANU, North Horr, Marsabit)",
    "Adow Aden Mohammed (ODM, Wajir South, Wajir)",
    "Ali Abdisirat Khalif (UDA, Mandera)",
    "Amina Dika Abdullahi (KANU, Tana River)",
    "Anthony Kimani Ichung'wah (UDA, Kikuyu, Kiambu)",
    "Anthony Njoroge Wainaina (UDA, Kieni, Nyeri)",
    "Bartoo Phyllis Jepkemoi (UDA, Moiben, Uasin Gishu)",
    "Baya Owen Yaa (UDA, Kilifi North, Kilifi)",
    "Benjamin Kipkirui Langat (UDA, Ainamoi, Kericho)",
    "Bernard Kitur Kibor (UDA, Nandi Hills, Nandi)",
    "Boss Gladys Jepkosgei (UDA, Uasin Gishu)",
    "Bowen David Kangongo (UDA, Marakwet East, Elgeyo Marakwet)",
    "Caleb Mule Mutiso (MCCP, Machakos Town, Machakos)",
    "Caroli Omondi (ODM, Suba South, Homa Bay)",
    "Charity Kathambi Chepkwony (UDA, Njoro, Nakuru)",
    "Charles Gumini Gimose (ANC, Hamisi, Vihiga)",
    "Charles Kamuren (UDA, Baringo South, Baringo)",
    "Charo Kenneth Kazungu Tungule (PAA, Ganze, Kilifi)",
    "Chelule Liza Chepkorir (UDA, Nakuru)",
    "Chiforomodo Mangale Munga (UDM, Lunga Lunga, Kwale)",
    "Christopher Aseka Wangaya (ODM, Khwisero, Kakamega)",
    "Chumel Moroto Samwel (UDA, Kapenguria, West Pokot)",
    "Daniel Epuyo Nanok (UDA, Turkana West, Turkana)",
    "Daniel Wanyama Sitati (UDA, Webuye West, Bungoma)",
    "David Gikaria (UDA, Nakuru Town East, Nakuru)",
    "David Kiplagat (UDA, Soy, Uasin Gishu)",
    "David Njuguna Kiaraho (JP, Ol Kalou, Nyandarua)",
    "Didmus Wekesa Barasa Mutua (UDA, Kimilili, Bungoma)",
    "Duncan Maina Mathenge (UDA, Nyeri Town, Nyeri)",
    "Eckomas Mwengi Mutuse (MCCP, Kibwezi West, Makueni)",
    "Edwin Chege Njuguna (UDA, Kandara, Muranga)",
    "Elijah Memusi Kanchory (ODM, Kajiado Central, Kajiado)",
    "Elisha Ochieng Odhiambo (ODM, Gem, Siaya)",
    "Eric Wamumbi (UDA, Mathira, Nyeri)",
    "Ernest Ogesi Kivai Kagesi (ANC, Vihiga, Vihiga)",
    "Fabian Kyule Muli (GDDP, Kangundo, Machakos)",
    "Faith Wairimu Gitau (UDA, Nyandarua)",
    "Farah Maalim (WDM-K, Dadaab, Garissa)",
    "Farah Salah Yakub (UDA, Fafi, Garissa)",
    "Feisal Abdallah Bader Salim (UDA, Msambweni, Kwale)",
    "Ferdinard Kevin Wanyonyi (FORD-KENYA, Kwanza, Trans Nzoia)",
    "Francis Kipyegon Sigei (UDA, Sotik, Bomet)",
    "Fred Chesebe Kapondi (UDA, Mount Elgon, Bungoma)",
    "Gabriel Koshal Tongoyo (UDA, Narok West, Narok)",
    "Gachagua George (UDA, Ndaragwa, Nyandarua)",
    "Geoffrey Wandeto Mwangi (UDA, Tetu, Nyeri)",
    "George Gitonga Murugara (UDA, Tharaka, Tharaka Nithi)",
    "George Macharia Kariuki (UDA, Ndia, Kirinyaga)",
    "George Risa Sunkuyia (UDA, Kajiado West, Kajiado)",
    "Gichuki Edwin Mugo (UDA, Mathioya, Muranga)",
    "Gideon Kimaiyo (UDA, Keiyo South, Elgeyo Marakwet)",
    "Gideon Ochanda Ogolla (ODM, Bondo, Siaya)",
    "Githinji Robert Gichimu (UDA, Gichugu, Kirinyaga)",
    "Hilary Kiplang'at Kosgei (UDA, Kipkelion West, Kericho)",
    "Hussein Abdi Barre (UDA, Tarbaj, Wajir)",
    "Ikana Fredrick Lusuli (ANC, Shinyalu, Kakamega)",
    "Ikiara Dorothy Muthoni (UDA, Meru)",
    "Iraya Joseph Wainaina (UDA, Uasin Gishu)",
    "Irene Njoki Mrembo (JP, Bahati, Nakuru)",
    "Jackson Kipkemoi Kosgei (UDA, Baringo)",
    "James Githua Kamau Wamacukuru (UDA, Kabete, Kiambu)",
    "James Mwangi Gakuya (UDA, Embakasi North, Nairobi)",
    "Jeremiah Omboko Milemba (ANC, Emuhaya, Vihiga)",
    "Jerusha Mongina Momanyi (JP, Nyamira)",
    "Joash Nyamache Nyamoko (UDA, North Mugirango, Nyamira)"
};

// -------- Show mpigs screen --------
static void show_mpigs(void) {
    clear_screen();
    draw_screen_border();

    // Title
    writestr_at(1, 30, "BETRAYERS OF KENYA", WHITE, BLACK);

    // Hardcoded analytics
    writestr_at(3, 3, "Total Traitors: ", WHITE, BLACK);
    write_int_at(3, 20, MPIGS_N, WHITE, BLACK);
    writestr_at(3, 23, "(2024 Impeachment Vote YES)", WHITE, BLACK);

    writestr_at(5, 3, "Party Split:", WHITE, BLACK);
    writestr_at(6, 5, "UDA:44  ODM:8  ANC:5  JP:4  KANU:2  MCCP:2", WHITE, BLACK);
    writestr_at(7, 5, "UDM:2  IND:1  FORD-K:1  WDM-K:1  PAA:1  GDDP:1  NAP-K:1", WHITE, BLACK);

    // Navigation
    writestr_at(23, 15, "SPACE/ENTER: View List     |     ESC: Back to Menu", WHITE, BLACK);

    // Wait for key
    drain_kbd();
    while (1) {
        uint8_t k = read_key();
        if (k == KEY_ESC) return;
        if (k == KEY_ENTER || k == KEY_SPACE) break;
    }

    // ===== LIST PAGES =====
    int per = 18;
    int pages = (MPIGS_N + per - 1) / per;
    int page = 0;

    while (1) {
        clear_screen();
        draw_screen_border();

        writestr_at(1, 26, "ROLL OF SHAME  (Page ", WHITE, BLACK);
        write_int_at(1, 46, page + 1, WHITE, BLACK);
        writechar_at(1, 47, '/', WHITE, BLACK);
        write_int_at(1, 48, pages, WHITE, BLACK);
        writechar_at(1, 49, ')', WHITE, BLACK);

        int start = page * per;
        int end = start + per;
        if (end > MPIGS_N) end = MPIGS_N;

        for (int i = start; i < end; i++) {
            int r = 3 + (i - start);
            char line[76];
            int p = 0;
            int n = i + 1;
            if (n < 10) { line[p++] = ' '; line[p++] = '0' + n; }
            else { line[p++] = '0' + (n / 10); line[p++] = '0' + (n % 10); }
            line[p++] = '.'; line[p++] = ' ';
            for (int j = 0; MPIGS[i][j] && p < 74; j++)
                line[p++] = MPIGS[i][j];
            line[p] = 0;
            writestr_at(r, 3, line, WHITE, BLACK);
        }

        if (page < pages - 1)
            writestr_at(23, 18, "SPACE/ENTER: Next     |     ESC: Back to Menu", WHITE, BLACK);
        else
            writestr_at(23, 15, "SPACE/ENTER: Back to Start     |     ESC: Back to Menu", WHITE, BLACK);

        drain_kbd();
        while (1) {
            uint8_t k = read_key();
            if (k == KEY_ESC) return;
            if (k == KEY_ENTER || k == KEY_SPACE) {
                page++;
                if (page >= pages) page = 0;
                break;
            }
        }
    }
}

// -------- Type text animation --------
static void type_text(void) {
    int msg_len = strlen(MSG);
    int row = 9;
    int col = (VGA_WIDTH - msg_len) / 2;
    for (int i = 0; i < msg_len; i++) {
        writechar_at(row, col + i, MSG[i], WHITE, BLACK);
        delay(8000000);
    }
}

// -------- Wait for YES or MPIGS key --------
static void main_loop(void) {
    while (1) {
        drain_kbd();
        uint8_t k = read_key();
        if (k == KEY_ENTER || k == KEY_SPACE) {
            show_acknowledged();
        } else if (k == KEY_M1 || k == KEY_M2) {
            show_mpigs();
        }
        // Redraw main screen after returning from any screen
        clear_screen();
        draw_screen_border();
        draw_wantam();
        delay(25000000);
        type_text();
        draw_yes_button();
        draw_mpigs_button();
    }
}

// -------- Entry point --------
__attribute__((section(".text.entry"))) void kernel_main(void) {
    clear_screen();
    draw_screen_border();
    draw_wantam();
    delay(25000000);
    type_text();
    draw_yes_button();
    draw_mpigs_button();
    main_loop();
}
