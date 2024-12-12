#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return bplus_ERROR;     \
    }                         \
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