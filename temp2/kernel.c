#include "tool.h"

void kernel_main() {
    char input[512];

    clear_screen();
    puts("ShadowOS -v1.0\n");
    puts("Type 'help' for available commands\n\n");
    detect_disk();
    
    while (1) {
         
        puts("\n$ ");
        gets(input, sizeof(input));
        
         if (strcmp(input, "exit") == 0) {
            puts("\n\n********** SYSTEM ABORT **********\n\n");
            break;
        }
        
        process_command(input);
        
    }
    
}




