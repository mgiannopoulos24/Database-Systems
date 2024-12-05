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
        return false; // Node is full
    }
    int i;
    for (i = leaf->key_count - 1; i >= 0 && leaf->keys[i] > record.id; i--) {
        leaf->keys[i + 1] = leaf->keys[i];
        memcpy(&leaf->records[i + 1], &leaf->records[i], sizeof(Record));
    }
    leaf->keys[i + 1] = record.id;
    memcpy(&leaf->records[i + 1], &record, sizeof(Record));
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
    return NULL;
}

// Helper function to initialize a B+ data node
void initialize_data_node(BPLUS_DATA_NODE *node) {
    node->is_leaf = true;
    node->key_count = 0;
    node->next_block = -1; // No next block initially
    memset(node->keys, 0, sizeof(node->keys));
    memset(node->records, 0, sizeof(node->records));
}