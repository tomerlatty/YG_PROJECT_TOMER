#include "cmplr.h"
#include "structures/vector.h"
#include <memory.h>
#include <stdlib.h>
#include <assert.h>



struct scope* scope_alloc() // create a new scope structure
{
    struct scope* scope = calloc(1, sizeof(struct scope));
    scope->entities = vector_create(sizeof(void*));
    vector_set_peek_pointer_end(scope->entities);
    vector_set_flag(scope->entities, VECTOR_FLAG_PEEK_DECREMENT);
    return scope;
}

void scope_dealloc(struct scope* scope) //free the memory of the scope 
{
   struct vector* vec = scope->entities;
   vector_free(vec);
   free(scope);
}

struct scope* scope_create_root(struct compile_process* process) // create a root scope 
{
    // there shouldnt be a current scope or a root scope at this point 
    assert(!process->scope.root);
    assert(!process->scope.current);

    struct scope* root_scope = scope_alloc();
    process->scope.root = root_scope;
    process->scope.current = root_scope;
    return root_scope;
}

void scope_free_root(struct compile_process* process) // free a scope struct 
{
    scope_dealloc(process->scope.root);
    process->scope.root = NULL;
    process->scope.current = NULL;
}

struct scope* scope_new(struct compile_process* process, int flags) // return us a new scope structure 
{
    assert(process->scope.root);
    assert(process->scope.current);

    struct scope* new_scope = scope_alloc();
    new_scope->flags = flags;
    new_scope->parent = process->scope.current; // the parent of the new scope will be the current scope we use 
    process->scope.current = new_scope;
    return new_scope;
}

void scope_iteration_start(struct scope* scope) // reset iterations , allows us to iterate from the start of our scope again 
{
    vector_set_peek_pointer(scope->entities, 0);
    if(scope->entities->flags & VECTOR_FLAG_PEEK_DECREMENT)
    {
        vector_set_peek_pointer_end(scope->entities); // start from the end 
    }
}



void* scope_iterate_back(struct scope* scope) // return us the last elemnt that was push
{
    if(vector_count(scope->entities) == 0)
        return NULL;

    return vector_peek_ptr(scope->entities);

}

void* scope_last_entity_at_scope(struct scope* scope) // return us the last entity 
{
    if(vector_count(scope->entities) == 0)
        return NULL;

    return vector_back_ptr(scope->entities); // return us the one before - the last elemnt that was push
}

void* scope_last_entity_from_scope_stop_at(struct scope* scope, struct scope* stop_scope) // recursive function to look for a entity from the scope to the stop_scope
{
    if(scope == stop_scope)
    {
        return NULL;
    }

    void* last = scope_last_entity_at_scope(scope);
    if(last)
    {
        return last;
    }
    struct scope* parent = scope->parent;
    if(parent)
    {
        return scope_last_entity_from_scope_stop_at(parent, stop_scope);

    }

    return NULL;
}

void* scope_last_entity_stop_at(struct compile_process* process, struct scope* stop_scope) // do the same like scope_last_entity_from_scope_stop_at what we send different parameters
{
    return scope_last_entity_from_scope_stop_at(process->scope.current, stop_scope);
}

void* scope_last_entity(struct compile_process* process)// do the same like scope_last_entity_from_scope_stop_at what we send different parameters
{
    // null to rich all the scope that exist 
    return scope_last_entity_stop_at(process, NULL);
}

void scope_push(struct compile_process* process, void* ptr, size_t elem_size) // push element to our scope 
{
    vector_push(process->scope.current->entities, &ptr);
    process->scope.current->size += elem_size;
}

void scope_finish(struct compile_process* process) // finishing a scope 
{
    struct scope* new_current_scope = process->scope.current->parent;
    scope_dealloc(process->scope.current);
    process->scope.current = new_current_scope;
    if(process->scope.root && !process->scope.current)
    {
        process->scope.root = NULL;

    }
}

struct scope* scope_current(struct compile_process* process) // return us the current scope 
{
    return process->scope.current;
}