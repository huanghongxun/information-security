#include "parser.h"
#include <ctype.h>
#include <stdarg.h>
#include "string_util.h"
#include "error.h"
#include <stdlib.h>
#include <assert.h>

#define STATE_SUCCESS 0
#define STATE_ERROR 1

tiny_parser_result_t tiny_syntax_parse(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    if (ctx.current_parser == NULL)
    {
        return tiny_syntax_make_failure_result(tiny_scanner_now(scanner)->token, 0, TINY_INVALID_PARSER);
    }
    else
    {
        return ctx.current_parser->parser(ctx, scanner);
    }
}

tiny_parser_t *tiny_make_parser()
{
    tiny_parser_t *parser = malloc(sizeof(tiny_parser_t));
    if (!parser) error("tiny_make_parser failed");
    parser->parser = NULL;
    parser->sibling = parser->child = NULL;
    parser->func = NULL;
    parser->tag = 0;
    parser->class = -1;
    parser->error = 0;
    return parser;
}

tiny_parser_ctx_t tiny_syntax_make_context(struct trie *parsers, tiny_parser_t *current_parser, int tag, int length)
{
    tiny_parser_ctx_t ctx = {
        .parsers = parsers,
        .current_parser = current_parser,
        .tag = tag,
        .length = length};
    return ctx;
}

tiny_parser_result_t tiny_syntax_make_success_result(tiny_ast_t *ast)
{
    tiny_parser_result_t result;
    result.ast = ast;
    result.state = STATE_SUCCESS;
    result.required_token = 0;
    return result;
}

tiny_parser_result_t tiny_syntax_make_failure_result(tiny_lex_token_t token, uint8_t required_token, int error)
{
    assert(error != 0);
    tiny_parser_result_t result = {
        .ast = NULL,
        .state = error,
        .fatal = false,
        .error_token = token,
        .required_token = required_token};
    return result;
}

static tiny_parser_result_t parser_kleene(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_ast_t *ast = tiny_make_ast(ctx.current_parser->tag);

    while (true)
    {
        tiny_scanner_token_t *save = tiny_scanner_now(scanner);
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, 0, -1),
            scanner);

        if (next.state == STATE_SUCCESS)
        {
            // 解析成功，添加子 AST
            tiny_ast_add_child(ast, next.ast);
        }
        else
        {
            if (next.fatal)
                return next;
            tiny_scanner_reset(scanner, save);
            return tiny_syntax_make_success_result(ast);
        }
    }
}

tiny_parser_t *tiny_make_parser_kleene(tiny_parser_t *single)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_kleene;
    ret->child = single;
    return ret;
}

static tiny_parser_result_t parser_kleene_until(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_ast_t *ast = tiny_make_ast(ctx.current_parser->tag);
    tiny_parser_result_t one = tiny_syntax_make_success_result(NULL);

    while (true)
    {
        tiny_scanner_token_t *save = tiny_scanner_now(scanner);
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, 0, -1),
            scanner);

        if (next.state == STATE_SUCCESS)
        {
            // 解析成功，添加子 AST
            tiny_ast_add_child(ast, next.ast);
        }
        else
        {
            if (next.fatal)
                return next;
            one = next;
            tiny_scanner_reset(scanner, save);
            break;
        }
    }

    // 检查 terminator 是否存在
    {
        tiny_scanner_token_t *save = tiny_scanner_now(scanner);
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child->sibling, 0, -1),
            scanner);

        if (next.state == STATE_SUCCESS)
        {
            tiny_free_ast(next.ast);
            tiny_scanner_reset(scanner, save);
            return tiny_syntax_make_success_result(ast);
        }
        else
        {
            if (next.fatal)
                return next;
            if (one.state != STATE_SUCCESS)
                return one;
            else
                return next;
        }
    }
}

tiny_parser_t *tiny_make_parser_kleene_until(tiny_parser_t *terminator, tiny_parser_t *replica)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_kleene_until;
    ret->child = replica;
    replica->sibling = terminator;
    return ret;
}

static tiny_parser_result_t parser_or(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_parser_result_t one;
    int diff = -1;
    tiny_scanner_token_t *save = tiny_scanner_now(scanner);
    for (tiny_parser_t *cld = ctx.current_parser->child; cld; cld = cld->sibling)
    {
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, cld, 0, -1),
            scanner);

        if (next.state == STATE_SUCCESS)
        {
            if (ctx.current_parser->tag != 0 && next.ast)
                next.ast->tag = ctx.current_parser->tag;
            return next;
        }
        else
        {
            if (next.fatal)
                return next;
            int mydiff = tiny_scanner_diff(scanner, save);
            if (mydiff > diff)
            {
                diff = mydiff;
                one = next;
            }
            tiny_scanner_reset(scanner, save);
        }
    }
    return one;
}

tiny_parser_t *tiny_make_parser_or(int n, ...)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_or;
    tiny_parser_t **ptr = &ret->child;

    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; ++i)
    {
        *ptr = va_arg(ap, tiny_parser_t *);
        ptr = &((*ptr)->sibling);
    }
    return ret;
}

static tiny_parser_result_t parser_grammar(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_parser_ctx_t subctx = {
        .parsers = ctx.parsers,
        .current_parser = trie_search(ctx.parsers, ctx.current_parser->name)};

    assert(subctx.current_parser != NULL);
    return tiny_syntax_parse(subctx, scanner);
}

tiny_parser_t *tiny_make_parser_grammar(const char *name)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_grammar;
    ret->name = name;
    return ret;
}

static tiny_parser_result_t parser_optional(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_scanner_token_t *save = tiny_scanner_now(scanner);
    tiny_ast_t *ast = tiny_make_ast(ctx.current_parser->tag);
    tiny_parser_result_t result = tiny_syntax_parse(
        tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, 0, -1),
        scanner);

    if (result.state == STATE_SUCCESS)
        tiny_ast_add_child(ast, result.ast);
    else if (result.fatal)
        return result;
    else
        tiny_scanner_reset(scanner, save);

    return tiny_syntax_make_success_result(ast);
}

tiny_parser_t *tiny_make_parser_optional(tiny_parser_t *optional)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_optional;
    ret->child = optional;
    return ret;
}

static tiny_parser_result_t parser_token_predicate(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_lex_token_t token = tiny_scanner_next(scanner);
    if (token.error)
    {
        tiny_parser_result_t result = tiny_syntax_make_failure_result(token, 0, token.error);
        result.fatal = token.error != TINY_EOF;
        return result;
    }

    bool (*predicate)(uint8_t) = ctx.current_parser->func;
    if (predicate(token.byte))
    {
        tiny_ast_t *ast = tiny_make_ast(ctx.current_parser->tag);
        ast->token = token;
        return tiny_syntax_make_success_result(ast);
    }
    else
    {
        return tiny_syntax_make_failure_result(token, 0, TINY_UNEXPECTED_TOKEN);
    }
}

tiny_parser_t *tiny_make_parser_token_predicate(bool (*predicate)(uint8_t))
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_token_predicate;
    ret->func = predicate;
    return ret;
}

static tiny_parser_result_t parser_token_eof(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_lex_token_t token = tiny_scanner_next(scanner);
    if (token.error == TINY_EOF)
    {
        return tiny_syntax_make_success_result(NULL);
    }
    else if (token.error != 0)
    {
        tiny_parser_result_t result = tiny_syntax_make_failure_result(token, 0, token.error);
        result.fatal = token.error != TINY_EOF;
        return result;
    }
    else
    {
        return tiny_syntax_make_failure_result(token, 0, TINY_EOF);
    }
}

tiny_parser_t *tiny_make_parser_token_eof()
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_token_eof;
    return ret;
}

static tiny_parser_result_t parser_action(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_parser_result_t next = tiny_syntax_parse(
        tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, 0, -1),
        scanner);
    if (next.state == STATE_SUCCESS)
    {
        void (*action)(tiny_ast_t *) = ctx.current_parser->func;
        action(next.ast);
    }
    return next;
}

tiny_parser_t *tiny_make_parser_action(tiny_parser_t *parser, void (*action)(tiny_ast_t *))
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_action;
    ret->child = parser;
    ret->func = action;
    return ret;
}

static tiny_parser_result_t parser_eliminate(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_parser_result_t result = tiny_syntax_parse(
        tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, 0, -1),
        scanner);
    if (result.ast)
    {
        tiny_free_ast(result.ast);
        result.ast = NULL;
    }
    return result;
}

tiny_parser_t *tiny_make_parser_eliminate(tiny_parser_t *parser)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_eliminate;
    ret->child = parser;
    return ret;
}
