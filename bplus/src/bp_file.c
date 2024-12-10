#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include <bp_datanode.h>
#include <stdbool.h>

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return bplus_ERROR;     \
    }                         \
  }


#define MAX_NODES 1024 // Μέγιστος αριθμός κόμβων που μπορούν να αποθηκευτούν
static BPLUS_DATA_NODE node_pool[MAX_NODES]; // Προκαθορισμένη δεξαμενή κόμβων
static int node_pool_index = 0; // Δείκτης για την τρέχουσα θέση στη δεξαμενή

// Συνάρτηση που επιστρέφει έναν νέο κόμβο από τη δεξαμενή
static BPLUS_DATA_NODE *allocate_node()
{
    if (node_pool_index >= MAX_NODES)
        return NULL; // Αν η δεξαμενή είναι γεμάτη, επιστρέφει NULL
    return &node_pool[node_pool_index++];
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

// Δημιουργία κόμβου με προεπιλεγμένες τιμές
static BPLUS_DATA_NODE *create_node(bool is_leaf)
{
    BPLUS_DATA_NODE *buffer = allocate_node();
    if (buffer == NULL)
        return NULL; // Επιστροφή NULL αν η δεξαμενή είναι γεμάτη

    BPLUS_DATA_NODE node;
    node.leaf = is_leaf; // Ρυθμίζεται αν είναι φύλλο
    node.n = 0; // Αρχικοποίηση πλήθους εγγραφών
    node.next = NULL; // Ο επόμενος κόμβος είναι NULL

    // Αρχικοποίηση παιδιών
    for (int i = 0; i < MAX_KEYS + 1; ++i)
        node.children[i] = NULL;

    // Αρχικοποίηση εγγραφών
    for (int i = 0; i < MAX_KEYS; ++i)
        node.records[i] = set_record(-1, "empty", "empty", "empty");

    memcpy(buffer, &node, sizeof(BPLUS_DATA_NODE));
    return buffer;
}

// Διαχωρισμός κόμβου όταν γεμίσει
static int split_child(BPLUS_DATA_NODE *parent, int i, BPLUS_DATA_NODE *child)
{
    BPLUS_DATA_NODE *new_child = create_node(child->leaf);
    if (new_child == NULL)
        return 0; // Επιστροφή 0 αν η δημιουργία αποτύχει

    int mid = child->n / 2; // Βρίσκεται το μέσο σημείο για διαχωρισμό

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

    return 1; // Επιτυχής διαχωρισμός
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


// Αναζήτηση εγγραφής με βάση το id
static Record search(BPLUS_DATA_NODE *node, int id)
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

// Εισαγωγή εγγραφής στο δέντρο
static int insert(BPLUS_INFO *info, Record record) 
{
    BPLUS_DATA_NODE *root = info->root;

    // Αναζήτηση για διπλότυπα
    Record search_result = search(root, record.id);
    if (search_result.id != -1)
        return 0; // Αν υπάρχει ήδη, επιστρέφει 0

    if (root->n == MAX_KEYS) {
        BPLUS_DATA_NODE *new_node = create_node(false);
        if (new_node == NULL)
            return -1; // Επιστροφή -1 αν αποτύχει η δημιουργία

        new_node->children[0] = root;
        split_child(new_node, 0, root);
        insert_unfull(new_node, record);
        info->root = new_node;
    } else {
        insert_unfull(root, record);
    }
    return 1; // Επιτυχής εισαγωγή
}

// Εμφάνιση όλων των εγγραφών του δέντρου
static void display(BPLUS_DATA_NODE *node)
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

// Καταστροφή όλων των κόμβων του δέντρου
static void destroy_node(BPLUS_DATA_NODE *node) 
{
    if (node == NULL)
        return;

    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++)
            destroy_node(node->children[i]);
    }

    memset(node, 0, sizeof(BPLUS_DATA_NODE));
}

// Συνάρτηση για δημιουργία αρχείου B+ δέντρου
int BP_CreateFile(char *fileName)
{
    CALL_BF(BF_CreateFile(fileName));
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(file_desc, block));

    BPLUS_INFO bplus_info;
    bplus_info.root = NULL;
    bplus_info.block_numbers = 1;

    memcpy(block, &bplus_info, sizeof(BPLUS_INFO));

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(file_desc));

    return 0;
}


// Συνάρτηση για άνοιγμα αρχείου B+ δέντρου
BPLUS_INFO *BP_OpenFile(char *fileName, int *file_desc)
{
    if (BF_OpenFile(fileName, file_desc) != BF_OK) {
        return NULL;
    }

    BF_Block *block;
    BF_Block_Init(&block);

    if (BF_GetBlock(*file_desc, 0, block) != BF_OK) {
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    BPLUS_INFO *bplus_info = malloc(sizeof(BPLUS_INFO));
    if (bplus_info == NULL) {
        perror("Error allocating memory for bplus_info");
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    int blocks_num;
    if (BF_GetBlockCounter(*file_desc, &blocks_num) != BF_OK) {
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    bplus_info->block_numbers = blocks_num;
    bplus_info->root = create_node(true);
    memcpy(BF_Block_GetData(block), bplus_info, sizeof(BPLUS_INFO));

    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    return bplus_info;
}

// Συνάρτηση για κλείσιμο αρχείου B+ δέντρου
int BP_CloseFile(int file_desc, BPLUS_INFO *info)
{
    if (info != NULL)
        free(info);

    return BF_CloseFile(file_desc) == BF_OK ? 0 : -1;
}

// Εισαγωγή εγγραφής στο αρχείο B+ δέντρου
int BP_InsertEntry(int file_desc, BPLUS_INFO *bplus_info, Record record)
{
    if (bplus_info == NULL || bplus_info->root == NULL) {
        fprintf(stderr, "Initialize the bplus_info first\n");
        return -1;
    }

    if (file_desc < 0) {
        fprintf(stderr, "File not opened correctly\n");
        return -1;
    }

    int res = insert(bplus_info, record);
    if (res == 0) {
        fprintf(stderr, "Duplicate id detected: %d\n", record.id);
        return -1;
    } else if (res == -1) {
        fprintf(stderr, "Error during insert\n");
        return -1;
    }

    BF_Block *block;
    BF_Block_Init(&block);

    CALL_BF(BF_AllocateBlock(file_desc, block));

    int blocks_num;
    CALL_BF(BF_GetBlockCounter(file_desc, &blocks_num));
    bplus_info->block_numbers = blocks_num;

    memcpy(BF_Block_GetData(block), bplus_info, sizeof(BPLUS_INFO));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    return blocks_num;
}

// Αναζήτηση εγγραφής στο αρχείο B+ δέντρου
int BP_GetEntry(int file_desc, BPLUS_INFO *bplus_info, int value, Record **result)
{
    if (bplus_info == NULL)
        return -1;

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(file_desc, bplus_info->block_numbers - 1, block));
    bplus_info = (BPLUS_INFO *)BF_Block_GetData(block);

    BPLUS_DATA_NODE *node = bplus_info->root;
    Record rec = search(node, value);
    if (rec.id != -1) {
        memcpy(*result, &rec, sizeof(Record));
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        return 0;
    }

    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);

    *result = NULL;
    return -1;
}