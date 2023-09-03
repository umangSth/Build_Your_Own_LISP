#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

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
  /* Create Some Parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    // Defining them with the following language 
    mpca_lang(MPCA_LANG_DEFAULT, 
    "                       \
    number : /-?[0-9]+(\\.[0-9]+)?/ ;   \
    operator : '+' | '-' | '*' | '/' | '%' ; \
    expr : <number> | '(' <operator> <expr>+ ')' ; \
    lispy : /^/ <operator> <expr>+ /$/ ; \
    ", Number, Operator, Expr, Lispy);

    puts("Lispy Version 0.0.0.0.1");
    puts("Prese Ctrl+c to Exit\n");

    // In a never ending loop
    while (1){
        
        // Output our prompt and get input
        char* input = readline("lispy> ");

        // Add input to history
        add_history(input);  

        // attempt to Parse the user Input
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)){
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        }else {
            // otherwise print the Error
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

         // Free retrieved input
        free(input);
    }
    // Undefine and Delete our parsers
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}