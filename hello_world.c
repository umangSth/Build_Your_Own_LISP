#include <stdio.h>
#include <stdlib.h>

// If we are compiling on Windows compile these functions
#ifdef _WIN32
#include <string.h>

static char buffer[2048];


// Fake Readline functions
char* readline(char* prompt){
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

// fake add_history function
void add_history(char* unused){}


// otherwise includ the editline headers
#else
#include <editline/realine.h>
#include <editline/history.h>
#endif

int main(int argc, char** argv){
    puts("Lispy Version 0.0.0.0.1");
    puts("Prese Ctrl+c to Exit\n");

    // In a never ending loop
    while (1){
        

        // Output our prompt and get input
        char* input = readline("lispy> ");

        // Add input to history
        add_history(input);  

        // Echo input back to user
        printf("No you're a %s", input);

        // Free retrieved input
        free(input);
    }

    return 0;
}