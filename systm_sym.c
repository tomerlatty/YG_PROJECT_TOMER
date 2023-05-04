#include "cmplr.h"
#include "structures/vector.h"

static void symresolver_push_symbol(struct compile_process* process, struct symbol* sym) // push a symbol to the table 
{
    vector_push(process->symbols.table, &sym);
} 


void symresolver_initialize(struct compile_process* process) // create our vector POINTER to the tables , think about that like a matri
{
    process->symbols.tables = vector_create(sizeof(struct vector*));
}

void symresolver_new_table(struct compile_process* process) // create a new vector 
{
    // save the current table
    vector_push(process->symbols.tables, &process->symbols.table);

    // overwrite the active table
    process->symbols.table = vector_create(sizeof(struct symbol*));
}

void symresolver_end_table(struct compile_process* process) // end table , its basically pop off the tables vector which essentially discards the active table  
{
    struct vector* last_table = vector_back_ptr(process->symbols.tables);
    process->symbols.table = last_table;
    vector_pop(process->symbols.tables);
}

struct symbol* symresolver_get_symbol(struct compile_process* process, const char* name) // get the symbol by its name 
{
    vector_set_peek_pointer(process->symbols.table, 0);
    struct symbol* symbol = vector_peek_ptr(process->symbols.table);
    while(symbol)
    {
        if(S_EQ(symbol->name, name))
        {
            break;
        }
        symbol = vector_peek_ptr(process->symbols.table);
    }

    return symbol;
}

struct symbol* symresolver_get_symbol_for_native_function(struct compile_process* process, const char* name) // return us the symbol only if its a native function 
{
    struct symbol* sym = symresolver_get_symbol(process, name);
    if(!sym)
    {
        return NULL;
    }
    if(sym->type != SYMBOL_TYPE_NATIVE_FUNCTION)
    {
        return NULL;
    }
    return sym;

}

struct symbol* symresolver_register_symbol(struct compile_process* process, const char* sym_name, int type, void* data) // register the symbolin the system (table) 
{
    if(symresolver_get_symbol(process, sym_name)) // if we have the symbol already 
    {
        return NULL;
    }
    // create the symbol
    struct symbol* sym = calloc(1, sizeof(struct symbol));
    sym->name = sym_name;
    sym->type = type;
    sym->data = data;
    // push the symbol 
    symresolver_push_symbol(process, sym);
    return sym;
}

struct node* symresolver_node(struct symbol* sym)  // resolve the node from the symbol 
{
    if(sym->type != SYMBOL_TYPE_NODE)
    {
        return NULL;
    }
    
    return sym->data;
}

void symresolver_build_for_variable_node(struct compile_process* process, struct node* node)
{
     if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION) // & - and operation, will work only if the both flags is equal 
    {
        // we do not register forward declaration 
        return;
    }

    symresolver_register_symbol(process, node->var.name, SYMBOL_TYPE_NODE, node);

}

void symresolver_build_for_function_node(struct compile_process* process, struct node* node)
{
    if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION) // & - and operation, will work only if the both flags is equal 
    {
        // we do not register forward declaration 
        return;
    }

    symresolver_register_symbol(process, node->func.name, SYMBOL_TYPE_NATIVE_FUNCTION, node);

}

void symresolver_build_for_structure_node(struct compile_process* process, struct node* node)
{
    if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION) // & - and operation, will work only if the both flags is equal 
    {
        // we do not register forward declaration 
        return;
    }

    symresolver_register_symbol(process, node->_struct.name, SYMBOL_TYPE_NODE, node);
}

void symresolver_build_for_union_node(struct compile_process* process, struct node* node)
{
    if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION)
    {
        // we do not register forward declaration 
        return;
    }

    symresolver_register_symbol(process, node->_union.name, SYMBOL_TYPE_NODE, node);

}

void symresolver_build_for_node(struct compile_process* process, struct node* node) // build the node symbols according there types 
{
    switch(node->type)
    {
    case NODE_TYPE_VARIABLE:
    symresolver_build_for_variable_node(process, node);
    break;

    case NODE_TYPE_FUNCTION:
    symresolver_build_for_function_node(process, node);
    break;

    case NODE_TYPE_STRUCT:
    symresolver_build_for_structure_node(process, node);
    break;

    case NODE_TYPE_UNION:
    symresolver_build_for_union_node(process, node);
    break;

    // ignore all other node types, because they cant become symbols
    }

}



