#include "x509.h"
#include "parser.h"
#include "asn.h"
#include "oid.h"
#include <ctype.h>
#include "string_util.h"

#define DEFINE(name, parser)            \
    {                                   \
        tiny_parser_t *p = parser;      \
        trie_insert(parsers, #name, p); \
    }

static int indent = 0;

static void print_indent()
{
    for (int i = 0; i < indent; ++i)
        printf("  ");
}

static void version_action(tiny_ast_t *ast)
{
    print_indent();
    printf("Version: V%d\n", ast2int_ast(ast)->value + 1);
}

static void certificate_serial_number_action(tiny_ast_t *ast)
{
    print_indent();
    printf("Serial Number: ");
    tiny_data_ast_t *data_ast = ast2data_ast(ast);
    uint8_t *bytes = data_ast->value;
    for (int i = 0; i < data_ast->length; ++i)
    {
        if (i > 0) putchar(' ');
        printf("%02X", bytes[i]);
    }
    putchar('\n');
}

static void algorithm_identifier_action(tiny_ast_t *ast)
{
    tiny_data_ast_t *data_ast = ast2data_ast(ast->child);
    const char *(*oid)[3] = data_ast->value;
    printf("%s\n", (*oid)[1]);
}

static void signature_action()
{
    print_indent();
    printf("Certificate Algorithm: ");
}

static void tbs_signature_action()
{
    print_indent();
    printf("TBS Certificate Algorithm: ");
}

static void not_before_action(tiny_ast_t *ast)
{
    print_indent();
    tiny_data_ast_t *data_ast = ast2data_ast(ast);
    char *string = data_ast->value;
    printf("Not Before: %s\n", string);
}

static void not_after_action(tiny_ast_t *ast)
{
    print_indent();
    tiny_data_ast_t *data_ast = ast2data_ast(ast);
    char *string = data_ast->value;
    printf("Not After: %s\n", string);
}

static void issuer_action()
{
    print_indent();
    printf("Issuer: ");
}

static void subject_action()
{
    print_indent();
    printf("Subject: ");
}

static void subject_public_key_preaction()
{
    print_indent();
    printf("Subject Public Key:\n");
    indent++;
    print_indent();
    printf("Algorithm: ");
}

static void subject_public_key_postaction(tiny_ast_t *ast)
{
    indent--;
}

static void public_key_action()
{
    print_indent();
    printf("Public key: ");
}

static void bit_string_action(tiny_ast_t *ast)
{
    tiny_data_ast_t *data_ast = ast2data_ast(ast);
    uint8_t *bytes = data_ast->value;
    if (data_ast->padding == 0)
    {
        for (int i = 0; i < data_ast->length; ++i)
        {
            if (i > 0) putchar(' ');
            printf("%02X", bytes[i]);
        }
    }
    else
    {
        for (int i = 0; i < data_ast->length; ++i)
        {
            int skip = i + 1 == data_ast->length ? data_ast->padding : 0;
            for (int j = 7; j >= skip; --j)
                printf("%d", (bytes[i] >> j) & 1);
        }
    }
    putchar('\n');
}

static void name_action(tiny_ast_t *ast)
{
    putchar('\n');
    for (tiny_ast_t *rdn = ast->child; rdn; rdn = rdn->sibling)
    {
        for (tiny_ast_t *attr = rdn->child; attr; attr = attr->sibling)
        {
            tiny_data_ast_t *oid_ast = ast2data_ast(attr->child);
            tiny_data_ast_t *str_ast = ast2data_ast(attr->child->sibling);
            OID_entry_t *oid = oid_ast->value;
            const char *value = str_ast->value;
            print_indent();
            if (attr->child->sibling->value_type == AST_VALUE_STRING)
                printf("  %s=%s\n", (*oid)[1], value);
            else
                printf("  %s=?\n", (*oid)[1]);
        }
    }
}

static void reduce_indent()
{
    indent -= 2;
}

struct trie *prepare_parsers()
{
    struct trie *parsers = trie_create();

    DEFINE(
        Certificate,
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(TBSCertificate),
            ACTION(signature_action, GRAMMAR(AlgorithmIdentifier), algorithm_identifier_action),
            ASN_PARSER_BIT_STRING));

    DEFINE(
        TBSCertificate,
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(Version),
            GRAMMAR(CertificateSerialNumber),
            ACTION(tbs_signature_action, GRAMMAR(AlgorithmIdentifier), algorithm_identifier_action),
            ACTION(issuer_action, GRAMMAR(Name), name_action),
            GRAMMAR(Validity),
            ACTION(subject_action, GRAMMAR(Name), name_action),
            GRAMMAR(SubjectPublicKeyInfo),
            OPTIONAL( // issuerUniqueID
                ASN_PARSER_TLV(
                    ASN_TAG_CLASS_CONTEXT,
                    1,
                    GRAMMAR(UniqueIdentifier))),
            OPTIONAL( // subjectUniqueID
                ASN_PARSER_TLV(
                    ASN_TAG_CLASS_CONTEXT,
                    2,
                    GRAMMAR(UniqueIdentifier))),
            OPTIONAL(
                ASN_PARSER_TLV(
                    ASN_TAG_CLASS_CONTEXT,
                    3,
                    GRAMMAR(Extensions)))));

    DEFINE(
        Version,
        POST_ACTION(
            ASN_PARSER_TLV(
                ASN_TAG_CLASS_CONTEXT,
                0, // [0]
                ASN_PARSER_INTEGER),
            version_action));

    DEFINE(
        CertificateSerialNumber,
        POST_ACTION(
            ASN_PARSER_BIG_INTEGER,
            certificate_serial_number_action));

    DEFINE(
        AlgorithmIdentifier,
        ASN_PARSER_MATCH_SEQUENCE(
            ASN_PARSER_OBJECT_IDENTIFIER,           // algorithm
            OR(GRAMMAR(DssParams), ASN_PARSER_NULL) // parameters
        ));

    DEFINE(
        DssParams,
        ASN_PARSER_MATCH_SEQUENCE(
            ASN_PARSER_INTEGER, // r
            ASN_PARSER_INTEGER  // s
            ));

    DEFINE(
        Name,
        GRAMMAR(RDNSequence));

    DEFINE(
        RDNSequence,
        ASN_PARSER_SEQUENCE_OF(GRAMMAR(RelativeDistinguishedName)));

    DEFINE(
        RelativeDistinguishedName,
        ASN_PARSER_SET_OF(GRAMMAR(AttributeTypeAndValue)));

    DEFINE(
        AttributeTypeAndValue,
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(AttributeType), // type
            GRAMMAR(AttributeValue) // value
            ));

    DEFINE(
        AttributeType,
        ASN_PARSER_OBJECT_IDENTIFIER);

    DEFINE(
        AttributeValue,
        ASN_PARSER_ANY);

    DEFINE(
        Validity,
        ASN_PARSER_MATCH_SEQUENCE(
            POST_ACTION(GRAMMAR(Time), not_before_action),
            POST_ACTION(GRAMMAR(Time), not_after_action)));

    DEFINE(
        Time,
        OR(
            ASN_PARSER_UTC_TIME,
            ASN_PARSER_GENERALIZED_TIME));

    DEFINE(
        UniqueIdentifier,
        ASN_PARSER_BIT_STRING);

    DEFINE(
        SubjectPublicKeyInfo,
        ACTION(
            subject_public_key_preaction,
            ASN_PARSER_MATCH_SEQUENCE(
                POST_ACTION(GRAMMAR(AlgorithmIdentifier), algorithm_identifier_action), // algorithm
                ACTION(                                                                 // subjectPublicKey
                    public_key_action,
                    ASN_PARSER_BIT_STRING,
                    bit_string_action)),
            subject_public_key_postaction));

    DEFINE(
        RSAPublicKey,
        ASN_PARSER_MATCH_SEQUENCE(
            ASN_PARSER_INTEGER, // modulus
            ASN_PARSER_INTEGER  // publicExponent
            ));
    
    DEFINE(
        Extensions,
        ASN_PARSER_SEQUENCE_OF(GRAMMAR(Extension))
    );
    
    DEFINE(
        Extension,
        ASN_PARSER_MATCH_SEQUENCE(
            ASN_PARSER_OBJECT_IDENTIFIER,
            OPTIONAL(ASN_PARSER_BOOLEAN),
            ASN_PARSER_OCTET_STRING
        )
    );

    return parsers;
}
