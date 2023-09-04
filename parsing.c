#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include <math.h>

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

int number_of_nodes(mpc_ast_t* t){
    if(t->children_num == 0) {return 1;}
    if(t->children_num >= 1){
        int total = 1;
        for(int i=0; i<t->children_num; i++){
            total = total + number_of_nodes(t->children[i]);
        }
        return total;
    }
    return 0;
}

// use operator string to see which operation to perform
long eval_op(long x, char* op, long y){
    if(strcmp(op, "+") == 0) {return x + y;}
    if(strcmp(op, "-")==0) {return x - y;}
    if(strcmp(op, "*")==0) {return x * y;}
    if(strcmp(op, "/")==0) {return x / y;}
    if(strcmp(op, "%")==0) {return x % y;}
    if(strcmp(op, "^")==0) {return pow(x, y);}
    if(strcmp(op, "min")==0){return x < y ? x : y;}
    if(strcmp(op, "max") == 0){return x > y ? x : y;}
    return 0;
}



long eval(mpc_ast_t* t){
    // if tagged as number return it directly. 
    if(strstr(t->tag, "number")){
        return atoi(t->contents);
    }
    // the operator is always second child.
    char* op= t->children[1] -> contents;

    // we store the third child in 'x'
    long x = eval(t->children[2]);
    
    // Iterate the remaining children and combining
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")){
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
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
    operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ; \
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
            // // load AST from output
            // mpc_ast_t* a = r.output;
            // printf("Tag: %s\n", a->tag);
            // printf("Content: %s\n", a->contents);
            // printf("Number of children: %i\n", a->children_num);

            // // get First Child
            // mpc_ast_t* c0 = a->children[0];
            // printf("First Child Tag: %s\n",c0->tag);
            // printf("First Child Contents: %s\n", c0->contents);
            // printf("First Child Number of Children: %i\n", c0->children_num);
            // mpc_ast_print(a);
            // mpc_ast_delete(a);

            long result = eval(r.output);
            printf("%li\n", result);
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