#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>

#define BUFFER_REALLOC_AMOUNT 2000

// this file contains all the definitions that related to the buffer 

struct buffer
{
    char* data; // the data that the buffer store
    int rindex;   // the read-index
    int len; // the current length of the buffer 
    int msize; // the maximum size of this buffer(that it can store)
};

// the prototypes :

struct buffer* buffer_create();

char buffer_read(struct buffer* buffer);
char buffer_peek(struct buffer* buffer);

void buffer_extend(struct buffer* buffer, size_t size);
void buffer_printf(struct buffer* buffer, const char* fmt, ...);
void buffer_printf_no_terminator(struct buffer* buffer, const char* fmt, ...);
void buffer_write(struct buffer* buffer, char c);
void* buffer_ptr(struct buffer* buffer);
void buffer_free(struct buffer* buffer);


#endif