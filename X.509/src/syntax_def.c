#include "syntax_def.h"
#include "parser.h"
#include "asn.h"
#include <ctype.h>
#include "string_util.h"

#define DEFINE(name, parser)            \
    {                                   \
        tiny_parser_t *p = parser;      \
        trie_insert(parsers, #name, p); \
    }

struct trie *prepare_parsers()
{
    struct trie *parsers = trie_create();

    DEFINE(
        Certificate,
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(TBSCertificate),
            GRAMMAR(AlgorithmIdentifier),
            ASN_PARSER_BIT_STRING));

    DEFINE(
        TBSCertificate,
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(Version),
            GRAMMAR(CertificateSerialNumber),
            GRAMMAR(AlgorithmIdentifier),
            GRAMMAR(Name),
            GRAMMAR(Validity),
            GRAMMAR(Name),
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
        ASN_PARSER_TLV(
            ASN_TAG_CLASS_CONTEXT,
            0, // [0]
            ASN_PARSER_INTEGER));

    DEFINE(
        CertificateSerialNumber,
        ASN_PARSER_INTEGER);

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
        ASN_PARSER_OBJECT_IDENTIFIER);

    DEFINE(
        Validity,
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(Time),
            GRAMMAR(Time)));

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
        ASN_PARSER_MATCH_SEQUENCE(
            GRAMMAR(AlgorithmIdentifier), // algorithm
            ASN_PARSER_BIT_STRING         // subjectPublicKey
            ));

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
