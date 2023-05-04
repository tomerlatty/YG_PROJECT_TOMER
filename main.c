#include <stdio.h>
#include "structures/vector.h"
#include "cmplr.h"

// this file contains the main function of the project

int main(){ // this is the main function, this function will start the project 

    int res = file_compiler("./test.c", "./test",0);
    if (res==COMPILER_FILE_COMPILED_OK)
    {
        printf("everything compiled fine \n");
    }
    else if (res==COMPILER_FAILED_WITH_ERRORS)
    {
        printf("Compiled failed \n");
    }
    else{
         printf("unknown response for compile file \n");
    }
    return 0;
}
