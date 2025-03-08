#include "io.h"

// Convert keyboard interrupt ( scancode ) to ASCII
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




