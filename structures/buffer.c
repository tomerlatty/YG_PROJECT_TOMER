#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// this file include all the functions that related to the buffer , when the buffer is used by us as a data structure

struct buffer* buffer_create() // create a new buffer 
{
    struct buffer* buf = calloc(sizeof(struct buffer), 1);
    buf->data = calloc(BUFFER_REALLOC_AMOUNT, 1);
    buf->len = 0;
    buf->msize = BUFFER_REALLOC_AMOUNT;
    return buf;
}

void buffer_extend(struct buffer* buffer, size_t size) // realloc a buffer 
{
    buffer->data = realloc(buffer->data, buffer->msize+size);
    buffer->msize+=size;
}

void buffer_need(struct buffer* buffer, size_t size) // if we need to put in a string that bigger than the space the buffer have 
{
    if (buffer->msize <= (buffer->len+size))
    {
        size += BUFFER_REALLOC_AMOUNT;
        buffer_extend(buffer, size);
    }
}


void buffer_printf(struct buffer* buffer, const char* fmt, ...) // write into the buffer , va_lust because we dont know the size of the data 
{
    va_list args; // variable that holds the information about the variable-length argument functions.
    va_start(args, fmt); // macro for holding info 
    int index = buffer->len;
    // Temporary, this is a limitation we are guessing the size is no more than 2048
    int len = 2048;
    buffer_extend(buffer, len); // realloc
    int actual_len = vsnprintf(&buffer->data[index], len, fmt, args);
    buffer->len += actual_len;
    va_end(args);
}

void buffer_printf_no_terminator(struct buffer* buffer, const char* fmt, ...) // like buffer_printf but this function operation not contain the NULL TERMINATOR set
{
    va_list args;
    va_start(args, fmt);
    int index = buffer->len;
    // Temporary, this is a limitation we are guessing the size is no more than 2048
    int len = 2048;
    buffer_extend(buffer, len); // realloc
    int actual_len = vsnprintf(&buffer->data[index], len, fmt, args);
    buffer->len += actual_len-1;
    va_end(args);
}

void buffer_write(struct buffer* buffer, char c) // write one character to the buffer 
{
    buffer_need(buffer, sizeof(char));

    buffer->data[buffer->len] = c;
    buffer->len++;
}

void* buffer_ptr(struct buffer* buffer) // return all the data of the buffer 
{
    return buffer->data;
}

char buffer_read(struct buffer* buffer) // to look into the buffer with the read-index increment (after that we will get the next char)
{
    if (buffer->rindex >= buffer->len)
    {
        return -1;
    }
    char c = buffer->data[buffer->rindex];
    buffer->rindex++;
    return c;
}

char buffer_peek(struct buffer* buffer) // to look into the buffer - this function will return the char in the place that the rindex set
{
    if (buffer->rindex >= buffer->len)
    {
        return -1;
    }
    char c = buffer->data[buffer->rindex];
    return c;
}

void buffer_free(struct buffer* buffer) // free the memory of the buffer 
{
    free(buffer->data);
    free(buffer);
}

