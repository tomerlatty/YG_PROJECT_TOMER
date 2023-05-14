#ifndef TOMER_COMPILER_H
#define TOMER_COMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// this file will contain all the prototypes, structures and definitions we need in our project 

// macro definition to compare two strings , a lot more clinner than strcmp
#define S_EQ(str, str2) \
        (str && str2 && (strcmp(str,str2) == 0))


struct pos // struct the represent position on the file 
{ 
    int line;
    int col;
    const char* filename;
};

// cases when something(token) its a number and we need to make it a number token 
#define NUMERIC_CASE \
    case '0':       \
    case '1':       \
    case '2':       \
    case '3':       \
    case '4':       \
    case '5':       \
    case '6':       \
    case '7':       \
    case '8':       \
    case '9'  

// all the operators 
#define OPERATOR_CASE_EXCLUDING_DIVISION \
    case '+':                           \
    case '-':                           \
    case '*':                           \
    case '>':                           \
    case '<':                           \
    case '^':                           \
    case '%':                           \
    case '!':                           \
    case '=':                           \
    case '~':                           \
    case '|':                           \
    case '&':                           \
    case '(':                           \
    case '[':                           \
    case ',':                           \
    case '.':                           \
    case '?'

// all the symbols
#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'

enum{
    LEXICAL_ANALYSIS_ALL_OK,
    LEXICAL_ANALYSIS_INPUT_ERROR
}; // enums for the state of the lexical_analysis 

enum{
    TOKEN_TYPE_IDENTIFIER, // names like function name, var name..
    TOKEN_TYPE_KEYWORD, // keyword like while, goto, if..
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE
};

enum // posible types of number 
{
    NUMBER_TYPE_NORMAL,
    NUMBER_TYPE_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLE
};

struct token{ // the struct of the token 

    int type; //type of the token
    int flags; // the flags of the token
    struct pos pos; // the position of the token
    union{
        char cval;
        const char* sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void* any;
    }; // the value of the token

    struct token_number
    {
        int type;
    }  num; // if the token its a number , what the type of the number 

    //True if their is whitespace between the token and the next token
    // i.e * a for operator token * would mean whitespace would be set for token "a"
    bool whitespace;

    // (Hello world) , every token inside the brackets will point to the start of this expression 
    const char* between_brackets;
};

struct lex_process; // prototype

// typedef of functions , making a bunch of functions pointers
typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process* process); 
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process* process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process* process,char c);

struct lex_process_functions{ // struct that contains functions pointers

    LEX_PROCESS_NEXT_CHAR next_char;
    LEX_PROCESS_PEEK_CHAR peek_char;
    LEX_PROCESS_PUSH_CHAR push_char;
};

struct lex_process{ // the struct of the lex_process 
    struct pos pos; // position
    struct vector* token_vec; // a vector of tokens
    struct compile_process* compiler; // pointer to the compile process its a part of 

    /*
        ((50))
    */
    int current_expression_count; // how many brackets are there at the moment 
    struct buffer* parentheses_buffer; // the buffer that hold the data inside the parentheses ()
    struct lex_process_functions* function; // the functions od the lexa

    // This will be private data that the lexer does not understand but the person using the lexer does understand
    void* private; 

};

enum // enum for the compiler process in compiler.c
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};

struct scope // // the struct of the scope 
{
    int flags;

    // void*, we basically can push anything to our scope
    struct vector* entities;

    // the total number of bytes this scope uses. aligned to 16 bytes

    size_t size;

    // NULL if no parent, pointer to our parent scope 
    struct scope* parent;
};

enum{ // types of symbols 
    SYMBOL_TYPE_NODE,
    SYMBOL_TYPE_NATIVE_FUNCTION, // macro function 
     SYMBOL_TYPE_UNKNOWN // case we have not idea what is this symbol 
};

struct symbol // the struct of the symbol 
// what is a symbols ?
// symbols are functions, structures, global variables We have identified during the compile process  
// we need that the compiler recognize them so we cant be able to declare two functions with the same name or something like that 
// we also want to know if we call a function , that this function is exist 
{
    const char* name; // the name of the symbol . its the function name, the structure name . if we have struct dog so dog its the name 
    int type; // what is the type of the symbol 
    void* data; // the data of the symbol 
};

struct compile_process // the structure of the process of the compiler 
{
    // the flags in regards to how this file should be compiled
    int flags;

    struct pos pos;
    struct compile_process_input_file{ // the input file 
        FILE* fp;
        const char* abs_path;
    }cfile;

    // a vector of tokens from lexical analysis.
    struct vector* token_vec;

    struct vector* node_vec; // used to push and pop nodes while we are doing the parsing process 
    struct vector* node_tree_vec; // the actual root of the tree 

    FILE* ofile; // the output file 

    struct
    {
        struct scope* root; // the main scope 
         struct scope* current; // the scope we are currently on 
    } scope; // the scope system 

    struct 
    {
        // current active symbol table; struct symbol* 
        struct vector* table;

        // struct vector* multiple symbol tables stored in here.. 
        struct vector* tables;
    } symbols;// the symbols system (symresolver)
};

enum // enums for the parser 
{
    PARSE_ALL_OK,
    PARSE_GENERAL_ERROR 
};

enum{ // the types of the node 
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_EXPRESSION_PARENTHESES,
    NODE_TYPE_NUMBER,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_STRING,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_VARIABLE_LIST,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_BODY,
    NODE_TYPE_STATEMENT_RETURN,
    NODE_TYPE_STATEMENT_IF,
    NODE_TYPE_STATEMENT_ELSE,
    NODE_TYPE_STATEMENT_WHILE,
    NODE_TYPE_STATEMENT_DO_WHILE,
    NODE_TYPE_STATEMENT_FOR,
    NODE_TYPE_STATEMENT_BREAK,
    NODE_TYPE_STATEMENT_CONTINUE,
    NODE_TYPE_STATEMENT_SWITCH,
    NODE_TYPE_STATEMENT_CASE,
    NODE_TYPE_STATEMENT_DEFAULT,
    NODE_TYPE_STATEMENT_GOTO,

    NODE_TYPE_UNARY,
    NODE_TYPE_TENARY,
    NODE_TYPE_LABEL,
    NODE_TYPE_STRUCT,
    NODE_TYPE_UNION,
    NODE_TYPE_BRACKET,
    NODE_TYPE_CAST,
    NODE_TYPE_BLANK

};

enum{ // enums for node flag's
    NODE_FLAG_INSIDE_EXPRESSION = 0b00000001,
    NODE_FLAG_IS_FORWARD_DECLARATION = 0b00000010,
    NODE_FLAG_HAS_VARIABLE_COMBINED = 0b00000100,
};

struct array_brackets // the struct of the array bracket
{
    // vector of struct node*, will contain each part of the array 
    struct vector* n_brackets; 
};

struct node; // prototype 
struct datatype // the struct of the datatype : int, char..
{
    int flags;
    // i.e type of long, int, float ect..
    int type;

    // i.e long int. int being the secondary
    struct datatype* secondary;

    //long
    const char* type_str;
    // the sizeof the datatype. if its a pointer , then the size should be 4 bytes 
    size_t size;

    int pointer_depth; // assentialy how many * there is before the pointer , if its a pointer i.a **s so pointer_depth = 2

    union // if its a node or a union 
    {
        struct node* struct_node;
        struct node* union_node;
    };

    struct array
    {
        struct array_brackets* brackets; // pointer to the brackets 

        // the total array size : equation = DATATYPE_SIZE * EACH_INDEX 

        size_t size; // the size of a datatype array 
    } array; 
    
};

struct parsed_switch_case 
{
    // index of the parsed case
    int index;
};


struct node // the struct of the node  
{
    int type; // the node types
    int flags; // the flags of the node 
    struct pos pos; // the position of the node in the file 

    struct node_binded
    {
        // pointer to our body node that this node is in 
        struct node* owner;
         // pointer to the function this node is in.
        struct node* function;
    }binded;

    union  // the things every node need base on his type 
    {
        struct exp
        {
            struct node* left;
            struct node* right;
            const char* op;
        }exp;

        struct parenthesis
        {
            // the expression inside the parenthesis node 
            struct node* exp;
        }parenthesis;

        struct var
        {
            struct datatype type; // the type of the varable
            int padding; // the padding of the variable 
            int aoffset;// aligned offset
            const char* name; // the name pf the variable 
            struct node* val; 
        }var;

        struct node_tenary
        {
            struct node* true_node;
            struct node* false_node;
        } tenary;

        struct varlist
        {
            // a list of struct node* variables
            struct vector* list;
        } var_list;

        struct bracket
        {
            // int [50]; [50] would be our bracket node. the inner would be NODE_TYPE_NUMBER value of 50.
            struct node* inner;
        }bracket;

        struct _struct
        {
            const char* name;
            struct node* body_n;

            /**
             * struct abc
             * {
             * 
             * } var_name;
             * 
             * NULL if no variable attached to structure.
            */
            struct node* var;
        } _struct;


        struct _union
        {
            const char* name;
            struct node* body_n;

            /**
             * struct abc
             * {
             * 
             * } var_name;
             * 
             * NULL if no variable attached to structure.
            */
            struct node* var;
        } _union;

        struct body
        {
            // struct node* vector of statements
            struct vector* statements;

            // the size of combined variables inside this body.
            size_t size;

            // true if the variable size had to be increased due to padding in the body
            bool padded;

            // a pointer to the largest variable node in the statements vector.
            struct node* largest_var_node;
        } body;

        struct function
        {
            // special flags
            int flags;
            // return type i.w void, int, long ect...
            struct datatype rtype;

            // i.e function name "main"
            const char* name;

            struct function_arguments
            {
                // vector of struct node* . Must be type NODE_TYPE_VARIABLE
                struct vector* vector;

                // how much to add to the eBP to find the first argument. (in assembly stack we add)
                size_t stack_addition;
            }args;

            // pointer to the function body node , NULL if this is A function prototype
            struct node* body_n;

            // the stack size for all variables inside this function.
            size_t stack_size;
        }func;

        struct statement
        {
            struct return_stmt
            {
                // the expression of the return 
                struct node* exp;
            } return_stmt;

            struct if_stmt
            {
                // if(COND){// body}
                struct node* cond_node;
                struct node* body_node;

                //if(COND){} else{}
                struct node* next;
            } if_stmt;

            struct else_stmt
            {
                struct node* body_node;
            } else_stmt;

            struct for_stmt
            {
                struct node* init_node;
                struct node* cond_node;
                struct node* loop_node;
                struct node* body_node;

            } for_stmt;

            struct while_stmt
            {
                struct node* exp_node;
                struct node* body_node;
            } while_stmt;

             struct do_while_stmt
            {
                struct node* exp_node;
                struct node* body_node;
            } do_while_stmt;

            struct switch_stmt
            {
                struct node* exp;
                struct node* body;
                struct vector* cases;
                bool has_default_case;

            }switch_stmt;

            struct _case_stmt
            {
                struct node* exp;
            }_case;

            struct _goto_stmt
            {
                struct node* label;
            }_goto;

        } stmt;

        struct node_label
        {
            struct node* name;
        } label;
  
        struct cast
        {
            struct datatype dtype;
            struct node* operand;
        }cast;
    };

    union // the value of the node , if it have one (like the tokens)
    {
        char cval;
        const char* sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
    };

};

enum // enums for the datatypes flags 
{
    DATATYPE_FLAG_IS_SIGNED =0b00000001,
    DATATYPE_FLAG_IS_STATIC =0b00000010,
    DATATYPE_FLAG_IS_CONST =0b00000100,
    DATATYPE_FLAG_IS_POINTER =0b00001000,
    DATATYPE_FLAG_IS_ARRAY =0b00010000,
    DATATYPE_FLAG_IS_EXTERN =0b00100000,
    DATATYPE_FLAG_IS_RESTRICT =0b01000000,
    DATATYPE_FLAG_IGNORE_TYPE_CHECKING =0b10000000,
    DATATYPE_FLAG_IS_SECONDARY =0b100000000,
    DATATYPE_FLAG_STRUCT_UNION_NO_NAME =0b1000000000,
    DATATYPE_FLAG_IS_LITERAL =0b10000000000,
};

enum // enums for the datatype types 
{
    DATA_TYPE_VOID, 
    DATA_TYPE_CHAR,
    DATA_TYPE_SHORT,
    DATA_TYPE_INTEGER,
    DATA_TYPE_LONG,
    DATA_TYPE_FLOAT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_STRUCT,
    DATA_TYPE_UNION,
    DATA_TYPE_UNKNOWN
};


enum
{
    DATA_TYPE_EXPECT_PRIMITIVE,
    DATA_TYPE_EXPECT_UNION,
    DATA_TYPE_EXPECT_STRUCT
};

enum
{
    DATA_SIZE_ZERO = 0,
    DATA_SIZE_BYTE = 1,
    DATA_SIZE_WORD = 2,
    DATA_SIZE_DWORD = 4,
    DATA_SIZE_DDWORD = 8
};

enum
{
    // the flag is set for native functions.
    FUNCTION_NODE_FLAG_IS_NATIVE = 0b00000001,
};



// THE PROTOTYPES OF THE FUNCTIONS IN THE PROJECT  :

int file_compiler(const char* filename, const char* out_filename, int flags);
struct compile_process* compile_process_create(const char* filename,const char* filename_out,int flags);

char compile_process_next_char(struct lex_process* lex_process);
char compile_process_peek_char(struct lex_process* lex_process);
void compile_process_push_char(struct lex_process* lex_process,char c );

void system_error(struct compile_process* compiler, const char* msg, ...);
void system_warning(struct compile_process* compiler, const char* msg, ...);

struct lex_process* lex_process_create(struct compile_process* compiler,struct lex_process_functions* functions,void* private);
void lex_process_free(struct lex_process* process);
void* lex_process_private(struct lex_process* process);
struct vector* lex_process_tokens(struct lex_process* process);
int lex(struct lex_process* process);
int parse(struct compile_process* process);

/*   Builds token for the input string.*/

bool token_is_keyword(struct token* token, const char* value);
bool token_is_identifier(struct token* token);
bool token_is_symbol(struct token* token, char c );

bool token_is_nl_or_comment_or_newline_seperator(struct token* token);
bool keyword_is_datatype(const char *str);
bool token_is_primitive_keyword(struct token* token);

bool datatype_is_struct_or_union_for_name(const char* name);
size_t datatype_element_size(struct datatype* dtype);
size_t datatype_size_for_access(struct datatype* dtype);
size_t datatype_size_no_ptr(struct datatype* dtype);
size_t datatype_size(struct datatype* dtype);
bool datatype_is_primitive(struct datatype* dtype);

bool token_is_operator(struct token* token, const char* val);

struct node* node_create(struct node* _node);
struct node* node_from_sym(struct symbol* sym);
struct node* node_from_symbol(struct compile_process* current_process, const char* name);
bool node_is_expression(struct node *node, const char *op);
bool is_array_node(struct node *node);
bool is_node_assignment(struct node *node);
bool node_is_expression_or_parentheses(struct node* node);
bool node_is_value_type(struct node* node);

struct node* struct_node_for_name(struct compile_process* current_process, const char* name);
struct node* union_node_for_name(struct compile_process* current_process, const char* name);

void make_tenary_node(struct node* true_node, struct node* false_node);
void make_case_node(struct node* exp_node);
void make_goto_node(struct node* label_node);
void make_label_node(struct node* name_node);

void make_continue_node();
void make_break_node();

void make_cast_node(struct datatype* dtype, struct node* operand_node);
void make_exp_node(struct node* left_node, struct node* right_node,const char* op);
void make_exp_parentheses_node(struct node* exp_node);

void make_bracket_node(struct node* node);
void make_body_node(struct vector* body_vec, size_t size, bool padded , struct node* largest_var_node);
void make_struct_node(const char* name, struct node* body_node);
void make_union_node(const char* name, struct node* body_node);
void make_switch_node (struct node* exp_node, struct node* body_node, struct vector* cases, bool has_default_case);
void make_function_node(struct datatype* ret_type, const char* name, struct vector* arguments, struct node* body_node);
void make_while_node (struct node* exp_node, struct node* body_node);
void make_do_while_node(struct node* body_node, struct node* exp_node);
void make_for_node (struct node* init_node, struct node* cond_node, struct node* loop_node, struct node* body_node);
void make_return_node(struct node* exp_node);
void make_if_node(struct node* cond_node, struct node* body_node, struct node* next_node);
void make_else_node(struct node* body_node);


struct node* node_pop();
struct node * node_peek();
struct node* node_peek_or_null();
void node_push(struct node* node);
void node_set_vector(struct vector* vec, struct vector* root_vec);

bool node_is_expressionable( struct node* node);
struct node* node_peek_expressionable_or_null();
bool node_is_struct_or_union_variable(struct node* node);

struct array_brackets* array_brackets_new();
void array_brackets_free(struct array_brackets* brackets);
void array_brackets_add(struct array_brackets* brackets, struct node* bracket_node);
struct vector* array_brackets_node_vector(struct array_brackets* brackets);
size_t array_brackets_calculate_size_from_index(struct datatype* dtype, struct array_brackets* brackets,int index);
size_t array_brackets_calculate_size(struct datatype* dtype, struct array_brackets* brackets);
int array_total_indexes(struct datatype* dtype);
bool datatype_is_struct_or_union(struct datatype* dtype);
struct node* variable_struct_or_union_body_node(struct node* node);
struct node* variable_node_or_list(struct node* node);


// Gets the variable size from the given variable node
size_t variable_size (struct node* var_node);
// Sums the variable size of all variable nodes inside the variable list node 
// returns the result
// size_t the sum of all variable node sizes in the list
size_t variable_size_for_list(struct node* var_list_node);
struct node* variable_node(struct node* node);
bool variable_node_is_primitive(struct node* node);

int padding(int val, int to);
int align_value(int val, int to);
int align_value_treat_positive(int val, int to);
int compute_sum_padding(struct vector* vec);


struct scope* scope_new(struct compile_process* process, int flags);
struct scope* scope_create_root(struct compile_process* process);
void scope_free_root(struct compile_process* process);
void scope_iteration_start(struct scope* scope);
void* scope_iterate_back(struct scope* scope);
void* scope_last_entity_at_scope(struct scope* scope);
void* scope_last_entity_from_scope_stop_at(struct scope* scope, struct scope* stop_scope);
void* scope_last_entity_stop_at(struct compile_process* process, struct scope* stop_scope);
void* scope_last_entity(struct compile_process* process);
void scope_push(struct compile_process* process, void* ptr, size_t elem_size);
void scope_finish(struct compile_process* process);
struct scope* scope_current(struct compile_process* process);

void symresolver_initialize(struct compile_process* process);
void symresolver_new_table(struct compile_process* process);
void symresolver_end_table(struct compile_process* process);
void symresolver_build_for_node(struct compile_process* process, struct node* node);
struct symbol* symresolver_get_symbol(struct compile_process* process, const char* name);
struct symbol* symresolver_get_symbol_for_native_function(struct compile_process* process, const char* name);

size_t function_node_argument_stack_addition(struct node* node);

#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATORS_IN_GROUP 12

enum
{
    ASSOCIATIVITY_LEFT_TO_RIGHT,
    ASSOCIATIVITY_RIGHT_TO_LEFT
};

struct expressionable_op_precedence_group
{
    char* operators[MAX_OPERATORS_IN_GROUP];
    int associtivity;
};

struct fixup;


// fixes the fixup. return true if the fixup was successful
typedef bool(*FIXUP_FIX)(struct fixup* fixup); // a function pointer 

// signifies the fixup has been removed from memory. 
// the implementor of this function pointer should free any memory related to the fixup.
typedef void(*FIXUP_END)(struct fixup* fixup);// a function pointer 

struct fixup_config // the struct of the fixup_config
{
    FIXUP_FIX fix; // the function pointers 
    FIXUP_END end;
    void* private; // will contain private data 
};

struct fixup_system// the struct of the fixup_system 
{
    // a vector of fixups.
    struct vector* fixups;
};

enum
{
    FIXUP_FLAG_RESOLVED = 0b00000001 // the fixup has been resolved
};

struct fixup // the struct of the fixup 
{
    int flags; // the flags of the fixup
    struct fixup_system* system; // the fixup system 
    struct fixup_config config; // the fixup configuration 
};

struct fixup_system* fixup_sys_new();
struct fixup_config* fixup_config(struct fixup* fixup);
void fixup_free(struct fixup* fixup);
void fixup_start_iteration(struct fixup_system* system);
struct fixup* fixup_next(struct fixup_system* system);
void fixup_sys_fixups_free(struct fixup_system* system);
void fixup_sys_free(struct fixup_system* system);
int fixup_sys_unresolved_fixups_conut(struct fixup_system* system);
struct fixup* fixup_register(struct fixup_system* system, struct fixup_config* config);
bool fixup_resolve(struct fixup* fixup);
void* fixup_private(struct fixup* fixup);
bool fixups_resolve(struct fixup_system* system);


#endif