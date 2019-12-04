#include "ast.h"
#include <stdlib.h>

void tiny_ast_add_child(tiny_ast_t *ast, tiny_ast_t *child)
{
    if (!child)
        return;
    tiny_ast_t **ptr = &ast->child;
    while (*ptr)
        ptr = &((*ptr)->sibling);
    *ptr = child;
}

void tiny_init_ast(tiny_ast_t *ast, int tag)
{
    ast->child = ast->sibling = NULL;
    ast->token.byte = 0;
    ast->token.error = 0;
    ast->constructed = false;
    ast->tag = tag;
    ast->value_type = AST_VALUE_UNDEFINED;
}

tiny_ast_t *tiny_make_ast(int tag)
{
    tiny_ast_t *ast = malloc(sizeof(tiny_ast_t));
    if (!ast) error("tiny_make_ast failed");
    tiny_init_ast(ast, tag);
    return ast;
}

void tiny_free_ast(tiny_ast_t *ast)
{
    if (!ast)
        return;
    tiny_free_ast(ast->child);
    tiny_free_ast(ast->sibling);

    if (ast->free) ast->free(ast);
    else free(ast);
}

void tiny_free_int_ast(tiny_ast_t *ast)
{
    free(ast2int_ast(ast));
}

void tiny_free_data_ast(tiny_ast_t *ast)
{
    free(ast2data_ast(ast));
}

size_t tiny_ast_child_count(tiny_ast_t *ast)
{
    size_t cnt = 0;
    for (tiny_ast_t *ptr = ast->child; ptr; ptr = ptr->sibling)
        cnt++;
    return cnt;
}
