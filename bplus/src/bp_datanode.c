#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"

#include "bp_datanode.h"

// Εισάγει εγγραφή σε φύλλο κόμβο
bool insert_into_leaf(BPLUS_DATA_NODE *leaf, Record record) {
    if (leaf->key_count >= MAX_KEYS) {
        return false; // Ο κόμβος είναι γεμάτος
    }
    // Εισαγωγή της εγγραφής με ταξινόμηση κατά id
    int i;
    for (i = leaf->key_count - 1; i >= 0 && leaf->keys[i] > record.id; i--) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->records[i + 1] = leaf->records[i];
    }
    leaf->keys[i + 1] = record.id;
    leaf->records[i + 1] = record;
    leaf->key_count++;
    return true;
}

// Αναζητά εγγραφή σε φύλλο κόμβο
Record* find_in_leaf(BPLUS_DATA_NODE *leaf, int id) {
    for (int i = 0; i < leaf->key_count; i++) {
        if (leaf->keys[i] == id) {
            return &leaf->records[i];
        }
    }
    return NULL; // Δεν βρέθηκε
}