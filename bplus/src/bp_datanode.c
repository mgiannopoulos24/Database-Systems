#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"

static BPLUS_DATA_NODE node_pool[MAX_NODES];    // Προκαθορισμένη δεξαμενή κόμβων
static int node_pool_index = 0;                 // Δείκτης για την τρέχουσα θέση στη δεξαμενή

// Συνάρτηση που επιστρέφει έναν νέο κόμβο από τη δεξαμενή
static BPLUS_DATA_NODE *allocate_node()
{
    return node_pool_index < MAX_NODES ? &node_pool[node_pool_index++] : NULL;
}

// Δημιουργία εγγραφής με τις παρεχόμενες τιμές
static Record set_record(int id, char *name, char *surname, char *city)
{
    Record record = { .id = id };
    strcpy(record.name, name);
    strcpy(record.surname, surname);
    strcpy(record.city, city);

    return record;
}

// Διαχωρισμός κόμβου όταν γεμίσει
static int split_child(BPLUS_DATA_NODE *parent, int i, BPLUS_DATA_NODE *child)
{
    BPLUS_DATA_NODE *new_child = create_node(child->leaf);
    if (new_child == NULL)
        return 0;                                                   // Επιστροφή 0 αν η δημιουργία αποτύχει

    int mid = child->n / 2;                                         // Βρίσκεται το μέσο σημείο για διαχωρισμό

    // Μεταφορά των εγγραφών στο νέο κόμβο
    new_child->n = child->n - mid - 1;
    memcpy(new_child->records, &child->records[mid + 1], new_child->n * sizeof(Record));

    // Αν ο κόμβος δεν είναι φύλλο, μεταφέρονται οι δείκτες παιδιών
    if (!child->leaf)
        memcpy(new_child->children, &child->children[mid + 1], (new_child->n + 1) * sizeof(BPLUS_DATA_NODE *));

    child->n = mid; // Μειώνεται το πλήθος εγγραφών του αρχικού κόμβου

    // Ενημέρωση του γονικού κόμβου
    for (int j = parent->n; j > i; --j)
        parent->children[j + 1] = parent->children[j];
    for (int j = parent->n - 1; j >= i; --j)
        parent->records[j + 1] = parent->records[j];

    parent->records[i] = child->records[mid];
    parent->children[i + 1] = new_child;
    parent->n++;

    // Ενημέρωση του δείκτη "next" αν ο κόμβος είναι φύλλο
    if (child->leaf) {
        new_child->next = child->next;
        child->next = new_child;
    }

    return 1;                                                       // Επιτυχής διαχωρισμός
}

// Εισαγωγή εγγραφής σε μη γεμάτο κόμβο
static void insert_unfull(BPLUS_DATA_NODE *node, Record record)
{
    int i = node->n - 1;

    if (node->leaf) {
        // Εισαγωγή εγγραφής με διατήρηση της σειράς
        while (i >= 0 && node->records[i].id > record.id) {
            node->records[i + 1] = node->records[i];
            --i;
        }
        node->records[i + 1] = record;
        node->n++;
    } else {
        // Βρίσκεται το κατάλληλο παιδί
        while (i >= 0 && node->records[i].id > record.id)
            --i;

        ++i;

        // Αν το παιδί είναι γεμάτο, γίνεται διαχωρισμός
        if (node->children[i]->n == MAX_KEYS) {
            split_child(node, i, node->children[i]);
            if (node->records[i].id < record.id)
                ++i;
        }
        insert_unfull(node->children[i], record);
    }
}

BPLUS_DATA_NODE *create_node(bool is_leaf)
{
    BPLUS_DATA_NODE *buffer = allocate_node();
    if (buffer == NULL)
        return NULL;        // Επιστροφή NULL αν η δεξαμενή είναι γεμάτη

    BPLUS_DATA_NODE node;
    node.leaf = is_leaf;    // Ρυθμίζεται αν είναι φύλλο
    node.n = 0;             // Αρχικοποίηση πλήθους εγγραφών
    node.next = NULL;       // Ο επόμενος κόμβος είναι NULL

    // Αρχικοποίηση παιδιών
    for (int i = 0; i < MAX_KEYS + 1; ++i)
        node.children[i] = NULL;

    // Αρχικοποίηση εγγραφών
    for (int i = 0; i < MAX_KEYS; ++i)
        node.records[i] = set_record(-1, "empty", "empty", "empty");

    memcpy(buffer, &node, sizeof(BPLUS_DATA_NODE));
    return buffer;
}

Record search(BPLUS_DATA_NODE *node, int id)
{
    Record rec = { .id = -1 };
    int i = 0;
    while (i < node->n && id > node->records[i].id)
        ++i;

    if (i < node->n && id == node->records[i].id) {
        rec = node->records[i];
        return rec; // Επιστροφή της εγγραφής αν βρεθεί
    }

    if (node->leaf)
        return rec; // Επιστροφή -1 αν δεν βρεθεί σε φύλλο

    return search(node->children[i], id); // Επαναληπτική αναζήτηση
}

int insert(BPLUS_INFO *info, Record record) 
{
    BPLUS_DATA_NODE *root = info->root;

    // Αναζήτηση για διπλότυπα
    Record search_result = search(root, record.id);
    if (search_result.id != -1)
        return 0;                                               // Αν υπάρχει ήδη, επιστρέφει 0

    if (root->n == MAX_KEYS) {
        BPLUS_DATA_NODE *new_node = create_node(false);
        if (new_node == NULL)
            return -1;                                          // Επιστροφή -1 αν αποτύχει η δημιουργία

        new_node->children[0] = root;
        split_child(new_node, 0, root);
        insert_unfull(new_node, record);
        info->root = new_node;
    } else {
        insert_unfull(root, record);
    }

    return 1;                                                   // Επιτυχής εισαγωγή
}

void display(BPLUS_DATA_NODE *node)
{
    if (node == NULL)
        return;
    int i;
    for (i = 0; i < node->n; i++) {
        if (!node->leaf) {
            display(node->children[i]);
        }
        printf("%d %s %s %s\n", node->records[i].id, node->records[i].name, node->records[i].surname, node->records[i].city);
    }
    if (!node->leaf) {
        display(node->children[i]);
    }
}

void destroy(BPLUS_DATA_NODE *node) 
{
    if (node == NULL)
        return;

    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++)
            destroy(node->children[i]);
    }

    memset(node, 0, sizeof(BPLUS_DATA_NODE));
}