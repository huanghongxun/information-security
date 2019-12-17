#ifndef ASN_H
#define ASN_H

#include "parser.h"

#define ASN_TAG_CLASS_NO_MATTER     -1
#define ASN_TAG_CLASS_UNIVERSAL     0
#define ASN_TAG_CLASS_APPLICATION   1
#define ASN_TAG_CLASS_CONTEXT       2
#define ASN_TAG_CLASS_PRIVATE       3

#define ASN_TAG_EOC                 0x00
#define ASN_TAG_BOOLEAN             0x01
#define ASN_TAG_INTEGER             0x02
#define ASN_TAG_BIT_STRING          0x03
#define ASN_TAG_OCTET_STRING        0x04
#define ASN_TAG_NULL                0x05
#define ASN_TAG_OBJECT_IDENTIFIER   0x06
#define ASN_TAG_OBJECT_DESCRIPTOR   0x07
#define ASN_TAG_EXTERNAL            0x08
#define ASN_TAG_REAL                0x09
#define ASN_TAG_ENUMERATED          0x0A
#define ASN_TAG_EMBEDDED_PDV        0x0B
#define ASN_TAG_UTF8_STRING         0x0C
#define ASN_TAG_SEQUENCE            0x10
#define ASN_TAG_SET                 0x11
#define ASN_TAG_NUMERIC_STRING      0x12
#define ASN_TAG_PRINTABLE_STRING    0x13
#define ASN_TAG_TELETEX_STRING      0x14
#define ASN_TAG_VIDEOTEX_STRING     0x15
#define ASN_TAG_IA5_STRING          0x16
#define ASN_TAG_UTC_TIME            0x17
#define ASN_TAG_GENERALIZED_TIME    0x18
#define ASN_TAG_GRAPHIC_STRING      0x19
#define ASN_TAG_VISIBLE_STRING      0x1A
#define ASN_TAG_GENERAL_STRING      0x1B
#define ASN_TAG_UNIVERSAL_STRING    0x1C
#define ASN_TAG_BMP_STRING          0x1D

#define ASN_PARSER_EOC                  asn_make_parser_skip(ASN_TAG_EOC)
#define ASN_PARSER_BOOLEAN              asn_make_parser_integer(ASN_TAG_BOOLEAN)
#define ASN_PARSER_INTEGER              asn_make_parser_integer(ASN_TAG_INTEGER)
#define ASN_PARSER_BIG_INTEGER          asn_make_parser_bytes(ASN_TAG_INTEGER)
#define ASN_PARSER_BIT_STRING           asn_make_parser_bits(ASN_TAG_BIT_STRING)
#define ASN_PARSER_OCTET_STRING         asn_make_parser_bytes(ASN_TAG_OCTET_STRING)
#define ASN_PARSER_NULL                 asn_make_parser_skip(ASN_TAG_NULL)
#define ASN_PARSER_OBJECT_IDENTIFIER    asn_make_parser_oid(ASN_TAG_OBJECT_IDENTIFIER)
#define ASN_PARSER_OBJECT_DESCRIPTOR    asn_make_parser_skip(ASN_TAG_OBJECT_DESCRIPTOR)
#define ASN_PARSER_EXTERNAL             asn_make_parser_skip(ASN_TAG_EXTERNAL)
#define ASN_PARSER_REAL                 asn_make_parser_skip(ASN_TAG_REAL)
#define ASN_PARSER_ENUMERATED           asn_make_parser_skip(ASN_TAG_ENUMERATED)
#define ASN_PARSER_EMBEDDED_PDV         asn_make_parser_skip(ASN_TAG_EMBEDDED_PDV)
#define ASN_PARSER_UTF8_STRING          asn_make_parser_skip(ASN_TAG_UTF8_STRING)
#define ASN_PARSER_MATCH_SEQUENCE(...)  asn_make_parser_match_sequence(ASN_TAG_SEQUENCE, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define ASN_PARSER_MATCH_SET(...)       asn_make_parser_match_sequence(ASN_TAG_SET, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define ASN_PARSER_SEQUENCE             asn_make_parser_sequence(ASN_TAG_SEQUENCE, ASN_PARSER_ANY)
#define ASN_PARSER_SET                  asn_make_parser_sequence(ASN_TAG_SET, ASN_PARSER_ANY)
#define ASN_PARSER_SEQUENCE_OF(parser)  asn_make_parser_sequence(ASN_TAG_SEQUENCE, parser)
#define ASN_PARSER_SET_OF(parser)       asn_make_parser_sequence(ASN_TAG_SET, parser)
#define ASN_PARSER_NUMERIC_STRING       asn_make_parser_bytes(ASN_TAG_NUMERIC_STRING)
#define ASN_PARSER_PRINTABLE_STRING     asn_make_parser_bytes(ASN_TAG_PRINTABLE_STRING)
#define ASN_PARSER_TELETEX_STRING       asn_make_parser_bytes(ASN_TAG_TELETEX_STRING)
#define ASN_PARSER_VIDEOTEX_STRING      asn_make_parser_bytes(ASN_TAG_VIDEOTEX_STRING)
#define ASN_PARSER_IA5_STRING           asn_make_parser_bytes(ASN_TAG_IA5_STRING)
#define ASN_PARSER_UTC_TIME             asn_make_parser_utc_time(ASN_TAG_UTC_TIME)
#define ASN_PARSER_GENERALIZED_TIME     asn_make_parser_generalized_time(ASN_TAG_GENERALIZED_TIME)
#define ASN_PARSER_GRAPHIC_STRING       asn_make_parser_bytes(ASN_TAG_GRAPHIC_STRING)
#define ASN_PARSER_VISIBLE_STRING       asn_make_parser_bytes(ASN_TAG_VISIBLE_STRING)
#define ASN_PARSER_GENERAL_STRING       asn_make_parser_bytes(ASN_TAG_GENERAL_STRING)
#define ASN_PARSER_UNIVERSAL_STRING     asn_make_parser_bytes(ASN_TAG_UNIVERSAL_STRING)
#define ASN_PARSER_BMP_STRING           asn_make_parser_skip(ASN_TAG_BMP_STRING)
#define ASN_PARSER_TLV(class, number, parser) asn_make_parser_tlv(class, number, parser)

#define ASN_INVALID_TAG_CLASS       0x80009
#define ASN_INVALID_OID             0x8000A
#define ASN_INVALID_TAG             0x8000B
#define ASN_INVALID_LENGTH          0x8000C
#define ASN_INVALID_BIT_STRING      0x8000D
#define ASN_INVALID_SEQUENCE        0x8000E
#define ASN_INVALID_UNUSED_BITS     0x8000F

typedef struct asn_tag_s
{
    int class;
    bool constructed;
    int number;

    tiny_lex_token_t token;
} asn_tag_t;

tiny_parser_t *asn_make_parser_skip(uint8_t tag);
tiny_parser_t *asn_make_parser_bytes(uint8_t tag);
tiny_parser_t *asn_make_parser_utc_time(uint8_t tag);
tiny_parser_t *asn_make_parser_generalized_time(uint8_t tag);
tiny_parser_t *asn_make_parser_bits(uint8_t tag);
tiny_parser_t *asn_make_parser_oid(uint8_t tag);
tiny_parser_t *asn_make_parser_integer(uint8_t tag);
tiny_parser_t *asn_make_parser_sequence(uint8_t tag, tiny_parser_t *);
tiny_parser_t *asn_make_parser_match_sequence(uint8_t tag, int n, ...);
tiny_parser_t *asn_make_parser_tlv(int class, uint8_t tag, tiny_parser_t *);

void asn_parser_init();
const char *find_parser_name_by_tag(uint8_t tag);

extern tiny_parser_t *ASN_PARSER_ANY;

#endif // ASN_H