#include "asn.h"
#include "parser.h"
#include "oid.h"
#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

tiny_parser_t *ASN_PARSER_ANY;

static tiny_parser_t *find_parser_by_tag(uint8_t tag)
{
    switch (tag)
    {
        case ASN_TAG_EOC: return ASN_PARSER_EOC; break;
        case ASN_TAG_BOOLEAN: return ASN_PARSER_BOOLEAN; break;
        case ASN_TAG_INTEGER: return ASN_PARSER_INTEGER; break;
        case ASN_TAG_BIT_STRING: return ASN_PARSER_BIT_STRING; break;
        case ASN_TAG_OCTET_STRING: return ASN_PARSER_OCTET_STRING; break;
        case ASN_TAG_NULL: return ASN_PARSER_NULL; break;
        case ASN_TAG_OBJECT_IDENTIFIER: return ASN_PARSER_OBJECT_IDENTIFIER; break;
        case ASN_TAG_OBJECT_DESCRIPTOR: return ASN_PARSER_OBJECT_DESCRIPTOR; break;
        case ASN_TAG_EXTERNAL: return ASN_PARSER_EXTERNAL; break;
        case ASN_TAG_REAL: return ASN_PARSER_REAL; break;
        case ASN_TAG_ENUMERATED: return ASN_PARSER_ENUMERATED; break;
        case ASN_TAG_EMBEDDED_PDV: return ASN_PARSER_EMBEDDED_PDV; break;
        case ASN_TAG_UTF8_STRING: return ASN_PARSER_UTF8_STRING; break;
        case ASN_TAG_SEQUENCE: return ASN_PARSER_SEQUENCE; break;
        case ASN_TAG_SET: return ASN_PARSER_SET; break;
        case ASN_TAG_NUMERIC_STRING: return ASN_PARSER_NUMERIC_STRING; break;
        case ASN_TAG_PRINTABLE_STRING: return ASN_PARSER_PRINTABLE_STRING; break;
        case ASN_TAG_TELETEX_STRING: return ASN_PARSER_TELETEX_STRING; break;
        case ASN_TAG_VIDEOTEX_STRING: return ASN_PARSER_VIDEOTEX_STRING; break;
        case ASN_TAG_IA5_STRING: return ASN_PARSER_IA5_STRING; break;
        case ASN_TAG_UTC_TIME: return ASN_PARSER_UTC_TIME; break;
        case ASN_TAG_GENERALIZED_TIME: return ASN_PARSER_GENERALIZED_TIME; break;
        case ASN_TAG_GRAPHIC_STRING: return ASN_PARSER_GRAPHIC_STRING; break;
        case ASN_TAG_VISIBLE_STRING: return ASN_PARSER_VISIBLE_STRING; break;
        case ASN_TAG_GENERAL_STRING: return ASN_PARSER_GENERAL_STRING; break;
        case ASN_TAG_UNIVERSAL_STRING: return ASN_PARSER_UNIVERSAL_STRING; break;
        case ASN_TAG_BMP_STRING: return ASN_PARSER_BMP_STRING; break;
    }
    return NULL;
}

const char *find_parser_name_by_tag(uint8_t tag)
{
    switch (tag)
    {
        case ASN_TAG_EOC: return "EOC"; break;
        case ASN_TAG_BOOLEAN: return "BOOLEAN"; break;
        case ASN_TAG_INTEGER: return "INTEGER"; break;
        case ASN_TAG_BIT_STRING: return "BIT_STRING"; break;
        case ASN_TAG_OCTET_STRING: return "OCTET_STRING"; break;
        case ASN_TAG_NULL: return "NULL"; break;
        case ASN_TAG_OBJECT_IDENTIFIER: return "OBJECT_IDENTIFIER"; break;
        case ASN_TAG_EXTERNAL: return "EXTERNAL"; break;
        case ASN_TAG_REAL: return "REAL"; break;
        case ASN_TAG_ENUMERATED: return "ENUMERATED"; break;
        case ASN_TAG_EMBEDDED_PDV: return "EMBEDDED_PDV"; break;
        case ASN_TAG_UTF8_STRING: return "UTF8_STRING"; break;
        case ASN_TAG_SEQUENCE: return "SEQUENCE"; break;
        case ASN_TAG_SET: return "SET"; break;
        case ASN_TAG_NUMERIC_STRING: return "NUMERIC_STRING"; break;
        case ASN_TAG_PRINTABLE_STRING: return "PRINTABLE_STRING"; break;
        case ASN_TAG_TELETEX_STRING: return "TELETEX_STRING"; break;
        case ASN_TAG_VIDEOTEX_STRING: return "VIDEOTEX_STRING"; break;
        case ASN_TAG_IA5_STRING: return "IA5_STRING"; break;
        case ASN_TAG_UTC_TIME: return "UTC_TIME"; break;
        case ASN_TAG_GENERALIZED_TIME: return "GENERALIZED_TIME"; break;
        case ASN_TAG_GRAPHIC_STRING: return "GRAPHIC_STRING"; break;
        case ASN_TAG_VISIBLE_STRING: return "VISIBLE_STRING"; break;
        case ASN_TAG_GENERAL_STRING: return "GENERAL_STRING"; break;
        case ASN_TAG_UNIVERSAL_STRING: return "UNIVERSAL_STRING"; break;
        case ASN_TAG_BMP_STRING: return "BMP_STRING"; break;
    }
    return NULL;
}

asn_tag_t parse_asn_tag(tiny_scanner_t *scanner)
{
    asn_tag_t tag;
    tiny_lex_token_t token = tiny_scanner_next(scanner);
    if (token.error) goto fail;
    tag.class = token.byte >> 6;
    tag.constructed = (token.byte & 0x20) != 0;
    tag.number = token.byte & 0x1F;
    if (tag.number == 0x1F) {
        int number = 0;
        do {
            token = tiny_scanner_next(scanner);
            if (token.error) goto fail;
            number = (number << 8) | token.byte;
        } while (token.byte & 0x80);
        tag.number = number;
    }
    return tag;
fail:
    tag.number = -1;
    tag.token = token;
    return tag;
}

static tiny_parser_result_t parser_sequence(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_ast_t *ast = tiny_make_ast(ASN_TAG_SEQUENCE);
    tiny_scanner_token_t *save = tiny_scanner_now(scanner);

    while (tiny_scanner_diff(scanner, save) < ctx.length)
    {
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, 0, -1),
            scanner);

        if (next.state == 0)
        {
            tiny_ast_add_child(ast, next.ast);
        }
        else
        {
            tiny_free_ast(ast);
            return next;
        }
    }

    if (tiny_scanner_diff(scanner, save) != ctx.length)
        return tiny_syntax_make_failure_result(save->token, 0, ASN_INVALID_SEQUENCE);
    else
        return tiny_syntax_make_success_result(ast);
}

tiny_parser_t *asn_make_parser_sequence(uint8_t tag, tiny_parser_t *parser)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_sequence;
    ret->child = parser;

    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_t *asn_make_raw_parser_sequence(tiny_parser_t *parser)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_sequence;
    ret->child = parser;
    return ret;
}

static tiny_parser_result_t parser_asn_bytes(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    if (false) {
        puts("trying constructed");
        tiny_scanner_token_t *now = tiny_scanner_now(scanner);

        tiny_parser_t *parser = asn_make_raw_parser_sequence(ASN_PARSER_ANY);
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, parser, 0, ctx.length),
            scanner);
        free(parser);
        if (next.state == 0) return next;
        else tiny_scanner_reset(scanner, now);
        printf("tried constructed, %x\n", next.state);
    }

    uint8_t *str = malloc(sizeof(uint8_t) * (ctx.length + 1));
    if (!str) error("parser_asn_bytes failed");
    tiny_lex_token_t token;

    str[ctx.length] = 0;
    for (int i = 0; i < ctx.length; ++i)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) goto fail;
        str[i] = token.byte;
    }
    
    tiny_make_data_ast(ast, 0, AST_VALUE_STRING);
    ast->length = ctx.length;
    ast->value = str;
    return tiny_syntax_make_success_result(&ast->ast);
fail:
    free(str);
    return tiny_syntax_make_failure_result(token, 0, token.error);
}

tiny_parser_t *asn_make_parser_bytes(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_asn_bytes;
    ret->tag = tag;
    
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static int parse_int(const char *s, const char *e)
{
    int result = 0;
    for (const char *p = s; p < e; ++p) result = result * 10 + (*p - '0');
    return result;
}

static void parse_time(const char *time, bool shortYear, char *out)
{
    int year, month, day, hour, minute = 0, second = 0, microsecond = 0, d, zoneMonth = 0, zoneMinute = 0;
    if (shortYear)
    {
        year = parse_int(time, time + 2);
        if (year < 70) year += 2000;
        else year += 1900;
        time += 2;
    }
    else
    {
        year = parse_int(time, time + 4);
        time += 4;
    }
    month = parse_int(time, time + 2), time += 2;
    day = parse_int(time, time + 2), time += 2;
    hour = parse_int(time, time + 2), time += 2;
    if (*time) minute = parse_int(time, time + 2), time += 2;
    if (*time) second = parse_int(time, time + 2), time += 2;
    if (*time == '.' || *time == ',')
    {
        time++;
        int len;
        for (len = 0; len <= 3 && isdigit(*(time + len)); ++len);
        microsecond = parse_int(time, time + len), time += len;
    }
    if (*time == 'Z') d = 0, time++;
    else if (*time == '+') d = 1, time++;
    else if (*time == '-') d = -1, time++;

    if (*time && *(time + 1))
        zoneMonth = parse_int(time, time + 2), time += 2;
    if (*time && *(time + 1))
        zoneMinute = parse_int(time, time + 2), time += 2;

    out += sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    if (microsecond) out += sprintf(out, ".%04d", microsecond);
    if (d == 0) out += sprintf(out, " UTC");
    else out += sprintf(out, " GMT%c%02d:%02d", d > 0 ? '+' : '-', zoneMonth, zoneMinute);
}

static tiny_parser_result_t parser_asn_utc_time(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    char str[ctx.length + 1];
    tiny_lex_token_t token;

    str[ctx.length] = 0;
    for (int i = 0; i < ctx.length; ++i)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) goto fail;
        str[i] = token.byte;
    }

    char *result = malloc(128);
    if (!result) error("parser_asn_utc_time failed");
    parse_time(str, true, result);
    
    tiny_make_data_ast(ast, 0, AST_VALUE_STRING);
    ast->length = strlen(result);
    ast->value = result;
    return tiny_syntax_make_success_result(&ast->ast);
fail:
    return tiny_syntax_make_failure_result(token, 0, token.error);
}

tiny_parser_t *asn_make_parser_utc_time(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_asn_utc_time;
    ret->tag = tag;
    
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_asn_generalized_time(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    char str[ctx.length + 1];
    tiny_lex_token_t token;

    str[ctx.length] = 0;
    for (int i = 0; i < ctx.length; ++i)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) goto fail;
        str[i] = token.byte;
    }

    char *result = malloc(128);
    if (!result) error("parser_asn_generalized_time failed");
    parse_time(str, false, result);
    
    tiny_make_data_ast(ast, 0, AST_VALUE_STRING);
    ast->length = strlen(result);
    ast->value = result;
    return tiny_syntax_make_success_result(&ast->ast);
fail:
    return tiny_syntax_make_failure_result(token, 0, token.error);
}

tiny_parser_t *asn_make_parser_generalized_time(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_asn_generalized_time;
    ret->tag = tag;
    
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_asn_bits(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_lex_token_t token = tiny_scanner_next(scanner);
    if (token.error) return tiny_syntax_make_failure_result(token, 0, token.error);
    int unused_bits = token.byte;

    /*if (!unused_bits)
    {
        tiny_scanner_token_t *now = tiny_scanner_now(scanner);
        tiny_parser_t *parser = asn_make_raw_parser_sequence(ASN_PARSER_ANY);
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, parser, 0, ctx.length - 1),
            scanner);
        free(parser);
        if (next.state == 0) return next;
        else tiny_scanner_reset(scanner, now);
    }*/

    uint8_t *str = malloc(sizeof(uint8_t) * ctx.length);
    if (!str) error("parser_asn_bits failed");
    str[ctx.length] = 0;

    for (int i = 1; i < ctx.length; ++i)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) goto fail;
        str[i - 1] = token.byte;
    }
    
    tiny_make_data_ast(ast, 0, AST_VALUE_BIT_ARRAY);
    ast->padding = unused_bits;
    ast->length = ctx.length - 1;
    ast->value = str;
    return tiny_syntax_make_success_result(&ast->ast);
fail:
    free(str);
    return tiny_syntax_make_failure_result(token, 0, token.error);
}

tiny_parser_t *asn_make_parser_bits(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_asn_bits;
    ret->tag = tag;
    
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_asn_oid(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    char str[128];
    int bits = 0, n = 0, c = 0;
    tiny_lex_token_t token;
    for (int i = 0; i < ctx.length; ++i)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) return tiny_syntax_make_failure_result(token, 0, token.error);
        n = (n << 7) | (token.byte & 0x7F);
        bits += 7;
        if (!(token.byte & 0x80))
        {
            if (c == 0)
            {
                int m = n < 80 ? n < 40 ? 0 : 1 : 2;
                c += snprintf(str + c, 128 - c, "%d.%d", m, (n - m * 40));
            }
            else
            {
                c += snprintf(str + c, 128 - c, ".%d", n);
            }
            n = 0;
            bits = 0;
        }
    }

    void *oid = trie_search(OID_trie, str);
    if (oid)
    {
        tiny_make_data_ast(ast, ASN_TAG_OBJECT_IDENTIFIER, AST_VALUE_STRUCT);
        ast->value = oid;
        return tiny_syntax_make_success_result(&ast->ast);
    }
    else
    {
        return tiny_syntax_make_failure_result(token, 0, ASN_INVALID_OID);
    }
}

tiny_parser_t *asn_make_parser_oid(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_asn_oid;
    ret->tag = tag;
    
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_asn_integer(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_lex_token_t token = tiny_scanner_next(scanner);
    if (token.error) goto fail;
    int neg = token.byte & 0x80, pad = neg ? 0xFF : 0, i = 0, n;
    while (token.byte == pad && ++i < ctx.length)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) goto fail;
    }
    if (i >= ctx.length)
    {
        n = neg ? -1 : 0;
        goto success;
    }
    if (neg) n = (int)token.byte - 256;
    else n = token.byte;
    for (++i; i < ctx.length; ++i)
    {
        token = tiny_scanner_next(scanner);
        if (token.error) goto fail;
        n = (n << 8) | token.byte;
    }
success:
    {
        tiny_make_int_ast(ast, ASN_TAG_INTEGER, AST_VALUE_INTEGRAL);
        ast->value = n;
        return tiny_syntax_make_success_result(&ast->ast);
    }
fail:
    return tiny_syntax_make_failure_result(token, 0, ASN_INVALID_OID);
}

tiny_parser_t *asn_make_parser_integer(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_asn_integer;
    ret->tag = tag;

    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_match_sequence(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_ast_t *ast = tiny_make_ast(ASN_TAG_SEQUENCE);
    tiny_scanner_token_t *save = tiny_scanner_now(scanner);

    for (tiny_parser_t *cld = ctx.current_parser->child; cld; cld = cld->sibling)
    {
        tiny_parser_result_t next = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, cld, 0, -1),
            scanner);

        if (next.state == 0)
        {
            tiny_ast_add_child(ast, next.ast);
        }
        else
        {
            tiny_free_ast(ast);
            return next;
        }
    }

    if (tiny_scanner_diff(scanner, save) != ctx.length)
        return tiny_syntax_make_failure_result(save->token, 0, ASN_INVALID_SEQUENCE);
    else
        return tiny_syntax_make_success_result(ast);
}

tiny_parser_t *asn_make_parser_match_sequence(uint8_t tag, int n, ...)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_match_sequence;
    tiny_parser_t **ptr = &ret->child;

    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; ++i)
    {
        *ptr = va_arg(ap, tiny_parser_t *);
        ptr = &((*ptr)->sibling);
    }
    
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_skip(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_ast_t *ast = tiny_make_ast(ctx.tag);
    tiny_scanner_token_t *save = tiny_scanner_now(scanner);

    while (tiny_scanner_diff(scanner, save) < ctx.length)
    {
        tiny_lex_token_t token = tiny_scanner_next(scanner);
        if (token.error) return tiny_syntax_make_failure_result(token, 0, token.error);
    }

    return tiny_syntax_make_success_result(ast);
}

tiny_parser_t *asn_make_parser_skip(uint8_t tag)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_skip;
    return asn_make_parser_tlv(ASN_TAG_CLASS_UNIVERSAL, tag, ret);
}

static tiny_parser_result_t parser_tlv(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner)
{
    tiny_scanner_token_t *save = tiny_scanner_now(scanner);
    // simple tag implmentation
    asn_tag_t tag = parse_asn_tag(scanner);
    if (tag.number < 0 || (ctx.current_parser->tag && ctx.current_parser->tag != tag.number))
    {
        return tiny_syntax_make_failure_result(tag.token, ctx.current_parser->tag, ASN_INVALID_TAG);
    }
    if (ctx.current_parser->class >= 0 && ctx.current_parser->class != tag.class)
    {
        return tiny_syntax_make_failure_result(tag.token, ctx.current_parser->tag, ASN_INVALID_TAG_CLASS);
    }

    tiny_lex_token_t length_byte = tiny_scanner_next(scanner);
    if (length_byte.error)
    {
        return tiny_syntax_make_failure_result(length_byte, 0, ASN_INVALID_LENGTH);
    }
    size_t length = 0;
    if (length_byte.byte >= 0x80)
    {
        length_byte.byte &= 0x7f;
        for (int i = 0; i < length_byte.byte; ++i)
        {
            tiny_lex_token_t token = tiny_scanner_next(scanner);
            if (token.error) return tiny_syntax_make_failure_result(token, 0, ASN_INVALID_LENGTH);
            // big-endian
            length = (length << 8) | token.byte;
        }
    }
    else
    {
        length = length_byte.byte;
    }

    // printf("%x %x %x %ld %p\n", tag.class, tag.constructed, tag.number, length, ctx.current_parser->child);

    tiny_parser_result_t result;

    if (!ctx.current_parser->child)
    {
        if (tag.class == 0)
        {
            tiny_scanner_reset(scanner, save);
            result = tiny_syntax_parse(
                tiny_syntax_make_context(ctx.parsers, find_parser_by_tag(tag.number), tag.number, length),
                scanner);
        }
        else if (tag.constructed)
        {
            tiny_parser_t *parser = asn_make_raw_parser_sequence(ASN_PARSER_ANY);
            result = tiny_syntax_parse(
                tiny_syntax_make_context(ctx.parsers, parser, tag.number, length),
                scanner);
            free(parser);
        }
        else
        {  
            tiny_parser_t *ret = tiny_make_parser();
            ret->parser = parser_asn_bytes;
            ret->tag = tag.number;
            result = tiny_syntax_parse(
                tiny_syntax_make_context(ctx.parsers, ret, tag.number, length),
                scanner);
            free(ret);
        }
    }
    else
    {
        result = tiny_syntax_parse(
            tiny_syntax_make_context(ctx.parsers, ctx.current_parser->child, tag.number, length),
            scanner);
    }
    
    if (result.ast)
    {
        result.ast->class = tag.class;
        result.ast->constructed = tag.constructed;
        result.ast->tag = tag.number;
    }
    return result;
}

tiny_parser_t *asn_make_parser_tlv(int class, uint8_t tag, tiny_parser_t *child)
{
    tiny_parser_t *ret = tiny_make_parser();
    ret->parser = parser_tlv;
    ret->class = class;
    ret->tag = tag;
    ret->child = child;
    return ret;
}

void asn_parser_init()
{
    ASN_PARSER_ANY = asn_make_parser_tlv(-1, 0, NULL);
}
