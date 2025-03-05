#include <stdint.h>
#include <stddef.h> 
#include <string.h>


#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F

volatile uint16_t* const video_memory = (uint16_t*)0xB8000;
uint16_t cursor_x = 0, cursor_y = 0;

// I/O Ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64


// I/O Functions
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Custom string functions (since we cannot use standard C library)
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

char *strcpy(char *dest, const char *src) {
    char *original = dest;
    while ((*dest++ = *src++));
    return original;
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


// Convert scancode to ASCII
char scancode_to_ascii(uint8_t scancode) { 
    
    static uint8_t capsLk = 0 ;
    static uint8_t shift = 0;
    
    static const char ascii_normal[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    
    static const char c_ascii_normal[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    
    static const char ascii_shifted[128] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    
    static const char cs_ascii_normal[128] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', 0, '|', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };


    if (scancode == 0x3A) { 
        capsLk = !capsLk; 
    }
    
    //Shift - ON
    if ( scancode == 0x36 || scancode == 0x2A ){
       shift = 1 ;
    }
    if ( scancode == 0xB6 || scancode == 0xAA ){
       shift = 0 ;
    }
    
   return (scancode < 128) ? 
    (capsLk) ? 
        (shift) ? cs_ascii_normal[scancode] : c_ascii_normal[scancode] 
    : 
        (shift) ? ascii_shifted[scancode] : ascii_normal[scancode] 
    : 0;

}

// Keyboard Input
char getchar() {
    uint8_t scancode;
    char ascii;
    
    while (1) {
        // Wait for keyboard data
        while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
        
        // Read the scancode
        scancode = inb(KEYBOARD_DATA_PORT);
        
        // Only process key presses (not releases)
        if (scancode) {
            ascii = scancode_to_ascii(scancode);
            if (ascii) return ascii;
        }
    }
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


// Custom Command Parser
void parse_command(const char* input, char* command, char* arg1, char* arg2) {
    int i = 0, j = 0;
    
    // Skip initial spaces
    while (input[i] == ' ') i++;
    
    // Get command
    while (input[i] != ' ' && input[i] != '\0') command[j++] = input[i++];
    command[j] = '\0';
    
    // Skip spaces
    while (input[i] == ' ') i++;
    
    // Get first argument
    j = 0;
    while (input[i] != ' ' && input[i] != '\0') arg1[j++] = input[i++];
    arg1[j] = '\0';
    
    // Skip spaces
    while (input[i] == ' ') i++;
    
    // Get second argument (rest of the input)
    j = 0;
    while (input[i] != '\0') arg2[j++] = input[i++];
    arg2[j] = '\0';
}

// Process Commands
void process_command(const char* input) {
    char command[20] = {0};
    char arg1[20] = {0};
    char arg2[20] = {0};
    
    // Skip processing if input is empty
    if (input[0] == '\0') return;
    
    parse_command(input, command, arg1, arg2);
    
    if (strcmp(command, "clear") == 0) {
        clear_screen();
    } else if (strcmp(command, "help") == 0) {
        puts("Available commands:\n");
        puts("  clear     - Clear the screen\n");
        puts("  pwd       - Print working directory\n");
        puts("  echo      - Display a message\n");
        puts("  help      - Show this help\n");
        puts("  exit      - System Abort\n");
    } else if (strcmp(command, "pwd") == 0) {
        puts("/ (root)\n");
    }  else if (strcmp(command, "echo") == 0) {
        if (arg1[0] != '\0') {
            puts(arg1);
            if (arg2[0] != '\0') {
                puts(" ");
                puts(arg2);
            }
            puts("\n");
        } else {
            puts("\n");
        }
    }  
    else {
        puts("Unknown command: ");
        puts(command);
        puts("\nType 'help' for available commands.\n");
    }
}


