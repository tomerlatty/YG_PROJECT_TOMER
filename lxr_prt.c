#include "cmplr.h"
#include "structures/vector.h"
#include "structures/buffer.h"
#include <string.h>
#include <assert.h>
#include <ctype.h>

// lex_process.c responsible for lax process and lexer.c responsible for the actual lexicak analysis 


// this define - function , its a key in our file , and this function write the char to the buffer if the expression is true , in a for loop 
#define LEX_GETC_IF(buffer, c, exp)     \
    for (c = peekc(); exp; c = peekc()) \
    {                                   \
        buffer_write(buffer, c);        \
        nextc();                        \
    }

struct token *read_next_token();// protortype , all the file know this function now 
bool lex_is_in_expression(); // prototype

static struct lex_process *lex_process; // the lex_process struct 
static struct token tmp_token; // used for temporary measures 

static char peekc() // peek ahead in our file 
{
    return lex_process->function->peek_char(lex_process); // basically we will call compile_process_peek_char
}

static char nextc()// this function read the next char from the file 
{
    char c = lex_process->function->next_char(lex_process);

    if (lex_is_in_expression()) // if we inside an expression we will write the character to the expression buffer 
    {
        buffer_write(lex_process->parentheses_buffer, c);
    }

    lex_process->pos.col += 1;
    if (c == '\n')
    {
        lex_process->pos.line += 1;
        lex_process->pos.col = 1;
    }

    return c;
}

static void pushc(char c) // we push char in to our file 
{
    lex_process->function->push_char(lex_process, c);
}

static char assert_next_char(char c) // function for assert next_c != c
{
    char next_c = nextc();
    assert(c == next_c);
    return next_c;
}

static struct pos lex_file_position() // return the position of the lex 
{
    return lex_process->pos;
}

struct token *token_create(struct token *_token) // create a token (copy to tmp_token)
{
    memcpy(&tmp_token, _token, sizeof(struct token));
    tmp_token.pos = lex_file_position();
    if (lex_is_in_expression())
    {
        tmp_token.between_brackets = buffer_ptr(lex_process->parentheses_buffer); // a pointer to the brackets of the expression
    }
    return &tmp_token;
}

static struct token *lexer_last_token() // return us the last token we pushed to the lexa 
{
    return vector_back_or_null(lex_process->token_vec);
}

static struct token *handle_whitespace() // this function handle whitespaces between tokens 
{
    struct token *last_token = lexer_last_token();
    if (last_token)
    {
        last_token->whitespace = true; // if it was a whitespace between the tokens 
    }

    nextc();
    return read_next_token();
}

const char *read_number_str() // return us the buffer with the number that he read 
{
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, (c >= '0' && c <= '9')); // if its a number 

    buffer_write(buffer, 0x00); // null terminator 
    return buffer_ptr(buffer); // return the actual pointer to the actual buffer data 
}

unsigned long long read_number() // we convert the string  to a number , with atoll 
{
    const char *s = read_number_str(); // read the number 
    return atoll(s); // translate to number
}

int lexer_number_type(char c) // return the type of the number 
{
    int res = NUMBER_TYPE_NORMAL;
    if (c == 'L')
    {
        res = NUMBER_TYPE_LONG;
    }
    else if (c == 'f')
    {
        res = NUMBER_TYPE_FLOAT;
    }

    return res;
}

struct token *token_make_number_for_value(unsigned long number)  // the function of making a number token 
{
    int number_type = lexer_number_type(peekc()); // what is the type of the number 
    if (number_type != NUMBER_TYPE_NORMAL)
    {
        nextc(); // pass the L or the f 
    }
    return token_create(&(struct token){.type = TOKEN_TYPE_NUMBER, .llnum = number, .num.type = number_type}); // create the token of number 
}

struct token *token_make_number() // return us a number token 
{
    return token_make_number_for_value(read_number()); 
}

static struct token *token_make_string(char start_delim, char end_delim) // make a string token 
{
    struct buffer *buf = buffer_create(); // buffer to hold the strimg 
    assert(nextc() == start_delim); // if its not start with "
    char c = nextc();
    for (; c != end_delim && c != EOF; c = nextc())
    {
        if (c == '\\')
        {
            // we dont handle an escape characters .
            continue;
        }
        buffer_write(buf, c);
    }
    buffer_write(buf, 0x00); // NULL TERMINATOR 
    return token_create(&(struct token){.type = TOKEN_TYPE_STRING, .sval = buffer_ptr(buf)});
}

static bool op_treated_as_one(char op) // this function tell us if we can only use operator once 
{
    return op == '(' || op == '[' || op == ',' || op == '.' || op == '*' || op == '?';
} // we handle the * different thats why its here 

static bool is_single_operator(char op) // return true id the char is an operator 
{
    return op == '+' ||
           op == '-' ||
           op == '/' ||
           op == '*' ||
           op == '=' ||
           op == '>' ||
           op == '<' ||
           op == '|' ||
           op == '&' ||
           op == '^' ||
           op == '%' ||
           op == '!' ||
           op == '(' ||
           op == '[' ||
           op == ',' ||
           op == '.' ||
           op == '~' ||
           op == '?';
}

bool op_valid(const char *op) // all the combinations that valid 
{
    return S_EQ(op, "+") ||
           S_EQ(op, "-") ||
           S_EQ(op, "*") ||
           S_EQ(op, "/") ||
           S_EQ(op, "!") ||
           S_EQ(op, "^") ||
           S_EQ(op, "+=") ||
           S_EQ(op, "-=") ||
           S_EQ(op, "*=") ||
           S_EQ(op, "/=") ||
           S_EQ(op, ">>") ||
           S_EQ(op, "<<") ||
           S_EQ(op, ">=") ||
           S_EQ(op, "<=") ||
           S_EQ(op, ">") ||
           S_EQ(op, "<") ||
           S_EQ(op, "||") ||
           S_EQ(op, "&&") ||
           S_EQ(op, "|") ||
           S_EQ(op, "&") ||
           S_EQ(op, "++") ||
           S_EQ(op, "--") ||
           S_EQ(op, "=") ||
           S_EQ(op, "!=") ||
           S_EQ(op, "==") ||
           S_EQ(op, "->") ||
           S_EQ(op, "(") ||
           S_EQ(op, "[") ||
           S_EQ(op, ",") ||
           S_EQ(op, ".") ||
           S_EQ(op, "...") ||
           S_EQ(op, "~") ||
           S_EQ(op, "?") ||
           S_EQ(op, "%");
}

void read_op_flush_back_keep_first(struct buffer *buffer) // push the characters back to the stream
{
    const char *data = buffer_ptr(buffer);
    int len = buffer->len;
    for (int i = len - 1; i >= 1; i--)
    {
        if (data[i] == 0x00)
        {
            continue;
        }
        pushc(data[i]);
    }
}

const char *read_op() // this function read an opertator from the file 
{
    bool single_operator = true;
    char op = nextc();
    struct buffer *buffer = buffer_create();
    buffer_write(buffer, op);

    if (!op_treated_as_one(op)) // something like ++
    {
        op = peekc();
        if (is_single_operator(op))
        {
            buffer_write(buffer, op);
            nextc(); // we pop the char we peek 
            single_operator = false;
        }
    }
  
    buffer_write(buffer, 0x00);   // NULL TERMINATOR
    char *ptr = buffer_ptr(buffer);
    if (!single_operator)
    {
        if (!op_valid(ptr)) // we need to check that its valid i.e "+-", so if its not valid 
        {
            read_op_flush_back_keep_first(buffer); // we need to push back 
            ptr[1] = 0x00; // NULL TERMINATOR 
        }
    }
    else if (!op_valid(ptr)) // if its not valid 
    {
        system_error(lex_process->compiler, "The operator %s is not valid\n", ptr);
    }
    return ptr;
}

static void lex_new_expression() // if the token that created is a new expression 
{
    lex_process->current_expression_count++;
    if (lex_process->current_expression_count == 1)
    {
        lex_process->parentheses_buffer = buffer_create(); // 1( 2( 3) 4) so after 1 everything will be shown to you in 2 3 
    }
}

static void lex_finish_expression() // function that closing the expression 
{
    lex_process->current_expression_count--;
    if (lex_process->current_expression_count < 0) // (()
    {
        system_error(lex_process->compiler, "you closed an expression that you never opend\n");
    }
}

bool lex_is_in_expression() // return if the process we do is inside a expression
{
    return lex_process->current_expression_count > 0;
}

bool keyword_is_datatype(const char *str)
{
    return S_EQ(str, "void") ||
           S_EQ(str, "char") ||
           S_EQ(str, "int") ||
           S_EQ(str, "short") ||
           S_EQ(str, "float") ||
           S_EQ(str, "double") ||
           S_EQ(str, "long") ||
           S_EQ(str, "struct") ||
           S_EQ(str, "union");
}

bool is_keyword(const char *str) // return true if the string is a keyword 
{
    return S_EQ(str, "unsigned") ||
           S_EQ(str, "signed") ||
           S_EQ(str, "char") ||
           S_EQ(str, "short") ||
           S_EQ(str, "int") ||
           S_EQ(str, "long") ||
           S_EQ(str, "float") ||
           S_EQ(str, "double") ||
           S_EQ(str, "void") ||
           S_EQ(str, "struct") ||
           S_EQ(str, "union") ||
           S_EQ(str, "static") ||
           S_EQ(str, "__ignore_typecheck") ||
           S_EQ(str, "return") ||
           S_EQ(str, "include") ||
           S_EQ(str, "sizeof") ||
           S_EQ(str, "if") ||
           S_EQ(str, "else") ||
           S_EQ(str, "while") ||
           S_EQ(str, "for") ||
           S_EQ(str, "do") ||
           S_EQ(str, "break") ||
           S_EQ(str, "continue") ||
           S_EQ(str, "switch") ||
           S_EQ(str, "case") ||
           S_EQ(str, "default") ||
           S_EQ(str, "goto") ||
           S_EQ(str, "typedef") ||
           S_EQ(str, "const") ||
           S_EQ(str, "extern") ||
           S_EQ(str, "restrict");
}

static struct token *token_make_operator_or_string() // the function that make operator token or string token include
{
    char op = peekc(); // read a char from the file 
    if (op == '<') // like <stdio.h
    {
        struct token *last_token = lexer_last_token(); // we want to check if there is a #include before  
        if (token_is_keyword(last_token, "include")) // #include <stdio.h>
        {
            return token_make_string('<', '>');
        }
    }
    struct token *token = token_create(&(struct token){.type = TOKEN_TYPE_OPERATOR, .sval = read_op()}); // create the operator token
    if (op == '(') // if the operator is an expression 
    {
        lex_new_expression();
    }
    return token;
}

struct token *token_make_one_line_comment() //handle the building of the // comment 
{
    struct buffer *buffer = buffer_create(); // the buffer to read the content 
    char c = 0;
    LEX_GETC_IF(buffer, c, c != '\n' && c != EOF); // read the content from the file 
    return token_create(&(struct token){.type = TOKEN_TYPE_COMMENT, .sval = buffer_ptr(buffer)});// make the comment token 
}

struct token *token_make_multiline_comment() //   handle  /**/ and create the token 
{
    struct buffer *buffer = buffer_create();
    char c = 0;
    while (1)
    {
        LEX_GETC_IF(buffer, c, c != '*' && c != EOF);
        if (c == EOF)
        {
            system_error(lex_process->compiler, "You did not close this multiline comment\n"); // /*
        }
        else if (c == '*')
        {
            // skip the *
            nextc();

            if (peekc() == '/') // we cam use how many *** we want  /* ******** */
            {
                nextc();
                break;
            }
        }
    }
    return token_create(&(struct token){.type = TOKEN_TYPE_COMMENT, .sval = buffer_ptr(buffer)}); // create the comment token 
}

struct token *handle_comment() // the function of handling a comment 
{
    char c = peekc(); // peek a char from the file 
    if (c == '/')
    {
        nextc();
        if (peekc() == '/') // if its a // 
        {
            nextc();
            return token_make_one_line_comment();
        }
        else if (peekc() == '*')
        {
            nextc();
            return token_make_multiline_comment(); // if its a /**/ 
        }

        pushc('/'); // push it back to the file 
        return token_make_operator_or_string();
    }

    return NULL;
}

static struct token *token_make_symbol() // the function that create a new symbol token
{
    char c = nextc(); // pass the symbol 
    if (c == ')')  // (.... )    so we need to close the expression
    {
        lex_finish_expression();
    }

    struct token *token = token_create(&(struct token){.type = TOKEN_TYPE_SYMBOL, .cval = c}); // create the symbol create
    return token;
}

static struct token *token_make_identifier_or_keyword() // function that make identifier token or keyword token 
{
    struct buffer *buffer = buffer_create();
    char c = 0;
    LEX_GETC_IF(buffer, c, (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'); // we get the word 

    // null terminator
    buffer_write(buffer, 0x00);

    // check if this is a keyword
    if (is_keyword(buffer_ptr(buffer)))
    {
        return token_create(&(struct token){.type = TOKEN_TYPE_KEYWORD, .sval = buffer_ptr(buffer)});// creating an keyword token 
    }

    return token_create(&(struct token){.type = TOKEN_TYPE_IDENTIFIER, .sval = buffer_ptr(buffer)}); // creating an identifier token 
}

struct token *read_special_token() // for situations where we dont really know what something is until we look further ahead
{
    char c = peekc();
    if (isalpha(c) || c == '_')
    {
        return token_make_identifier_or_keyword();
    }

    return NULL;
}

struct token *token_make_newline() // function that make a newline token 
{
    nextc(); 
    return token_create(&(struct token){.type = TOKEN_TYPE_NEWLINE}); // creating a new line token 
}

char lex_get_escaped_char(char c) // handle the escaped characters 
{
    char co = 0;
    switch (c)
    {
    case 'n':
        co = '\n';
        break;

    case '\\':
        co = '\\';
        break;

    case 't':
        co = '\t';
        break;

    case '\'':
        co = '\'';
        break;

    // we can add here whatever we want ..

    }

    


    return co;
}

void lexer_pop_token() // pop the last token from the vector 
{
    vector_pop(lex_process->token_vec);
}

bool is_hex_char(char c) // all the hexa options 
{
    c = tolower(c);
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

const char *read_hex_number_str() // read the hexadecimal number 
{
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, is_hex_char(c)); // if its on the conditions of the hex
    // Write our null terminator
    buffer_write(buffer, 0x00);
    return buffer_ptr(buffer);
}

struct token *token_make_special_number_hexadecimal() // return a hexadecimal number token 
{
    // skip the "x"
    nextc();

    unsigned long number = 0;
    const char *number_str = read_hex_number_str();
    number = strtol(number_str, 0, 16); // convert hexadecimal string into a number that the computer can underdtand 
    return token_make_number_for_value(number);
}

void lexer_validate_binary_string(const char *str) // check that a string is a binary number 
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] != '0' && str[i] != '1') // only can be 0,1
        {
            system_error(lex_process->compiler, "This is not a valid binary number\n");
        }
    }
}

struct token *token_make_special_number_binary()// return a binary  number token 
{
    // skip the "b"
    nextc();

    unsigned long number = 0;
    const char *number_str = read_number_str();
    lexer_validate_binary_string(number_str);
    number = strtol(number_str, 0, 2);// convert binary string into a number that the computer can underdtand 
    return token_make_number_for_value(number); // make the token 
}

struct token *token_make_special_number() // number can be  hexadecimal, binary, decimal  xAB43  b01110101110 
{
    struct token *token = NULL;
    struct token *last_token = lexer_last_token();// check the last token 
    if (!last_token || !(last_token->type == TOKEN_TYPE_NUMBER && last_token->llnum == 0)) // 0x 0b , before a special number we need to add the zero , and if there is no zero so the special number its not a special number
    {
        return token_make_identifier_or_keyword(); // if its not a special number 
    }
    lexer_pop_token();

    char c = peekc();
    if (c == 'x') // hexa 0x50
    {
        token = token_make_special_number_hexadecimal();
    }
    else if (c == 'b') // binary 0b010
    {
        token = token_make_special_number_binary();
    }

    return token;
}

struct token *token_make_quote() // creating a quote '' token
{
    assert_next_char('\'');
    char c = nextc();
    if (c == '\\')
    {
        c = nextc();
        c = lex_get_escaped_char(c);
    }
    if (nextc() != '\'')
    {
        system_error(lex_process->compiler, "You opened a quote ' but did not close it with a ' character");
    }
    return token_create(&(struct token){.type = TOKEN_TYPE_NUMBER, .cval = c}); //every char habe a ascii, even a number is inside a quote it still a number
}

struct token *read_next_token() // the function that responsible of building the tokens 
{
    struct token *token = NULL;
    char c = peekc(); // peek a char from the file 

    token = handle_comment(); // check if its a comment 
    if (token) 
    {
        return token; // we process the comment 
    }
    switch (c)
    {
    NUMERIC_CASE: // if its one of the cases on the NUMERIC_CASE enum
        token = token_make_number();
        break;

    OPERATOR_CASE_EXCLUDING_DIVISION: // if c its one of the oparators , excluding division because forward slash can also be at the start of a comment 
        token = token_make_operator_or_string();
        break;

    SYMBOL_CASE: // if c its one of the symbols  
        token = token_make_symbol();
        break;

    case 'b':
    case 'x':
        token = token_make_special_number(); // make the hexa or binary number 
        break;

    case '"': // because string must to be  in "" 
        token = token_make_string('"', '"');
        break;
    case '\'': // case quote 
        token = token_make_quote();
        break;

    // we dont care about whitespace, tabs,  ignore them
    case ' ':
    case '\t':
        token = handle_whitespace();
        break;

    case '\n': // case its a new line 
        token = token_make_newline();
        break;
    case EOF: //END OF FILE 
        // we have finished lexical analysis on the file
        break;

    default: //case that we dont know what is this and we need to handle this 
        token = read_special_token();
        if (!token) // if read_special_token failed 
        {
            system_error(lex_process->compiler, "Unexpected token\n");
        }
    }
    return token;
}
int lex(struct lex_process *process) //the main function of the lexical analysis 
{
    // the stuff that we need to know about the process , intilaize 
    process->current_expression_count = 0;
    process->parentheses_buffer = NULL; 
    lex_process = process;
    process->pos.filename = process->compiler->cfile.abs_path;

    struct token *token = read_next_token();
    while (token) // read all the tokens in the file 
    {
        vector_push(process->token_vec, token); // we push all the tokens to the vector 
        token = read_next_token(); // read the next token 
    }
    return LEXICAL_ANALYSIS_ALL_OK;
}

char lexer_string_buffer_next_char(struct lex_process *process) //  move on to the next letter in the file 
{
    struct buffer *buf = lex_process_private(process);
    return buffer_read(buf);
}

char lexer_string_buffer_peek_char(struct lex_process *process)//  peek the next letter in the file 
{
    struct buffer *buf = lex_process_private(process);
    return buffer_peek(buf);
}

void lexer_string_buffer_push_char(struct lex_process *process, char c)  // push a letter into the file 
{
    struct buffer *buf = lex_process_private(process);
    buffer_write(buf, c);
}

struct lex_process_functions lexer_string_buffer_functions = { // the functions that we will need for the lexa process
    .next_char = lexer_string_buffer_next_char,
    .peek_char = lexer_string_buffer_peek_char,
    .push_char = lexer_string_buffer_push_char

};

struct lex_process *tokens_build_for_string(struct compile_process *compiler, const char *str)
// essentialy anyone outside of alexa can call this function , they can pass in a string and the function will then create a series of tokens in a lex process instance 
{
    struct buffer *buffer = buffer_create();
    buffer_printf(buffer, str);
    struct lex_process *lex_process = lex_process_create(compiler, &lexer_string_buffer_functions, buffer);
    if (!lex_process)
    {
        return NULL;
    }

    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK)
    {
        return NULL;
    }

    return lex_process;
}