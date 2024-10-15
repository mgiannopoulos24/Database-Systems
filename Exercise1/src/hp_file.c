#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

// Μακροεντολή για τον έλεγχο σφαλμάτων στις κλήσεις της BF
#define CALL_BF(call, retval)       \
{                                   \
  BF_ErrorCode code = call;         \
  if (code != BF_OK) {              \
    BF_PrintError(code);            \
    return retval;                  \
  }                                 \
}

// Δημιουργία αρχείου σωρού
int HP_CreateFile(char *fileName) {
    CALL_BF(BF_CreateFile(fileName), HP_ERROR); // Δημιουργία αρχείου BF
    
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc), HP_ERROR); // Άνοιγμα αρχείου BF

    BF_Block *block;
    BF_Block_Init(&block); // Αρχικοποίηση block μνήμης

    CALL_BF(BF_AllocateBlock(file_desc, block), HP_ERROR); // Δέσμευση πρώτου block

    char *block_data = BF_Block_GetData(block);

    // Αρχικοποίηση των μεταδεδομένων του αρχείου σωρού
    HP_info hp_info = {
        .fileDesc = file_desc,
        .lastBlockId = 0, 
        .recordsPerBlock = (BF_BLOCK_SIZE - sizeof(HP_block_info)) / sizeof(Record)
    };
    strcpy(hp_info.fileName, fileName);

    // Αντιγραφή των μεταδεδομένων στο block
    memcpy(block_data, &hp_info, sizeof(HP_info));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block), HP_ERROR);

    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(file_desc), HP_ERROR); // Κλείσιμο του αρχείου
    
    return HP_OK;
}

// Άνοιγμα αρχείου σωρού
HP_info* HP_OpenFile(char *fileName, int *file_desc) {
    BF_ErrorCode code;
    
    code = BF_OpenFile(fileName, file_desc); // Άνοιγμα αρχείου
    if (code != BF_OK) {
        BF_PrintError(code);
        return NULL;
    }

    BF_Block *block;
    BF_Block_Init(&block);

    CALL_BF(BF_GetBlock(*file_desc, 0, block), NULL); // Ανάγνωση του πρώτου block (μεταδεδομένα)

    char *block_data = BF_Block_GetData(block);

    // Δέσμευση μνήμης για τη δομή HP_info
    HP_info *hp_info = malloc(sizeof(HP_info));
    if (hp_info == NULL) {
        fprintf(stderr, "Error allocating memory for HP_info.\n");
        CALL_BF(BF_UnpinBlock(block), NULL);
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }

    // Αντιγραφή μεταδεδομένων από το block στη δομή HP_info
    memcpy(hp_info, block_data, sizeof(HP_info));
    
    CALL_BF(BF_UnpinBlock(block), NULL);
    BF_Block_Destroy(&block);

    return hp_info;
}

// Κλείσιμο αρχείου σωρού
int HP_CloseFile(int file_desc, HP_info *hp_info) {
    free(hp_info); // Αποδέσμευση της δομής HP_info
    CALL_BF(BF_CloseFile(file_desc), HP_ERROR); // Κλείσιμο του αρχείου
    return HP_OK;
}

// Εισαγωγή εγγραφής στο αρχείο σωρού
int HP_InsertEntry(int file_desc, HP_info *hp_info, Record record) {
    BF_Block *block;
    BF_Block_Init(&block);

    // Αναζήτηση για διαθέσιμο block
    for (int block_num = 1; block_num <= hp_info->lastBlockId; block_num++) {
        CALL_BF(BF_GetBlock(file_desc, block_num, block), HP_ERROR);

        char *block_data = BF_Block_GetData(block);
        HP_block_info *block_info = (HP_block_info *)(block_data + BF_BLOCK_SIZE - sizeof(HP_block_info));

        // Αν το block έχει διαθέσιμο χώρο, εισάγεται η εγγραφή
        if (block_info->numRecords < hp_info->recordsPerBlock) {
            memcpy(block_data + block_info->numRecords * sizeof(Record), &record, sizeof(Record));
            block_info->numRecords++;
            BF_Block_SetDirty(block);
            CALL_BF(BF_UnpinBlock(block), HP_ERROR);
            BF_Block_Destroy(&block);
            return block_num;
        }

        CALL_BF(BF_UnpinBlock(block), HP_ERROR);
    }

    // Αν δεν υπάρχει διαθέσιμο block, δεσμεύεται νέο
    CALL_BF(BF_AllocateBlock(file_desc, block), HP_ERROR);
    char *block_data = BF_Block_GetData(block);

    // Εισαγωγή της εγγραφής στο νέο block
    memcpy(block_data, &record, sizeof(Record));
    HP_block_info block_info = { .numRecords = 1, .nextBlock = -1 };
    memcpy(block_data + BF_BLOCK_SIZE - sizeof(HP_block_info), &block_info, sizeof(HP_block_info));

    hp_info->lastBlockId++;
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block), HP_ERROR);
    BF_Block_Destroy(&block);

    // Ενημέρωση των μεταδεδομένων στο πρώτο block
    BF_Block *info_block;
    BF_Block_Init(&info_block);
    CALL_BF(BF_GetBlock(file_desc, 0, info_block), HP_ERROR);
    char *info_data = BF_Block_GetData(info_block);
    memcpy(info_data, hp_info, sizeof(HP_info));
    BF_Block_SetDirty(info_block);
    CALL_BF(BF_UnpinBlock(info_block), HP_ERROR);
    BF_Block_Destroy(&info_block);

    return hp_info->lastBlockId;
}

// Αναζήτηση όλων των εγγραφών με το δεδομένο ID
int HP_GetAllEntries(int file_desc, HP_info *hp_info, int value) {
    BF_Block *block;
    BF_Block_Init(&block);

    int blocks_read = 0;

    // Σάρωση όλων των blocks για αναζήτηση εγγραφών με το ζητούμενο ID
    for (int block_num = 1; block_num <= hp_info->lastBlockId; block_num++) {
        CALL_BF(BF_GetBlock(file_desc, block_num, block), HP_ERROR);
        char *block_data = BF_Block_GetData(block);
        HP_block_info *block_info = (HP_block_info *)(block_data + BF_BLOCK_SIZE - sizeof(HP_block_info));

        for (int i = 0; i < block_info->numRecords; i++) {
            Record record;
            memcpy(&record, block_data + i * sizeof(Record), sizeof(Record));

            if (record.id == value) {
                printf("Record found: ID: %d, Name: %s, Surname: %s, City: %s\n", record.id, record.name, record.surname, record.city);
            }
        }

        blocks_read++;
        CALL_BF(BF_UnpinBlock(block), HP_ERROR);
    }

    BF_Block_Destroy(&block);
    return blocks_read;
}
