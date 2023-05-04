#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// this file contains 

// We want at least 20 vector element spaces in reserve before having
// to reallocate memory again
#define VECTOR_ELEMENT_INCREMENT 20

enum // flags for the state of the vector 
{
    VECTOR_FLAG_PEEK_DECREMENT = 0b00000001
};

struct vector
{
    // the data that the vector store
    void* data;

    // The pointer index is the index that will be read next upon calling "vector_peek".
    // This index will then be incremented
    int pindex;

    // the last index (thee read-index ) on the vector , for example 15/20(20 is the max index) element we used so the rindex is 15
    int rindex;

    int mindex; // the max index of the vector 

    int count; // count how many stuctures store in the vector

    int flags; // the flags of the vector 
    size_t esize; // the size of the data structure


    // Vector of struct vector, holds saves of this vector. You can save the internal state
    // at all times with vector_save
    // Data is not restored and is permenant, save does not respect data, only pointers
    // and variables are saved. Useful to temporarily push the vector state
    // and restore it later.
    struct vector* saves;
};

// the prototypes :

struct vector* vector_create(size_t esize);
void vector_free(struct vector* vector);
void* vector_at(struct vector* vector, int index);
void* vector_peek_ptr_at(struct vector* vector, int index);
void* vector_peek_no_increment(struct vector* vector);
void* vector_peek(struct vector* vector);
void *vector_peek_at(struct vector *vector, int index);
void vector_set_flag(struct vector* vector, int flag);
void vector_unset_flag(struct vector* vector, int flag);
void vector_pop_last_peek(struct vector* vector);
void* vector_peek_ptr(struct vector* vector);
void vector_set_peek_pointer(struct vector* vector, int index);
void vector_set_peek_pointer_end(struct vector* vector);
void vector_push(struct vector* vector, void* elem);
void vector_push_at(struct vector *vector, int index, void *ptr);
void vector_pop(struct vector* vector);
void vector_peek_pop(struct vector* vector);
void* vector_back(struct vector* vector);
void *vector_back_or_null(struct vector *vector);
void* vector_back_ptr(struct vector* vector);
void* vector_back_ptr_or_null(struct vector* vector);
const char* vector_string(struct vector* vec);
bool vector_empty(struct vector* vector);
void vector_clear(struct vector* vector);
int vector_count(struct vector* vector);
int vector_fread(struct vector* vector, int amount, FILE* fp);
void* vector_data_ptr(struct vector* vector);
int vector_insert(struct vector *vector_dst, struct vector *vector_src, int dst_index);
int vector_pop_at_data_address(struct vector* vector, void* address);
int vector_pop_value(struct vector* vector, void* val);
void vector_pop_at(struct vector *vector, int index);
void vector_peek_back(struct vector* vector);
int vector_current_index(struct vector* vector);
void vector_save(struct vector* vector);
void vector_restore(struct vector* vector);
void vector_save_purge(struct vector* vector);
size_t vector_element_size(struct vector* vector);
struct vector* vector_clone(struct vector* vector);

#endif