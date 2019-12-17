#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "scanner.h"
#include "trie.h"
#include "ast.h"
#include "defs.h"

// 克林闭包，表示 replica*
#define KLEENE(replica) tiny_make_parser_kleene(replica)
// 克林闭包，匹配到 terminator，表示 replica*
#define KLEENE_UNTIL(terminator, replica) tiny_make_parser_kleene_until(terminator, replica)
// 或，表示 a | b | c | ... | ...
#define OR(...) tiny_make_parser_or(PP_NARG(__VA_ARGS__), __VA_ARGS__)
// 表示引用产生式，如引用特定的产生式 S
#define GRAMMAR(name) tiny_make_parser_grammar(#name)
// 可选，表示 [optional]
#define OPTIONAL(optional) tiny_make_parser_optional(optional)
// 匹配 EOF
#define TOKEN_EOF tiny_make_parser_token_eof()
// 匹配指定字符
#define TOKEN_PREDICATE(predicate) tiny_make_parser_token_predicate(predicate)
// 在产生 AST 后执行操作
#define POST_ACTION(parser, action) tiny_make_parser_postaction(parser, action)
// 在产生 AST 前执行操作
#define PRE_ACTION(parser, action) tiny_make_parser_preaction(parser, action)
// 在产生 AST 前后执行操作
#define ACTION(preaction, parser, postaction) POST_ACTION(PRE_ACTION(parser, preaction), postaction)
// 表示丢弃产生式 parser 所产生的语法树
#define ELIMINATE(parser) tiny_make_parser_eliminate(parser)

struct tiny_parser_token_seq_s {
    tiny_lex_token_t token;

    struct tiny_parser_token_seq_s *next;
};

struct tiny_parser_s;

struct tiny_parser_result_s {
    tiny_ast_t *ast;
    int state;
    bool fatal;
    tiny_lex_token_t error_token;
    uint8_t required_token;
};

struct tiny_parser_ctx_s {
    struct trie *parsers;
    struct tiny_parser_s *current_parser;
    int tag;
    bool constructed;
    int length;
};

struct tiny_parser_s {
    struct tiny_parser_result_s (*parser)(struct tiny_parser_ctx_s, tiny_scanner_t *);
    struct tiny_parser_s *sibling;
    struct tiny_parser_s *child;
    const char *name;
    int class;
    int tag;
    int error;
    void *func;
};

typedef struct tiny_parser_result_s tiny_parser_result_t;
typedef struct tiny_parser_ctx_s tiny_parser_ctx_t;
typedef struct tiny_parser_s tiny_parser_t;

tiny_parser_t *tiny_make_parser();
tiny_parser_t *tiny_make_parser_kleene(tiny_parser_t *replica);
tiny_parser_t *tiny_make_parser_kleene_until(tiny_parser_t *terminator, tiny_parser_t *replica);
tiny_parser_t *tiny_make_parser_or(int n, ...);
tiny_parser_t *tiny_make_parser_grammar(const char *name);
tiny_parser_t *tiny_make_parser_optional(tiny_parser_t *optional);
tiny_parser_t *tiny_make_parser_token_eof();
tiny_parser_t *tiny_make_parser_token_predicate(bool (*predicate)(uint8_t));
tiny_parser_t *tiny_make_parser_preaction(tiny_parser_t *parser, void (*action)());
tiny_parser_t *tiny_make_parser_postaction(tiny_parser_t *parser, void (*action)(tiny_ast_t *));
tiny_parser_t *tiny_make_parser_eliminate(tiny_parser_t *parser);

void tiny_syntax_next_token(tiny_parser_ctx_t *machine, tiny_lex_token_t token);
tiny_parser_ctx_t tiny_syntax_make_context(struct trie *parsers, tiny_parser_t *current_parser, int tag, int length);
tiny_parser_result_t tiny_syntax_make_success_result(tiny_ast_t *ast);
tiny_parser_result_t tiny_syntax_make_failure_result(tiny_lex_token_t token, uint8_t required_token, int error);
tiny_parser_result_t tiny_syntax_parse(tiny_parser_ctx_t ctx, tiny_scanner_t *scanner);

#endif // PARSER_H