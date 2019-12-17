#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "asn.h"
#include "oid.h"
#include "error.h"
#include "x509.h"

#define BUF_SIZE 1024

struct byte_reader
{
    uint8_t *bytes;
    size_t cur;
    size_t len;
};

struct byte_reader read_all(FILE *stream)
{
    size_t len;
    size_t content_len = 0;
    size_t content_size = BUF_SIZE;
    uint8_t *content = malloc(content_size);
    if (!content)
    {
        error("read_all failed");
    }
    // TODO: 需要处理 buf 大小不足的问题
    while ((len = fread(content + content_len, sizeof(uint8_t), BUF_SIZE, stream)))
    {
        content_len += len;
        while (content_size - (content_len + 1) < BUF_SIZE)
        {
            content = realloc(content, content_size *= 2);
            if (!content)
            {
                error("read_all failed");
            }
        }
    }

    content[content_len] = 0;

    struct byte_reader reader;
    reader.bytes = content;
    reader.cur = 0;
    reader.len = content_len;
    return reader;
}

void byte_reader(void *ctx, tiny_lex_token_t *token)
{
    struct byte_reader *reader = ctx;
    token->error = reader->cur >= reader->len ? EOF : 0;
    if (!token->error)
        token->byte = reader->bytes[reader->cur++];
}

void usage(int argc, char *argv[])
{
    fprintf(stderr, "X.509 certificate reader\n");
    fprintf(stderr, "Usage: %s [mode: ASN/X509] [file]\n\n", argv[0]);
}

void print_ast(tiny_ast_t *ast, int indent, FILE *stream)
{
    for (int i = 0; i < indent; ++i)
        fprintf(stream, "  ");
    if (ast->class != 0) fprintf(stream, "[%d] ", ast->tag);
    else fprintf(stream, "%s ", find_parser_name_by_tag(ast->tag));
    switch (ast->value_type)
    {
        case AST_VALUE_STRING: fprintf(stream, "%s", (char *)ast2data_ast(ast)->value); break;
        case AST_VALUE_INTEGRAL: fprintf(stream, "%ld", (int64_t)ast2int_ast(ast)->value); break;
        case AST_VALUE_STRUCT: {
            if (ast->tag == ASN_TAG_OBJECT_IDENTIFIER)
            {
                const char *(*oid)[3] = ast2data_ast(ast)->value;
                fprintf(stream, "%s %s (%s)", (*oid)[0], (*oid)[1], (*oid)[2]);
            }
        } break;
    }
    fprintf(stream, "\n");
    if (ast->child)
        print_ast(ast->child, indent + 1, stream);
    if (ast->sibling)
        print_ast(ast->sibling, indent, stream);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        usage(argc, argv);
        return 1;
    }

    FILE *infile = fopen(argv[2], "rb");
    if (!infile)
        error("Unable to open input file %s", argv[3]);
    struct byte_reader reader = read_all(infile);

    OID_init_trie();
    asn_parser_init();

    tiny_scanner_t scanner;
    tiny_scanner_begin(&scanner, &reader, byte_reader);

    if (strcmp(argv[1], "ASN") == 0)
    {
        tiny_parser_ctx_t ctx;
        ctx.current_parser = ASN_PARSER_ANY;
        tiny_parser_result_t result = tiny_syntax_parse(ctx, &scanner);
        if (result.state == 0)
        {
            print_ast(result.ast, 0, stdout);
        }
        else
        {
            error("Failed to parse certificate");
        }
    }
    else if (strcmp(argv[1], "X509") == 0)
    {
        tiny_parser_ctx_t ctx;
        ctx.parsers = prepare_parsers();
        ctx.current_parser = trie_search(ctx.parsers, "Certificate");
        tiny_parser_result_t result = tiny_syntax_parse(ctx, &scanner);
        if (result.state == 0)
        {
            // print_ast(result.ast, 0, stdout);
        }
        else
        {
            error("Failed to parse certificate: %x", result.state);
        }
    }
    else
    {
        error("Unrecognized mode: %s, only 'ASN' or 'X509' is accepted", argv[2]);
    }
    
    

    return 0;
}