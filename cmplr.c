#include "cmplr.h"
#include <stdarg.h>
#include <stdlib.h>

// this file is the core of the project and constitutes its foundations

struct lex_process_functions compiler_lex_functions = {
    .next_char = compile_process_next_char,
    .peek_char = compile_process_peek_char,
    .push_char = compile_process_push_char
};

void system_error(struct compile_process* compiler, const char* msg, ...) // providing a error message to the user during the compilning if something go wrong
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, " on line %i, col %i in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
    exit(-1);
}

void system_warning(struct compile_process* compiler, const char* msg, ...)// providing a warning message to the user during the compilning
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, " on line %i, col %i in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
}

int file_compiler(const char* filename, const char* out_filename, int flags) // the main function of the compiler , start all the process of the compiling
{
    struct compile_process* process = compile_process_create(filename, out_filename, flags); // we "start" a new procoess , build a new struct
    if (!process) // if failed
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    // preform lexical analysis
    
    struct lex_process* lex_process = lex_process_create(process,&compiler_lex_functions,NULL); // creating a new lex_process, build a new struct 

    if(!lex_process)// if failed
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    if(lex(lex_process)!= LEXICAL_ANALYSIS_ALL_OK) // if failed
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    process ->token_vec = lex_process->token_vec ; // the vector that the parser work on 

    // preform parsing

    if(parse(process) != PARSE_ALL_OK)// if failed
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    
    return COMPILER_FILE_COMPILED_OK; // if all the steps gone ok 
    
}