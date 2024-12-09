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


static Record set_record(int id, char *name, char *surname, char *city)
{
    Record record = { .id = id };
    strcpy(record.name, name);
    strcpy(record.surname, surname);
    strcpy(record.city, city);

    return record;
}

static BPLUS_DATA_NODE *create_node(bool is_leaf)
{
    BPLUS_DATA_NODE *node;
    if ((node = malloc(sizeof(BPLUS_DATA_NODE))) == NULL)
        return NULL;

    node->leaf = is_leaf;
    node->n = 0;
    node->next = NULL;
    for (int i = 0; i < MAX_KEYS + 1; ++i)
        node->children[i] = malloc(sizeof(BPLUS_DATA_NODE));

    for (int i = 0; i < MAX_KEYS; ++i)
        node->records[i] = set_record(-1, "empty", "empty", "empty");
    
    return node;
}

static int split_child(BPLUS_DATA_NODE *parent, int i, BPLUS_DATA_NODE *child)
{
    // Create a new node to hold keys/children after the split
    BPLUS_DATA_NODE *new_child;
    if ((new_child = create_node(child->leaf)) == NULL)
        return 0;

    // Middle index to split the keys
    int mid = child->n / 2;

    // Transfer keys to the new child
    new_child->n = child->n - mid - 1; // Keys after the mid go to the new child
    for (int j = 0; j < new_child->n; ++j)
        new_child->records[j] = child->records[mid + 1 + j];

    // If the node is not a leaf, transfer children pointers as well
    if (!child->leaf) {
        for (int j = 0; j <= new_child->n; ++j)
            new_child->children[j] = child->children[mid + 1 + j];
    }

    // Reduce the number of keys in the original child
    child->n = mid;

    // Update the parent: Shift keys and children pointers to make room
    for (int j = parent->n; j > i; --j)
        parent->children[j + 1] = parent->children[j];

    for (int j = parent->n - 1; j >= i; --j)
        parent->records[j + 1] = parent->records[j];

    // Insert the middle key from the child into the parent
    parent->records[i] = child->records[mid];
    parent->children[i + 1] = new_child;
    parent->n++;

    // If the child is a leaf, update the next pointer
    if (child->leaf) {
        new_child->next = child->next;
        child->next = new_child;
    }

    return 1;
}

// returns
// 1 on success
// 0 if the record.id exists
static int insert_unfull(BPLUS_DATA_NODE *node, Record record)
{
    int i = node->n - 1;

    if (node->leaf) {
        // Check for duplicates and find insertion position
        while (i >= 0 && node->records[i].id > record.id) {
            node->records[i + 1] = node->records[i];
            --i;
        }
        if (i >= 0 && node->records[i].id == record.id)
            return 0; // Duplicate found

        // Insert the record
        node->records[i + 1] = record;
        node->n++;
        return 1;
    }

    // Non-leaf node: Find the child to descend into
    while (i >= 0 && node->records[i].id > record.id)
        --i;

    ++i;
    // Check for duplicates in the current node
    if (i < node->n && node->records[i].id == record.id)
        return 0; 

    // Split child if full and adjust the insertion path
    if (node->children[i]->n == MAX_KEYS) {
        split_child(node, i, node->children[i]);
        if (node->records[i].id < record.id)
            ++i;
    }

    // Recurse into the appropriate child
    return insert_unfull(node->children[i], record);
}


// returns
// 1 on success
// 0 if the record.id exists
// -1 on error
static int insert(BPLUS_INFO *info, Record record)
{
    BPLUS_DATA_NODE *root = info->root;
    if (root->n == MAX_KEYS) {
        BPLUS_DATA_NODE *new_node;
        if ((new_node = create_node(false)) == NULL)
            return -1;
        
        new_node->children[0] = root;
        split_child(new_node, 0, root);
        if (insert_unfull(new_node, record) == 0)
            return 0;
        info->root = new_node;
    } else {
        if(insert_unfull(root, record) == 0)
            return 0;
    return 1;
    }
}

static Record search(BPLUS_DATA_NODE *node, int id)
{
    Record rec = { .id = -1};
    int i = 0;
    while (i < node->n && id > node->records[i].id)
        ++i;

    if (i < node->n && id == node->records[i].id) {
        rec = node->records[i];
        return rec;
    }

    if (node->leaf)
        return rec;

    // Check if the child pointer is valid before dereferencing
    if (node->children[i] == NULL)
        return rec; // This prevents segfault if the child pointer is invalid
    
    return search(node->children[i], id);
}

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

static void destroy(BPLUS_DATA_NODE *node) 
{
    if (node == NULL)
        return;

    // If the node is not a leaf, recursively free its children
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++)
            destroy(node->children[i]);
    }

    // Free the node itself
    free(node);
}

int BP_CreateFile(char *fileName)
{
    CALL_BF(BF_CreateFile(fileName));
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(file_desc, block));

    BPLUS_INFO bplus_info;
    bplus_info.root = NULL;         // Root node is null for an empty tree
    bplus_info.block_numbers = 1;   // First block reserved for metadata

    memcpy(block, &bplus_info, sizeof(BPLUS_INFO));

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(file_desc));

    return 0;
}


BPLUS_INFO* BP_OpenFile(char *fileName, int *file_desc)
{
    // Open the file
    if (BF_OpenFile(fileName, file_desc) != BF_OK) {
        return NULL;
    }

    BF_Block *block;
    BF_Block_Init(&block);

    // Get the first block
    if (BF_GetBlock(*file_desc, 0, block) != BF_OK) {
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    // Extract metadata
    BPLUS_INFO *bplus_info = malloc(sizeof(BPLUS_INFO));
    if (bplus_info == NULL) {
        perror("Huston, we have a problem\n");
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

int BP_CloseFile(int file_desc, BPLUS_INFO* info)
{  
    if (info != NULL) {
        if (info->root != NULL)
            destroy(info->root);
        free(info);
    }

    return BF_CloseFile(file_desc) == BF_OK ? 0 : -1;
}

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

    // Insert the record into the root node
    int res = insert(bplus_info, record);
    if (res == 0) {
        fprintf(stderr, "I cannot insert the record: %d %s %s %s because id %d already exists\n", record.id, record.name, record.surname, record.city, record.id);
        return -1;
    } else if (res == -1) {
        fprintf(stderr, "Error on insert\n");
        return -1;
    }

    BF_Block *block;
    BF_Block_Init(&block);

    // Allocate a block for the root node
    CALL_BF(BF_AllocateBlock(file_desc, block));

    int blocks_num;
    CALL_BF(BF_GetBlockCounter(file_desc, &blocks_num));
    bplus_info->block_numbers = blocks_num;

    // Copy the root node into the allocated block
    memcpy(BF_Block_GetData(block), bplus_info, sizeof(BPLUS_INFO));

    // Set block dirty and unpin it
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    return blocks_num; // Successfully inserted the first record
}


int BP_GetEntry(int file_desc, BPLUS_INFO *bplus_info, int value, Record **result)
{
    if (bplus_info == NULL)
        return -1;

    // Initialize the block variable to read the data
    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(file_desc, bplus_info->block_numbers - 1, block));
    bplus_info = (BPLUS_INFO*)BF_Block_GetData(block);

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

    // Record not found
    *result = NULL;
    return -1; // Failure
}
