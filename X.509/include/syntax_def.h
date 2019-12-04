#ifndef SYNTAX_DEF_H
#define SYNTAX_DEF_H

#include "trie.h"

#define X509_TAG_EOC               0x00
#define X509_TAG_BOOLEAN           0x01
#define X509_TAG_INTEGER           0x02
#define X509_TAG_BIT_STRING        0x03
#define X509_TAG_OCTET_STRING      0x04
#define X509_TAG_NULL              0x05
#define X509_TAG_OBJECT_IDENTIFIER 0x06
#define X509_TAG_OBJECT_DESCRIPTOR 0x07
#define X509_TAG_EXTERNAL          0x08
#define X509_TAG_REAL              0x09
#define X509_TAG_ENUMERATED        0x0A
#define X509_TAG_EMBEDDED_PDV      0x0B
#define X509_TAG_UTF8_STRING       0x0C
#define X509_TAG_SEQUENCE          0x10
#define X509_TAG_SET               0x11
#define X509_TAG_NUMERIC_STRING    0x12
#define X509_TAG_PRINTABLE_STRING  0x13
#define X509_TAG_TELETEX_STRING    0x14 // aka T61String
#define X509_TAG_VIDEOTEX_STRING   0x15
#define X509_TAG_IA5_STRING        0x16
#define X509_TAG_UTC_TIME          0x17
#define X509_TAG_GENERALIZED_TIME  0x18
#define X509_TAG_GRAPHIC_STRING    0x19
#define X509_TAG_VISIBLE_STRING    0x1A
#define X509_TAG_GENERAL_STRING    0x1B
#define X509_TAG_UNIVERSAL_STRING  0x1C
#define X509_TAG_BMP_STRING        0x1D



struct trie *prepare_parsers();

#endif // SYNTAX_DEF_H