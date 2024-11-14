#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include <bp_datanode.h>
#include <bp_indexnode.h>
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




// Δημιουργία και αρχικοποίηση B+ δέντρου
int BP_CreateFile(char *fileName) {
    CALL_BF(BF_CreateFile(fileName));
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(file_desc, block));
    
    char *data = BF_Block_GetData(block);
    memset(data, 0, BF_BLOCK_SIZE); // Αρχικοποίηση του block με μηδενικά

    // Αποθήκευση μεταδεδομένων B+ δέντρου στο πρώτο μπλοκ
    BPLUS_INFO info;
    info.root = -1; // Ρίζα δεν έχει δημιουργηθεί ακόμα
    info.height = 0;
    memcpy(data, &info, sizeof(BPLUS_INFO));

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(file_desc));

    return 0;
}

// Άνοιγμα αρχείου B+ δέντρου και ανάκτηση μεταδεδομένων
BPLUS_INFO* BP_OpenFile(char *fileName, int *file_desc) {
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
    
    char *data = BF_Block_GetData(block);
    BPLUS_INFO *info = malloc(sizeof(BPLUS_INFO));
    if (info == NULL) {
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }
    memcpy(info, data, sizeof(BPLUS_INFO));
    
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    return info;
}

// Κλείσιμο αρχείου B+ δέντρου
int BP_CloseFile(int file_desc, BPLUS_INFO* info) {
    if (info) {
        free(info); // Απελευθέρωση της δομής μεταδεδομένων
    }
    return BF_CloseFile(file_desc) == BF_OK ? 0 : -1;
}

// Εισαγωγή εγγραφής στο B+ δέντρο
int BP_InsertEntry(int file_desc, BPLUS_INFO *bplus_info, Record record) {
    BF_Block *block;
    BF_Block_Init(&block);

    if (bplus_info->root == -1) {
        if (BF_AllocateBlock(file_desc, block) != BF_OK) {
            BF_Block_Destroy(&block);
            return -1;
        }
        char *data = BF_Block_GetData(block);
        
        BPLUS_DATA_NODE *root_node = (BPLUS_DATA_NODE*)data;
        root_node->is_leaf = true;
        root_node->key_count = 0;

        int blocks_num;
        if (BF_GetBlockCounter(file_desc, &blocks_num) != BF_OK) {
            BF_Block_Destroy(&block);
            return -1;
        }
        bplus_info->root = blocks_num - 1;

        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
    }

    int block_id = bplus_info->root;
    while (true) {
        if (BF_GetBlock(file_desc, block_id, block) != BF_OK) {
            BF_Block_Destroy(&block);
            return -1;
        }
        char *data = BF_Block_GetData(block);
        
        BPLUS_DATA_NODE *data_node = (BPLUS_DATA_NODE*)data;
        if (data_node->is_leaf) {
            if (insert_into_leaf(data_node, record)) {
                BF_Block_SetDirty(block);
                BF_UnpinBlock(block);
                BF_Block_Destroy(&block);
                return block_id;
            } else {
                BF_UnpinBlock(block);
                BF_Block_Destroy(&block);
                return -1;
            }
        } else {
            block_id = find_next_block((BPLUS_INDEX_NODE*)data, record.id);
            BF_UnpinBlock(block);
        }
    }
    BF_Block_Destroy(&block);
    return -1;
}

// Αναζήτηση εγγραφής σε B+ δέντρο
int BP_GetEntry(int file_desc, BPLUS_INFO *bplus_info, int id, Record **result) {
    *result = NULL;
    int block_id = bplus_info->root;
    BF_Block *block;
    BF_Block_Init(&block);

    while (true) {
        if (block_id == -1) {
            BF_Block_Destroy(&block);
            return -1;
        }

        CALL_BF(BF_GetBlock(file_desc, block_id, block));
        char *data = BF_Block_GetData(block);

        BPLUS_DATA_NODE *data_node = (BPLUS_DATA_NODE*)data;
        if (data_node->is_leaf) {
            Record* found_record = find_in_leaf(data_node, id);
            if (found_record != NULL) {
                // Δημιουργία νέας εγγραφής και αντιγραφή των δεδομένων
                *result = malloc(sizeof(Record));
                if (*result == NULL) {
                    BF_UnpinBlock(block);
                    BF_Block_Destroy(&block);
                    return -1;
                }
                memcpy(*result, found_record, sizeof(Record));
                BF_UnpinBlock(block);
                BF_Block_Destroy(&block);
                return 0;
            }
            BF_UnpinBlock(block);
            BF_Block_Destroy(&block);
            return -1;
        } else {
            BPLUS_INDEX_NODE *index_node = (BPLUS_INDEX_NODE*)data;
            block_id = find_next_block(index_node, id);
            BF_UnpinBlock(block);
        }
    }
    
    BF_Block_Destroy(&block);
    return -1;
}