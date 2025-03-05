
#include "tool.h"

// Kernel Entry
void kernel_main() {
    char input[256];

    clear_screen();
    puts("SimpleOS v1.0\n");
    puts("Type 'help' for available commands\n\n");

    while (1) {
        puts("$ ");
        gets(input, sizeof(input));
        if ( strcmp( input , "exit" ) == 0 ){
            break ;
        }
        process_command(input);
    }
    puts("\n\n********** SYSTEM ABORT **********\n\n");
}




