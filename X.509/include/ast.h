#ifndef AST_H
#define AST_H

#include "scanner.h"
#include <stddef.h>

#define AST_VALUE_UNDEFINED 0
#define AST_VALUE_INTEGRAL 1
#define AST_VALUE_STRING 2
#define AST_VALUE_STRUCT 3
#define AST_VALUE_BYTE_ARRAY 4

typedef struct tiny_ast_s
{
    uint8_t class;
    uint8_t constructed;
    int tag;
    tiny_lex_token_t token;
    int value_type;
    void (*free)(struct tiny_ast_s *);

    struct tiny_ast_s *child;
    struct tiny_ast_s *sibling;
} tiny_ast_t;

typedef struct tiny_int_ast_s
{
    int value;
    tiny_ast_t ast;
} tiny_int_ast_t;
#define ast2int_ast(le) to_struct((le), tiny_int_ast_t, ast)
#define tiny_make_int_ast(ast, desc, valuetype)           \
    tiny_int_ast_t *ast = malloc(sizeof(tiny_int_ast_t)); \
    if (!ast)                                             \
        error("tiny_make_int_ast failed");                \
    tiny_init_ast(&ast->ast, desc);                       \
    ast->ast.value_type = valuetype;                      \
    ast->ast.free = tiny_free_int_ast
void tiny_free_int_ast(tiny_ast_t *);

typedef struct tiny_data_ast_s
{
    int length;
    void *value;
    tiny_ast_t ast;
} tiny_data_ast_t;
#define ast2data_ast(le) to_struct((le), tiny_data_ast_t, ast)
#define tiny_make_data_ast(ast, desc, valuetype)            \
    tiny_data_ast_t *ast = malloc(sizeof(tiny_data_ast_t)); \
    if (!ast)                                               \
        error("tiny_make_data_ast failed");                 \
    tiny_init_ast(&ast->ast, desc);                         \
    ast->ast.value_type = valuetype;                        \
    ast->ast.free = tiny_free_data_ast
void tiny_free_data_ast(tiny_ast_t *);

void tiny_init_ast(tiny_ast_t *ast, int desc);
tiny_ast_t *tiny_make_ast(int desc);

void tiny_free_ast(tiny_ast_t *ast);

void tiny_ast_add_child(tiny_ast_t *ast, tiny_ast_t *child);

size_t tiny_ast_child_count(tiny_ast_t *ast);

#endif // AST_H