#ifndef BP_DATANODE_H
#define BP_DATANODE_H

#include <stdbool.h>
#include "record.h"
#include "bp_file.h"
#include "bp_indexnode.h"

#define MAX_NODES 32768                                     // Μέγιστος αριθμός κόμβων που μπορούν να αποθηκευτούν
#define MAX_KEYS 4                                          // Μέγιστος αριθμός κλειδιών κάθε κόμβου

typedef struct bplus_info BPLUS_INFO;                       // Δήλωση της δομής για την insert

typedef struct bplus_data_node {
    Record records[MAX_KEYS];
    struct bplus_data_node *children[MAX_KEYS + 1];         // ο πίνακας με τα παιδιά που έχει
    int n;                                                  // τρέχον αριθμός κλειδιών
    bool leaf;
    struct bplus_data_node *next;                           // δείκτης στο επόμενο leaf
} BPLUS_DATA_NODE;

// Επιστρέφει ένα νέο κόμβο με άδειες πληροφορίες και 
// την πληροφορία αν είναι φύλλο ή όχι ή NULL στην περίπτωηση 
// που το δέντρο έχει παραπάνω κόμβους από MAX_NODES.
BPLUS_DATA_NODE *create_node(bool is_leaf);

// Αναζήτηση εγγραφής με βάση το id.
// Επιστρέφει την εγγραφή αν υπάρχει το συγκεκριμένο
// id, διαφορετικά επιστρέφει μία εγγραφή με id = -1.
Record search(BPLUS_DATA_NODE *node, int id);

// Εισαγωγή εγγραφής στο δέντρο.
// Επιστρέφει: 
// 0 σε περίπτωση που το id της εγγραφής υπάρχει
// 1 όταν η εισαγωγή της εγγραφής γίνεται με επιτυχία
// -1 σε περίπτωση αποτυχίας της εισαγωγής.
int insert(BPLUS_INFO *info, Record record);

// Εμφάνιση όλων των εγγραφών του δέντρου
void display(BPLUS_DATA_NODE *node);

// Καταστροφή όλων των κόμβων του δέντρου
void destroy(BPLUS_DATA_NODE *node);

#endif 