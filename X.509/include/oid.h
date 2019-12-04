#ifndef OID_H
#define OID_H

#include "trie.h"

extern const char *OID[][3];

// OID_trie: map<string, const char *(*)[3]>
// from OID key to OID entry
extern struct trie *OID_trie;

void OID_init_trie();

#endif // OID_H