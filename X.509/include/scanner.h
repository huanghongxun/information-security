#ifndef SCANNER_H
#define SCANNER_H

#include "list.h"
#include "defs.h"

typedef struct tiny_lex_token_s
{
    uint8_t byte;
    int error;
} tiny_lex_token_t;

typedef struct tiny_scanner_token_s
{
    tiny_lex_token_t token;

    list_entry_t list;
} tiny_scanner_token_t;

typedef struct tiny_scanner_s
{
    list_entry_t tokens;

    list_entry_t *cur;

    void *ctx;
    void (*reader)(void *ctx, tiny_lex_token_t *token);
} tiny_scanner_t;

#define le2scannertoken(le, member) \
    to_struct((le), struct tiny_scanner_token_s, member)

tiny_scanner_token_t *tiny_scanner_now(tiny_scanner_t *);

tiny_lex_token_t tiny_scanner_next(tiny_scanner_t *);

tiny_lex_token_t tiny_scanner_peek(tiny_scanner_t *);

void tiny_scanner_reset(tiny_scanner_t *, tiny_scanner_token_t *token);

int tiny_scanner_diff(tiny_scanner_t *scanner, tiny_scanner_token_t *token);

void tiny_scanner_begin(tiny_scanner_t *scanner, void *ctx, void (*reader)(void *ctx, tiny_lex_token_t *token));

#endif