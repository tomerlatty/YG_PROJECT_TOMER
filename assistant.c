#include "cmplr.h"
#include "structures/vector.h"
#include <assert.h>
#include <stdlib.h>

// this file include functions that are neccesery to our compiling process 
// -----------------------------------------------------------------------

size_t variable_size (struct node* var_node) // return us the size of a node 
{
    assert(var_node->type == NODE_TYPE_VARIABLE);
    return datatype_size(&var_node->var.type);
}
 
size_t variable_size_for_list(struct node* var_list_node) // return us the size of a list of nodes . node->node->node  == 3 *sizeof node
{
    assert(var_list_node->type == NODE_TYPE_VARIABLE_LIST);
    size_t size = 0;
    vector_set_peek_pointer(var_list_node->var_list.list, 0);
    struct node* var_node = vector_peek_ptr(var_list_node->var_list.list);
    while(var_node)
    {
        size += variable_size(var_node);
        var_node = vector_peek_ptr(var_list_node->var_list.list);
    }

    return size;
}

struct node* variable_struct_or_union_body_node(struct node* node) // return us the node of the sturct or union
{
    if(!node_is_struct_or_union_variable(node))
    {
        return NULL;
    }

    if (node->var.type.type == DATA_TYPE_STRUCT)
    {
        return node->var.type.struct_node->_struct.body_n;
    }

    // return the union body
    if(node->var.type.type == DATA_TYPE_UNION)
    {
        return node->var.type.union_node->_union.body_n;
    }
    return NULL;
}

// -----------------------------------------------------------------------
// the padding part :
// PUSH and POP instructions use 4 bytes at a time, memory access is performed on boundaries of a 4 bytes , we need to handle this 
// how to solve the problem?reorder instructions or  align our data so it fits into a 4 byte boundary, pad NULL at the end of an unaligned variable forcing it to be aligned 

int padding(int val, int to) // will pad the value add to whatever this is, we need ((val % to) == 0), so this function return what we need to add
{
    if(to <= 0)
    {
        return 0;
    }

    if((val % to) == 0)
    {
        return 0;
    }

    return to- (val%to) % to;
}

int align_value(int val, int to) // this function will do the align uses adding the return value of padding function 
{
    if (val % to)
    {
        val += padding(val, to);
    }

    return val;
}

int align_value_treat_positive(int val, int to) // it will treat the val and to as positive together or negative together(positive)
{
    assert(to >= 0);
    if(val < 0)
    {
        to = -to;
    }
    return align_value(val,to);
}

int compute_sum_padding(struct vector* vec) // we sum all of the padding in the vector 
{
    // this function ignore ant nodes that are not node type variable because the type of the things that going to passed into
    // compute some padding literally going to be the statement vectors for bodies 
    int padding = 0;
    int last_type = -1;
    bool mixed_types = false;
    vector_set_peek_pointer(vec, 0);
    struct node* cur_node = vector_peek_ptr(vec);
    struct node* last_node = NULL;
    while(cur_node)
    {
        if(cur_node->type != NODE_TYPE_VARIABLE) // ignore the nodes that not variable 
        {
            cur_node = vector_peek_ptr(vec);
            continue;
        }

        padding += cur_node->var.padding;
        last_type = cur_node->var.type.type;
        last_node = cur_node;
        cur_node = vector_peek_ptr(vec);
    }

    return padding;
}

// -----------------------------------------------------------------------
// the datatype  part :
bool datatype_is_struct_or_union(struct datatype* dtype) // return us if the dtype is a struct or a union 
{
    return dtype->type == DATA_TYPE_STRUCT || dtype->type == DATA_TYPE_UNION;
}

bool datatype_is_struct_or_union_for_name(const char* name) // return us if the name is struct or union 
{
    return S_EQ(name, "union") || S_EQ(name, "struct");
}

size_t datatype_element_size(struct datatype* dtype) // return us the elemtnt(datatype) size 
{
    if (dtype->flags & DATATYPE_FLAG_IS_POINTER) // if pointer its a DWORD 
    {
        return DATA_SIZE_DWORD;
    }

    return dtype->size;
}

size_t datatype_size_for_array_access(struct datatype* dtype) // return us the size of datatype for array 
{
    if(datatype_is_struct_or_union(dtype) && dtype->flags & DATATYPE_FLAG_IS_POINTER && 
        dtype->pointer_depth == 1) //struct array
    {
        // struct abc* abc; abc[0]
        return dtype->size;
    }

    return dtype->size;
}

size_t datatype_size_no_ptr(struct datatype* dtype) // return us the size of datatype 
{
    if (dtype->flags & DATATYPE_FLAG_IS_ARRAY)// if its a array 
    {
        return dtype->array.size;
    }

    return dtype->size;
}

size_t datatype_size(struct datatype* dtype) // return the SIZE of the datatype 
{
    if(dtype->flags & DATATYPE_FLAG_IS_POINTER && dtype->pointer_depth >0) // if its a pointer 
    {
        return DATA_SIZE_DWORD;
    }

    if(dtype->flags & DATATYPE_FLAG_IS_ARRAY) // if its a array 
    {
        return dtype->array.size;
    }
    
    return dtype->size; // single elements 
}

bool datatype_is_primitive(struct datatype* dtype) // if its not struct or union its a primitive 
{
    return !datatype_is_struct_or_union(dtype);
}

// -----------------------------------------------------------------------
// the priorities:

// Format : {operator1,operator2,operator3,NULL}

struct expressionable_op_precedence_group op_precedence[TOTAL_OPERATOR_GROUPS] = {
    {.operators={"++","--","()","[]","(","[",".","->",NULL}, .associtivity=ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"*","/","%",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"+","-",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"<<",">>",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"<","<=",">",">=",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"==","!=",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"&",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"^",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"|",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"&&",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"||",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators={"?",":",NULL}, .associtivity =  ASSOCIATIVITY_RIGHT_TO_LEFT},
    {.operators={"=","+=","-=","*=","/=","%=","<<=",">>=","&=","^=","|=",NULL}, .associtivity=ASSOCIATIVITY_RIGHT_TO_LEFT},
    {.operators={",",NULL}, .associtivity = ASSOCIATIVITY_LEFT_TO_RIGHT}
};

// -----------------------------------------------------------------------
// the array part :

struct array_brackets* array_brackets_new() // create a new structure of array brackets 
{
    struct array_brackets* brackets = calloc(1, sizeof(struct array_brackets));
    brackets->n_brackets = vector_create(sizeof(struct node*));
    return brackets;
}

void array_brackets_free(struct array_brackets* brackets) // free the memory of the brackets 
{
    free(brackets);
}

void array_brackets_add(struct array_brackets* brackets, struct node* bracket_node) // add a array bracket node to the structure 
{
    assert(bracket_node->type == NODE_TYPE_BRACKET);
    vector_push(brackets->n_brackets, &bracket_node);
}

struct vector* array_brackets_node_vector(struct array_brackets* brackets) // return the vector of the brackets 
{
    return brackets->n_brackets;
}

size_t array_brackets_calculate_size_from_index(struct datatype* dtype, struct array_brackets* brackets,int index) // calculate the array bracket size from a given index 
{
    struct vector* array_vec = array_brackets_node_vector(brackets); // getting the vector of the array brackets 
    size_t size = dtype->size; // array of integer,char.. so the size of them sizeof(int)..
    if(index >= vector_count(array_vec))// char* abc;
    {
        // char* abc;
        // return abc[0]; return abc[1];
        return size;
    }

    vector_set_peek_pointer(array_vec, index);
    struct node* array_bracket_node = vector_peek_ptr(array_vec);
    if(!array_bracket_node)
    {
        return 0;
    }

    while(array_bracket_node)
    {
        assert(array_bracket_node->bracket.inner->type == NODE_TYPE_NUMBER);
        int number = array_bracket_node->bracket.inner->llnum; // [5] so 5
        size *= number;
        array_bracket_node = vector_peek_ptr(array_vec);
    }

    return size;
   
}

size_t array_brackets_calculate_size(struct datatype* dtype, struct array_brackets* brackets) // calculate the array bracket size from 0 - the start of the brackets  
{
    return array_brackets_calculate_size_from_index(dtype, brackets,0);
}

int array_total_indexes(struct datatype* dtype) // how much brackets we have i.e a[2][4][6] so the index will be 3 - 3 [][][]
{
    assert(dtype->flags & DATATYPE_FLAG_IS_ARRAY);
    struct array_brackets* brackets = dtype->array.brackets;
    return vector_count(brackets->n_brackets);
}

// -----------------------------------------------------------------------
// the fixup part :
// in c language, we can define structures anywhere in a source file, and they can be forward declared
// so , how do we know that a structure being referenced actually exists if we havent parsed it yet?
// slution- a fixup system 
// when we encounter a stucture variable decleration and a structure doesnt exist, we register a fixup which is basically a reminder to our compiler 

struct fixup_system* fixup_sys_new() // create a new fixup_system 
{
    struct fixup_system* system = calloc(1, sizeof(struct fixup_system));
    system->fixups = vector_create(sizeof(struct fixup));
    return system;
}

struct fixup_config* fixup_config(struct fixup* fixup) // return us the fixup configuration of the fixup 
{
    return &fixup->config;
}

void fixup_free(struct fixup* fixup)
{
    fixup->config.end(fixup); // gives the implementor of this fixup to be able to free its memory data
    free(fixup);
}

void fixup_start_iteration(struct fixup_system* system) // set the peek pointer of the fixups to zero 
{
    vector_set_peek_pointer(system->fixups,0);
}

struct fixup* fixup_next(struct fixup_system* system) // get next to the next element
{
    return vector_peek_ptr(system->fixups);
}

void fixup_sys_fixups_free(struct fixup_system* system) // clean oll the fixups in the fixup _system 
{
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while(fixup)
    {
        fixup_free(fixup);
        fixup = fixup_next(system);
    }
}

void fixup_sys_free(struct fixup_system* system) // free the actual fixyp_system
{
    fixup_sys_fixups_free(system);
    vector_free(system->fixups);
    free(system);
}

int fixup_sys_unresolved_fixups_conut(struct fixup_system* system) // return as how much fixups we not handle yet  
{
    size_t c = 0;
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while(fixup)
    {
        if(fixup->flags & FIXUP_FLAG_RESOLVED)
        {
            fixup = fixup_next(system);
            continue;
        }
        c++;
        fixup = fixup_next(system);
        
    }
    return c ;
}

struct fixup* fixup_register(struct fixup_system* system, struct fixup_config* config) // create a new fixup structure
{
    struct fixup* fixup = calloc(1, sizeof(struct fixup));
    memcpy(&fixup->config, config, sizeof(struct fixup_config));
    fixup->system = system;
    vector_push(system->fixups, fixup);
    return fixup;
}

bool fixup_resolve(struct fixup* fixup) // resolve the fixup
{
    if(fixup_config(fixup)->fix(fixup))
    {
        fixup->flags |= FIXUP_FLAG_RESOLVED; // resolve the fixup flag 
        return true;
    }

    return false; // failed to fix it , because the fix function returned false 
}

void* fixup_private(struct fixup* fixup) // return the private data of the fixup 
{
    return fixup_config(fixup)->private;
}

bool fixups_resolve(struct fixup_system* system) // resolve all the fixups in the fixup_system 
{
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while(fixup)
    {
        if(fixup->flags & FIXUP_FLAG_RESOLVED)
        {
            continue;
        }
        fixup_resolve(fixup);
        fixup = fixup_next(system);

    }

    return fixup_sys_unresolved_fixups_conut(system) == 0; // if its equal to zero then that means we seccessfully resolved all fix ups 
}

// this part contains all the functions that we need for the lex_process

struct lex_process* lex_process_create(struct compile_process* compiler,struct lex_process_functions* functions,void* private){ // function to create a new struct of lex_process

    struct lex_process* process = calloc(1,sizeof(struct lex_process));
    process->function = functions;
    process->token_vec = vector_create(sizeof(struct token)); // creating a new vec
    process->compiler = compiler;
    process->private = private;
    process->pos.line = 1;
    process->pos.col = 1;
    return process;
}

void lex_process_free(struct lex_process* process) // free all the memory of the lex_process
{
    vector_free(process->token_vec);
    free(process);
}

void* lex_process_private(struct lex_process* process) // return the private data of the lex_process 
{
    return process->private;
}

struct vector* lex_process_tokens(struct lex_process* process){ // return the vector of the lex_process
    return process->token_vec;
}
