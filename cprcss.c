#include <stdio.h>
#include <stdlib.h>
#include "cmplr.h"
#include "structures/vector.h"

// this file stands for the compiler process, and the functions that found here are related to the compiler process operations

struct compile_process* compile_process_create(const char* filename,const char* filename_out,int flags) // this main function, create a new compile_process structure and actually starting a new process 
{
    FILE* file = fopen(filename,"r");
    if(!file){
        return NULL;
    }

    FILE* out_file = NULL;
    if(filename_out) // if the user not provided a file name out, 
            //then we will not open anything for writing because sometimes we might want to compile something just to show on the terminal
    {
        out_file =fopen(filename_out, "w");
        if(!out_file)
        {
            return NULL;
        }
    }
    
    struct  compile_process* process = calloc(1, sizeof(struct compile_process));
    process->node_vec = vector_create(sizeof(struct node*));
    process->node_tree_vec = vector_create(sizeof(struct node*));

     // we build the process fields
    process->flags = flags;
    process->cfile.fp =file;
    process->ofile = out_file;

    symresolver_initialize(process);
    symresolver_new_table(process);

    return process;

}

char compile_process_next_char(struct lex_process* lex_process){ // a function to get to the next char in the file and return it 

    struct compile_process* compiler = lex_process->compiler;
    compiler->pos.col +=1;
    char c = getc(compiler->cfile.fp); // get one char from the file 
    if(c== '\n'){ // if we go to new line , we need to set the changes 
        compiler->pos.line +=1;
        compiler->pos.col = 1;
    }

    return c;
}

char compile_process_peek_char(struct lex_process* lex_process){ // a function to peek to the next char in the file and return it 

    struct compile_process* compiler = lex_process ->compiler;
    char c = getc(compiler->cfile.fp); // get the character from the file
    ungetc(c, compiler->cfile.fp); // push the character back to the file 
    return c;

}

void compile_process_push_char(struct lex_process* lex_process,char c ){ // a function to push a character to the file 
    struct compile_process* compiler = lex_process->compiler;
    ungetc(c,compiler->cfile.fp); // push the character that provided
}