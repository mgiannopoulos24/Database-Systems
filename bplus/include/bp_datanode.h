#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>



typedef struct {
    bool is_leaf;
    int key_count;
    int keys[MAX_KEYS]; // Υποθετικός μέγιστος αριθμός κλειδιών
    Record records[MAX_KEYS]; // Αποθήκευση εγγραφών για φύλλα
    int next_block; // Δείκτης στον επόμενο κόμβο
} BPLUS_DATA_NODE;

// Εισάγει εγγραφή σε φύλλο κόμβο
bool insert_into_leaf(BPLUS_DATA_NODE *leaf, Record record);

// Αναζητά εγγραφή σε φύλλο κόμβο
Record* find_in_leaf(BPLUS_DATA_NODE *leaf, int id);

#endif