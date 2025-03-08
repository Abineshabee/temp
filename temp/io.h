#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

volatile uint16_t* const video_memory = (uint16_t*)0xB8000;
uint16_t cursor_x = 0, cursor_y = 0;

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
void putchar(char c);
void puts(const char* str);
void print_dec(int num);
void print_hex(uint32_t num);
void print_scancode(uint8_t scancode);
void print_binary(int num);
char getchar();
void itoa(int value, char* str, int base);
void gets(char* buffer, int max_length);


// Write byte to port
static inline void outio(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Read byte from port
static inline uint8_t inio(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// I/O Functions
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Write 16-bit word to port
static inline void outio16(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Read 16-bit word from port
static inline uint16_t inio16(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void io_wait() {
    outio(0x80, 0); // A small delay by writing to an unused I/O port
}


void *memset(void *ptr, int value, size_t num) {
    uint8_t *p = (uint8_t*)ptr;
    while (num--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t*)dest;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

char *strcpy(char *dest, const char *src) {
    char *original = dest;
    while ((*dest++ = *src++));
    return original;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return n == 0 ? 0 : (*(const unsigned char*)s1 - *(const unsigned char*)s2);
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *original = dest;
    while (n-- && (*dest++ = *src++));
    return original;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

// VGA Functions
void update_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos));
}

void clear_screen() {
    uint16_t blank = (WHITE_ON_BLACK << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void scroll() {
    uint16_t blank = (WHITE_ON_BLACK << 8) | ' ';
    
    // Move all lines up
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        video_memory[i] = video_memory[i + VGA_WIDTH];
    }
    
    // Clear the last line
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        video_memory[i] = blank;
    }
    
    cursor_y--;
}

void putchar(char c) {
    uint16_t attribute = WHITE_ON_BLACK << 8;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            video_memory[cursor_y * VGA_WIDTH + cursor_x] = attribute | ' ';
        }
    } else {
        video_memory[cursor_y * VGA_WIDTH + cursor_x] = attribute | c;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        scroll();
    }
    
    update_cursor();
}

void puts(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}



void print_string(const char *str) {
    while (*str) {
        putchar(*str++);
    }
}

void print_dec(int num) {
    if (num == 0) {
        putchar('0');
        return;
    }
    char buffer[12];  // Enough for 32-bit int
    int i = 0;
    while (num) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (--i >= 0) {
        putchar(buffer[i]);
    }
}

void print_hex(uint32_t num) {
    puts("0x");
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        putchar(hex_chars[(num >> i) & 0xF]);
    }
}


void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;  // Move past '%'
            switch (*format) {
                case 's': {
                    char *str = va_arg(args, char*);
                    puts(str);
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    print_dec(num);
                    break;
                }
                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    print_hex(num);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    break;
                }
                case '%': {
                    putchar('%');
                    break;
                }
                default: {
                    putchar('%');
                    putchar(*format);  // Print the unknown format specifier
                    break;
                }
            }
        } else {
            putchar(*format);
        }
        format++;
    }

    va_end(args);
}


void itoa(int value, char* str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    if (base < 2 || base > 36) { *str = '\0'; return; }  // Invalid base check

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[tmp_value % base];
    } while (value);

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

// Function to print scancode (assuming puts() exists)
void print_scancode(uint8_t scancode) {
    char buffer[4];  // Buffer for scancode conversion to string

    itoa(scancode, buffer, 16);  // Convert scancode to hex string
    puts("Scancode: 0x");
    puts(buffer);
    puts("\n");
}

void print_binary(int num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        putchar((num & (1 << i)) ? '1' : '0');
    }
    puts("\n");
}


void gets(char* buffer, int max_length) {
    int i = 0;
    char c;
    
    while (i < max_length - 1) {
        c = getchar();
        
        if (c == '\n' || c == '\r') {
            putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                putchar('\b');
            }
        } else {
            buffer[i++] = c;
            putchar(c);
        }
    }
    
    buffer[i] = '\0';
}







