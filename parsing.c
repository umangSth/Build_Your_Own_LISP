#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include <math.h>

// If we are compiling on Windows compile these functions
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// Declare New lval struct
typedef struct lval {
    int type;
    long num;
    // Error and Symbol types have some string data
    char* err;
    char* sym;

    // Count and Pointer to a list of "lval*"
    int count;
    struct lval** cell;
} lval;

// create Enumeration of possible lval Types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

// Create Enumeration of Possible Error Types
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};


// Construct a pointer to a new Number lval *
lval* lval_num(long x){
    lval* v =  malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

//Construct a pointer to a new Error lval
lval* lval_err(char* m){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m)+1);
    strcpy(v-> err, m);
    return v;
}

// Construct a pointer to a new Symbol lval
lval* lval_sym(char* s){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s)+1);
    strcpy(v->sym, s);
    return v;
}

// a pointer to a new empty Sexpr lval
lval* lval_sexpr(void){
    lval* v = malloc(sizeof(lval));
    v-> type = LVAL_SEXPR;
    v-> count = 0;
    v-> cell = NULL;
    return v;
}

// print an "lval"
void lval_print(lval v){
    switch(v.type){
        // In the case the type is a name print it 
        // then 'break' out of the switch
        case LVAL_NUM: printf("%li", v.num); break;

        // In the case the type is an error
        case LVAL_ERR:
            // check what type of error it is and print it
            if (v.err == LERR_DIV_ZERO){
                printf("Error: Division By Zero!");
            }
            if (v.err == LERR_BAD_OP){
                printf("Error: Invalid Operator!");
            }
            if (v.err == LERR_BAD_NUM){
                printf("Error: Invalid Number!");
            }
        break;
    }
}

void lval_del(lval* v){
    switch (v-> type){
        // Do nothing special for number type
        case LVAL_NUM: break;

        // for Err or Sym free the string data 
        case LVAL_ERR: free(v-> err); break;
        case LVAL_SYM: free(v-> sym); break;

        //If Sexpr then delete all elements inside 
        case LVAL_SEXPR:
            for(int i=0; i< v->count; i++){
                lval_del(v-> cell[i]);
            }
            // Also free the memory allocated to contain the pointers
            free(v->cell);
            break;
    }
    // free the memory allocated for the "Lval" struct itself
    free(v);
}



// Print an "lval" followed by a newline
void lval_println(lval v) {lval_print(v); putchar('\n');}

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
lval eval_op(lval x, char* op, lval y){

    // If either value is an error return it
    if(x.type == LVAL_ERR) {return x;}
    if(y.type == LVAL_ERR) {return y;}

    // otherwise do maths on the number values
    if(strcmp(op, "+") == 0) {return lval_num(x.num + y.num);}
    if(strcmp(op, "-")==0) {return lval_num(x.num - y.num);}
    if(strcmp(op, "*")==0) {return lval_num(x.num * y.num);}
    if(strcmp(op, "/")==0) {
        // if second operand is zero return error
        return y.num == 0 ? lval_err(LERR_DIV_ZERO)
        :lval_num(x.num/y.num);        
        }
    if(strcmp(op, "%")==0) {return lval_num(x.num % y.num);}
    if(strcmp(op, "^")==0) {return lval_num(pow(x.num, y.num));}
    if(strcmp(op, "min")==0){return lval_num(x.num < y.num ? x.num : y.num);}
    if(strcmp(op, "max") == 0){return lval_num(x.num > y.num ? x.num : y.num);}
    return lval_err(LERR_BAD_OP);
}



lval eval(mpc_ast_t* t){
    // if tagged as number return it directly. 
    if(strstr(t->tag, "number")){
        // check if there is some error in conversion
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x):lval_err(LERR_BAD_NUM);
    }
    // the operator is always second child.
    char* op= t->children[1] -> contents;

    // we store the third child in 'x'
    lval x = eval(t->children[2]);
    
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
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    // Defining them with the following language 
    mpca_lang(MPCA_LANG_DEFAULT, 
    "                       \
    number : /-?[0-9]+(\\.[0-9]+)?/ ;   \
    symbol : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ; \
    sexpr  : '(' <expr>* ')'  ;   \
    expr : <number> | <symbol> | <sexpr> ; \
    lispy : /^/ <expr>* /$/ ; \
    ", Number, Symbol, Expr, Lispy);

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
            lval result = eval(r.output);
            lval_println(result);
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
    mpc_cleanup(4, Number, Symbol, Sexpr, Expr, Lispy);
    return 0;
}