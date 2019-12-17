#ifndef OID_H
#define OID_H

#include "trie.h"

typedef const char *(OID_entry_t)[3];
typedef OID_entry_t *OID_entry_p;

extern OID_entry_t OID[];

// OID_trie: map<string, OID_entry_t *>
// from OID key to OID entry
extern struct trie *OID_trie;

void OID_init_trie();

#endif // OID_H