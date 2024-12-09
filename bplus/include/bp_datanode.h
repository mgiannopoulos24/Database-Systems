#ifndef BP_DATANODE_H
#define BP_DATANODE_H

#include <stdbool.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>

#define MAX_KEYS 4

typedef struct bplus_data_node {
    Record records[MAX_KEYS];
    struct bplus_data_node *children[MAX_KEYS + 1];         // ο πίνακας με τα παιδιά που έχει
    int n;                                                  // τρέχον αριθμός κλειδιών
    bool leaf;
    struct bplus_data_node *next;                           // δείκτης στο επόμενο leaf.
} BPLUS_DATA_NODE;

#endif 