//Tool - Operation System

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>


#define MAX_FILES        9
#define MAX_FILENAME_LEN 32
#define VGA_WIDTH        80
#define VGA_HEIGHT       25
#define WHITE_ON_BLACK   0x0F
#define ATA_PRIMARY_IO   0x1F0
#define ATA_READY        0x08
#define ATA_DRQ          0x08  
#define ATA_BSY          0x80  
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ATA_STATUS      (ATA_PRIMARY_IO + 7)

volatile uint16_t* const video_memory = (uint16_t*)0xB8000;
uint16_t cursor_x = 0, cursor_y = 0;
uint32_t meta_lba   = 1 ;
uint32_t file_count = 0 ;                  
uint32_t file_lba   = 2 ;  

void scroll();
void test_fs(); 
void io_wait();
char getchar();
void print_help(); 
void detect_disk();
int save_metafile();
int load_metafile();
void clear_screen();
void get_date_time();
void update_cursor();
void putchar(char c);
void print_dec(int num);
void print_binary(int num);
void puts(const char* str);
void print_hex(uint32_t num);
size_t strlen(const char *s);
void print_working_directory();
uint8_t bcd_to_bin(uint8_t bcd);
void printf(const char* format, ...);
void print_scancode(uint8_t scancode);
uint8_t read_rtc_register(uint8_t reg);
void process_command(const char* input);
void gets(char* buffer, int max_length);
char scancode_to_ascii(uint8_t scancode);
void read_file_sys(const char* filename);
void itoa(int value, char* str, int base);
void remove_file_sys(const char* filename);
int strcmp(const char *s1, const char *s2);
void create_file_sys(const char* filename);
int count_char(const char *str, char target);
void write_to_file_sys(const char* filename);
void echo(const char* arg1, const char* arg2);
int is_empty_asm(uint8_t *buffer, size_t size);
int read_file_data( const char *read_file_name );
char *strncpy(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncatt(char dest[], const char src[], size_t n);
void format_filename(const char* original, char* result , int lba_value);
void parse_command(const char* input, char* command, char* arg1, char* arg2);
int write_file_data(const char *write_file_name, const char *write_file_data);
void get_time(int* year, int* month, int* day, int* hour, int* min, int* sec);
void parse_key_value_pairs(const char *input, int keys[], char values[MAX_FILES][MAX_FILENAME_LEN], int *count);

static inline void outio(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inio(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outio16(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inio16(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void io_wait() {
    outio(0x80, 0); 
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

static inline void outtime(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t intime(uint16_t port) {
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t read_rtc_register(uint8_t reg) {
    outtime(0x70, reg);     
    return intime(0x71);     
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

void get_time(int* year, int* month, int* day, int* hour, int* min, int* sec) {
    *year  = bcd_to_bin(read_rtc_register(0x09)) + 2000;  // RTC year
    *month = bcd_to_bin(read_rtc_register(0x08));         // RTC month
    *day   = bcd_to_bin(read_rtc_register(0x07));         // RTC day
    *hour  = bcd_to_bin(read_rtc_register(0x04));         // RTC hour
    *min   = bcd_to_bin(read_rtc_register(0x02));         // RTC minutes
    *sec   = bcd_to_bin(read_rtc_register(0x00));         // RTC seconds
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

char *strncatt(char dest[], const char src[], size_t n) {
    int i = 0, j = 0;
    while (dest[i] != '\0') {
        i++;
    }
    while (src[j] != '\0' && j < n) {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
    return dest;
}

int count_char(const char *str, char target) {
    int count = 0;
    while (*str) {
        if (*str == target) {
            count++;
        }
        str++;
    }
    return count;
}

void format_filename(const char* original, char* result , int lba_value) {
    char str[20];
    result[0] = ' ';
    itoa( lba_value , str, 10);
    result[1] = str[0] ;
    result[2] = ':' ;
    int i = 0;
    while (original[i] != '\0') {
        result[i + 3] = original[i];
        i++;
    }
    
    result[i + 3] = ' ';
    result[i + 4] = ',';
    result[i + 5] = '\0';
    printf( "%s" , result );
}

void parse_key_value_pairs(const char *input, int keys[], char values[MAX_FILES][MAX_FILENAME_LEN], int *count) {
    *count = 0;  // Initialize the count

    while (*input && *count < MAX_FILES) {
        // Skip leading spaces
        while (*input == ' ') input++;

        // Read integer key manually
        int key = 0;
        while (*input >= '0' && *input <= '9') {
            key = key * 10 + (*input - '0');
            input++;
        }

        // Check if ':' exists
        if (*input != ':') return;
        input++; // Skip ':'

        // Skip leading spaces after ':'
        while (*input == ' ') input++;

        // Read filename manually
        int i = 0;
        while (*input && *input != ',' && i < MAX_FILENAME_LEN - 1) {
            values[*count][i++] = *input++;
        }

        // Trim trailing spaces
        while (i > 0 && values[*count][i - 1] == ' ') {
            i--;
        }
        values[*count][i] = '\0'; // Null-terminate the string

        // Store the extracted key
        keys[*count] = key;
        
        // Increase the count
        (*count)++;

        // Skip any spaces or commas
        while (*input == ',' || *input == ' ') input++;
    }
}

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
    
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        video_memory[i] = video_memory[i + VGA_WIDTH];
    }
    
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

void process_command(const char* input) {
    char command[50] = {0};
    char arg1[50] = {0};
    char arg2[100] = {0};
    
    // Skip processing if input is empty
    if (input[0] == '\0') return;
    
    parse_command(input, command, arg1, arg2);
    
    // Execute corresponding functions
    if (strcmp(command, "clear") == 0) {
        clear_screen();
    } else if (strcmp(command, "help") == 0) {
        print_help();
    } else if (strcmp(command, "pwd") == 0) {
        print_working_directory();
    } else if (strcmp(command, "echo") == 0) {
        echo(arg1, arg2);
    } else if (strcmp(command, "create") == 0) {
        create_file_sys(arg1);
    } else if (strcmp(command, "write") == 0) {
        write_to_file_sys(arg1);
    } else if (strcmp(command, "read") == 0) {
        read_file_sys(arg1);
    } else if (strcmp(command, "remove") == 0) {
        remove_file_sys(arg1);
    } else if (strcmp(command, "list") == 0) {
        load_metafile();
    } else if (strcmp(command, "time") == 0) {
        get_date_time();
    } else if (strcmp(command, "date") == 0) {
        get_date_time();
    } else {
        puts("Unknown command: ");
        puts(command);
        puts("\nType 'help' for available commands.\n");
    }
}

void print_help() {
    puts("Available commands:\n");
    puts("  clear     - Clear the screen.\n");
    puts("  pwd       - Print working directory.\n");
    puts("  echo      - Display a message.\n");
    puts("  create    - Create a new file.\n");
    puts("  write     - Write content to a file.\n");
    puts("  read      - Read content from a file.\n");
    puts("  remove    - Delete a file.\n");
    puts("  list      - List all files.\n");
    puts("  time      - Get time.\n");
    puts("  date      - Get Date.\n");
    puts("  help      - Show this help.\n");
    puts("  exit      - System Abort.\n");
}

void print_working_directory() {
    puts("/ (root)\n");
}

void echo(const char* arg1, const char* arg2) {
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

void create_file_sys(const char* filename) {
    if (filename[0] != '\0') {
        puts("Creating file: ");
        puts(filename);
        puts("\nFile created successfully.\n");
        // Call file system function here
        // fs_create(filename);
        save_metafile(filename);
    } else {
        puts("Usage: create <filename>\n");
    }
}

void write_to_file_sys(const char* filename) {
    if (filename[0] != '\0') {
        puts("Enter content for file : ");
        puts(filename);
        puts(" (end with empty line):\n");

        char content[200] = {0};
        gets(content, sizeof(content));

        puts("Content written to file: ");
        puts(filename);
        puts("\n");
        // Call file system function here
        // fs_write(filename, content, strlen(content));
        write_file_data( filename , content );
    } else {
        puts("Usage: write <filename>\n");
    }
}

void read_file_sys(const char* filename) {
    if (filename[0] != '\0') {
        puts("Contents of file ");
        puts(filename);
        puts(":\n");

        // Simulated read
        //puts("[Simulated file content would appear here]\n");
        // Call file system function here
        // fs_read(filename, buffer, sizeof(buffer));
        read_file_data( filename );
    } else {
        puts("Usage: read <filename>\n");
    }
}

void remove_file_sys(const char* filename) {
    if (filename[0] != '\0') {
        puts("Removing file: ");
        puts(filename);
        puts("\nFile removed successfully.\n");

        // Call file system function here
        // fs_remove(filename);
    } else {
        puts("Usage: remove <filename>\n");
    }
}

void get_date_time(){
     int year, month, day, hour, min, sec;
     get_time(&year, &month, &day, &hour, &min, &sec);
     
     
     printf("Current Date and Time: %d-%d-%d %d:%d:%d\n",year, month, day, hour , min , sec);
}

void detect_disk() {
    uint8_t status = inio(ATA_STATUS);
    if (status == 0) {
        puts("Disk Status : No Memory Devices Detected.\n");
    } else {
        puts("Disk Status : Memory Device Detected!\n");
    }
}

int ata_read_sector(uint32_t lba, uint8_t *buffer) {
    printf("Reading LBA: %d\n", lba);

    outio(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    io_wait();
    
    outio(ATA_PRIMARY_IO + 1, 0);           
    outio(ATA_PRIMARY_IO + 2, 1);           
    outio(ATA_PRIMARY_IO + 3, lba & 0xFF);  
    outio(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);  
    outio(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF); 
    io_wait();
    
    outio(ATA_PRIMARY_IO + 7, 0x20);
    io_wait();
    
    int timeout = 100000;
    uint8_t status;
    do {
        status = inio(ATA_STATUS);
        if (--timeout == 0) {
            printf("Error: Disk timeout waiting for data!\n");
            return -1;
        }
    } while ((status & ATA_BSY) || !(status & ATA_DRQ));
    
    //printf("Drive ready, reading data...\n");
    
    for (int i = 0; i < 256; i++) {
        uint16_t data = inio16(ATA_PRIMARY_IO);
        buffer[i * 2] = data & 0xFF;        // Low byte
        buffer[i * 2 + 1] = (data >> 8);    // High byte
        /*
        // Only print the first few values for debugging
        if (i < 5) {
            printf("[%d]: Read 0x%04X (%c%c)\n", i, data, 
                   (buffer[i * 2] >= 32 && buffer[i * 2] <= 126) ? buffer[i * 2] : '.',
                   (buffer[i * 2 + 1] >= 32 && buffer[i * 2 + 1] <= 126) ? buffer[i * 2 + 1] : '.');
        }
        */
    }
    
    printf("Data read complete!\n");
    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t *buffer) {
    printf("Writing LBA: %d\n", lba);
    
    outio(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    io_wait();
    
    outio(ATA_PRIMARY_IO + 1, 0);           
    outio(ATA_PRIMARY_IO + 2, 1);           
    outio(ATA_PRIMARY_IO + 3, lba & 0xFF);  
    outio(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);  
    outio(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF); 
    io_wait();
    
    outio(ATA_PRIMARY_IO + 7, 0x30);
    io_wait();
    
    int timeout = 100000;
    uint8_t status;
    do {
        status = inio(ATA_STATUS);
        if (--timeout == 0) {
            printf("Error: Disk timeout waiting for ready state!\n");
            return -1;
        }
    } while ((status & ATA_BSY) || !(status & ATA_DRQ));
    
    //printf("Writing data...\n");
    
    for (int i = 0; i < 256; i++) {
        uint16_t word = buffer[i * 2] | ((buffer[i * 2 + 1] << 8) & 0xFF00);
        outio16(ATA_PRIMARY_IO, word);
        /*
        // Only print the first few values for debugging
        if (i < 5) {
            printf("[%d]: Writing 0x%04X (%c%c)\n", i, word, 
                   (buffer[i * 2] >= 32 && buffer[i * 2] <= 126) ? buffer[i * 2] : '.',
                   (buffer[i * 2 + 1] >= 32 && buffer[i * 2 + 1] <= 126) ? buffer[i * 2 + 1] : '.');
        }
        */
    }
    
    timeout = 100000;
    do {
        status = inio(ATA_STATUS);
        if (--timeout == 0) {
            printf("Error: Disk timeout after write!\n");
            return -1;
        }
    } while (status & ATA_BSY);
    
    printf("Write complete!\n");
    return 0;
}

/*
void test_persistent_storage() {
    uint8_t buffer[512];
    
    detect_disk() ;
    
    // Initialize buffer with zeros
    memset(buffer, 0, 512);
    
    // Write test string
    strcpy((char*)buffer, "I am Abinesh!");
    
    
    // Debug Print Before Writing
    printf("Before writing (decimal values):\n");
    for (int i = 0; i < 32; i++) {
        printf("%d ", buffer[i]);  
    }
    printf("\n");
    
    printf("Write raw hex values:\n");
    for (int i = 0; i < 32; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
    
    
    
    // Write to Disk
    if (ata_write_sector(1, buffer) != 0) {
        printf("Write failed!\n");
        return;
    }
    
    
    // Clear Buffer Before Reading
    memset(buffer, 0, 512);
    
    // Read from Disk
    if (ata_read_sector(1, buffer) != 0) {
        printf("Read failed!\n");
        return;
    }
    
    
    // Print buffer as a string
    printf("Read from disk: %s\n", buffer);
}

*/

//Check is Buffer is Empty
int is_empty_asm(uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (buffer[i] != 0)
            return 0;
    }
    return 1;
}

//load_metafile
int load_metafile( ) {
     uint8_t buffer[512];
     const char *input ;
     int keys[MAX_FILES];
     char values[MAX_FILES][MAX_FILENAME_LEN];
     int count = 0;
     
     memset(buffer, 0, 512);
     if (ata_read_sector( meta_lba , buffer) != 0) {
        printf("Read failed!\n");
        return -1 ;
    }

    if (is_empty_asm(buffer, 512)) {
       printf("Buffer is empty\n");
    } else {
      //writeprintf("\nFILES : %s\n", buffer );
      input = (const char *)buffer; 
      parse_key_value_pairs(input, keys, values, &count);
      for (int i = 0; i < count; i++) {
        printf("\nKey: %d, Value: %s\n", keys[i], values[i]);
      }
    }

return 1 ;
}

//save_metafile
int save_metafile(const char *file_name) {
     uint8_t buffer[512];
     char formatted[50];
     char target = ',';
             
     memset(buffer, 0, 512);
     if (ata_read_sector( meta_lba , buffer) != 0) {
        printf("Read failed!\n");
        return -1 ;
     }

    if (is_empty_asm(buffer, 512)) {
       format_filename(file_name, formatted , 1 );
       strcpy((char*)buffer,  formatted );
    } else {
       int count = count_char(  buffer , target);
       format_filename( file_name , formatted , count + 1 );
       strncatt((char *)buffer, (char *)formatted , sizeof(buffer) - strlen((char *)buffer) - 1);
       strcpy( (char*)buffer,  buffer );
    }
     
     if (ata_write_sector( meta_lba , buffer) != 0) {
        printf("Write failed!\n");
        return -1 ;
    }
    
return 1 ;
}


int write_file_data(const char *write_file_name, const char *write_file_data) {
    uint8_t buffer[512];
    const char *input;
    int keys[MAX_FILES];
    char values[MAX_FILES][MAX_FILENAME_LEN];
    int count = 0;

    memset(buffer, 0, 512);
    if (ata_read_sector( meta_lba , buffer) != 0) {
        printf("Read failed!\n");
        return -1;
    }

    input = (const char *)buffer;  
    parse_key_value_pairs(input, keys, values, &count);
    
    for (int i = 0; i < count; i++) {
        printf("\nKey: %d, Value:%s:\n", keys[i], values[i]);
        //printf("COMARE : %d \n", strcmp( "file1.txt" , write_file_name) );
        if (strcmp(values[i], write_file_name) == 0) {
            memset(buffer, 0, 512);
            strcpy((char*)buffer, write_file_data );
            if (ata_write_sector(keys[i] + 10, buffer) != 0) {
                printf("Write failed!\n");
                return -1;
            }
            printf("Write successful at LBA: %d\n", keys[i] + 10);
            return 0;
        }
    }
    printf("File not found: %s\n", write_file_name);
    return -1;
}

int read_file_data( const char *read_file_name ) {
    uint8_t buffer[512];
    const char *input;
    int keys[MAX_FILES];
    char values[MAX_FILES][MAX_FILENAME_LEN];
    int count = 0;

    memset(buffer, 0, 512);
    if (ata_read_sector( meta_lba , buffer) != 0) {
        printf("Read failed!\n");
        return -1;
    }

    input = (const char *)buffer;  
    parse_key_value_pairs(input, keys, values, &count);
    
    for (int i = 0; i < count; i++) {
        if (strcmp(values[i], read_file_name) == 0) {
            memset(buffer, 0, 512);
            if (ata_read_sector(keys[i] + 10, buffer) != 0) {
                printf("Read failed!\n");
                return -1;
            }
            printf("\nData : %s \n", buffer );
            printf("Read successful at LBA: %d\n", keys[i] + 10);
            return 0;
        }
    }
    printf("File not found: %s\n", read_file_name);
    return -1;
}

void test_fs() {
     load_metafile();
     save_metafile("file1.txt");
     load_metafile();
     save_metafile("file2.txt");
     load_metafile();
     write_file_data("file1.txt" , "hello");
     read_file_data("file1.txt");
return;
}
