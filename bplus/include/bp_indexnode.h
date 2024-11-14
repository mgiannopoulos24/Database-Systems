#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H
#include <record.h>
#include <bf.h>
#include <bp_file.h>

#define MAX_KEYS 4
#define bplus_ERROR -1

typedef struct {
    bool is_leaf;
    int key_count;
    int keys[MAX_KEYS]; // Κλειδιά για τον κόμβο ευρετηρίου
    int children[MAX_KEYS + 1]; // Δείκτες σε παιδιά κόμβους
} BPLUS_INDEX_NODE;

// Βρίσκει τον επόμενο κόμβο για τη διαδρομή στο B+ δέντρο
int find_next_block(BPLUS_INDEX_NODE *node, int id);

#endif