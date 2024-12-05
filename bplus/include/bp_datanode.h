#ifndef BP_DATANODE_H
#define BP_DATANODE_H

#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>

// Define the structure for a data node in the B+ tree
typedef struct {
    bool is_leaf;
    int key_count;
    int keys[MAX_KEYS];
    Record records[MAX_KEYS];
    int next_block;
} BPLUS_DATA_NODE;

// Initialize a B+ data node
void initialize_data_node(BPLUS_DATA_NODE *node);

// Insert a record into a leaf node
bool insert_into_leaf(BPLUS_DATA_NODE *leaf, Record record);

// Find a record in a leaf node
Record* find_in_leaf(BPLUS_DATA_NODE *leaf, int id);

#endif