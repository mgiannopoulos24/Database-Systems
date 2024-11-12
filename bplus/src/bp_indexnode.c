#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"

#include "bp_indexnode.h"

// Βρίσκει τον δείκτη του επόμενου μπλοκ για τη διαδρομή στο B+ δέντρο
int find_next_block(BPLUS_INDEX_NODE *node, int id) {
    int i = 0;
    while (i < node->key_count && id >= node->keys[i]) {
        i++;
    }
    return node->children[i];
}