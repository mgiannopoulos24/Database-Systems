#include "ht_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "record.h"

// Μακροεντολή για τον έλεγχο σφαλμάτων στις κλήσεις της BF
#define CALL_BF(call)                             \
  do {                                            \
    BF_ErrorCode code = call;                     \
    if (code != BF_OK) {                          \
      BF_PrintError(code);                        \
      return HT_ERROR;                            \
    }                                             \
  } while (0)

// Δημιουργία αρχείου πίνακα κατακερματισμού
int HT_CreateFile(char *fileName, int buckets) {
    CALL_BF(BF_CreateFile(fileName)); // Δημιουργία αρχείου BF

    int fileDesc;
    CALL_BF(BF_OpenFile(fileName, &fileDesc)); // Άνοιγμα αρχείου BF

    HT_info ht_info;
    ht_info.fileDesc = fileDesc;
    strncpy(ht_info.fileName, fileName, 255); // Αντιγραφή ονόματος αρχείου
    ht_info.buckets = buckets;

    BF_Block *block;
    BF_Block_Init(&block); // Αρχικοποίηση block μνήμης

    CALL_BF(BF_AllocateBlock(fileDesc, block)); // Δέσμευση του πρώτου block για μεταδεδομένα
    char *data = BF_Block_GetData(block);
    memcpy(data, &ht_info, sizeof(HT_info)); // Αποθήκευση των μεταδεδομένων
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

    // Δέσμευση blocks για τα buckets
    HT_block_info bucketEntry;
    bucketEntry.blockId = -1; // Αρχικοποίηση κενών buckets
    for (int i = 0; i < buckets; ++i) {
        CALL_BF(BF_AllocateBlock(fileDesc, block)); // Δέσμευση block για το bucket
        data = BF_Block_GetData(block);
        memcpy(data, &bucketEntry, sizeof(HT_block_info)); // Αποθήκευση bucket info
        BF_Block_SetDirty(block);
        CALL_BF(BF_UnpinBlock(block));
    }

    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(fileDesc)); // Κλείσιμο αρχείου BF

    return HT_OK;
}

// Άνοιγμα αρχείου πίνακα κατακερματισμού
HT_info* HT_OpenFile(char *fileName) {
    int fileDesc;
    CALL_BF(BF_OpenFile(fileName, &fileDesc)); // Άνοιγμα αρχείου BF

    int num_blocks;
    CALL_BF(BF_GetBlockCounter(fileDesc, &num_blocks)); // Έλεγχος ύπαρξης blocks
    if (num_blocks == 0) {
        CALL_BF(BF_CloseFile(fileDesc));
        return NULL;
    }

    // Δέσμευση μνήμης για τη δομή HT_info
    HT_info *ht_info = malloc(sizeof(HT_info));
    if (ht_info == NULL) {
        CALL_BF(BF_CloseFile(fileDesc));
        return NULL;
    }

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(fileDesc, 0, block)); // Ανάγνωση του πρώτου block (μεταδεδομένα)
    char *data = BF_Block_GetData(block);
    memcpy(ht_info, data, sizeof(HT_info)); // Αντιγραφή μεταδεδομένων στη δομή HT_info
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    return ht_info;
}

// Κλείσιμο αρχείου πίνακα κατακερματισμού
int HT_CloseFile(HT_info *header_info) {
    CALL_BF(BF_CloseFile(header_info->fileDesc)); // Κλείσιμο αρχείου BF
    free(header_info); // Αποδέσμευση της δομής HT_info
    return HT_OK;
}

// Εισαγωγή εγγραφής στον πίνακα κατακερματισμού
int HT_InsertEntry(HT_info *header_info, Record record) {
    if (header_info == NULL) {
        return HT_ERROR;
    }

    // Υπολογισμός του bucket στο οποίο θα τοποθετηθεί η εγγραφή
    int bucket_index = record.id % header_info->buckets;

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(header_info->fileDesc, bucket_index + 1, block)); // Ανάγνωση του block του bucket
    char *data = BF_Block_GetData(block);
    HT_block_info bucketEntry;
    memcpy(&bucketEntry, data, sizeof(HT_block_info)); // Ανάγνωση πληροφοριών του bucket

    int current_block_id = bucketEntry.blockId;
    BF_Block *dataBlock;
    BF_Block_Init(&dataBlock);

    // Εύρεση κενής θέσης για την εισαγωγή της εγγραφής
    while (current_block_id != -1) {
        CALL_BF(BF_GetBlock(header_info->fileDesc, current_block_id, dataBlock));
        char *blockData = BF_Block_GetData(dataBlock);
        Record *records = (Record *)blockData;

        // Αναζήτηση κενής εγγραφής στο block
        for (int i = 0; i < (BF_BLOCK_SIZE / sizeof(Record)); i++) {
            if (records[i].id == -1) {
                memcpy(&records[i], &record, sizeof(Record)); // Εισαγωγή εγγραφής
                BF_Block_SetDirty(dataBlock);
                CALL_BF(BF_UnpinBlock(dataBlock));
                BF_Block_Destroy(&dataBlock);
                CALL_BF(BF_UnpinBlock(block));
                BF_Block_Destroy(&block);
                return bucket_index;
            }
        }

        // Αν το block είναι γεμάτο, προχωράμε στο επόμενο
        HT_block_info *block_info = (HT_block_info *)(blockData + BF_BLOCK_SIZE - sizeof(HT_block_info));
        current_block_id = block_info->nextBlock;
        CALL_BF(BF_UnpinBlock(dataBlock));
    }

    // Δέσμευση νέου block αν δεν υπάρχει διαθέσιμος χώρος
    CALL_BF(BF_AllocateBlock(header_info->fileDesc, dataBlock));
    int new_block_id;
    CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &new_block_id));
    new_block_id -= 1;

    char *newBlockData = BF_Block_GetData(dataBlock);
    Record *new_records = (Record *)newBlockData;
    memcpy(&new_records[0], &record, sizeof(Record)); // Εισαγωγή της νέας εγγραφής

    // Αρχικοποίηση των υπόλοιπων εγγραφών στο block ως κενές
    for (int i = 1; i < (BF_BLOCK_SIZE / sizeof(Record)); i++) {
        new_records[i].id = -1;
    }

    HT_block_info *new_block_info = (HT_block_info *)(newBlockData + BF_BLOCK_SIZE - sizeof(HT_block_info));
    new_block_info->blockId = new_block_id;
    new_block_info->nextBlock = -1;

    // Αν το bucket δεν έχει προηγούμενο block, το ενημερώνουμε
    if (bucketEntry.blockId == -1) {
        bucketEntry.blockId = new_block_id;
        memcpy(data, &bucketEntry, sizeof(HT_block_info));
        BF_Block_SetDirty(block);
    } else {
        // Διαφορετικά ενημερώνουμε το τελευταίο block της αλυσίδας
        HT_block_info *last_block_info = (HT_block_info *)(data + BF_BLOCK_SIZE - sizeof(HT_block_info));
        last_block_info->nextBlock = new_block_id;
        BF_Block_SetDirty(dataBlock);
    }

    BF_Block_SetDirty(dataBlock);
    CALL_BF(BF_UnpinBlock(dataBlock));
    BF_Block_Destroy(&dataBlock);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return bucket_index;
}

// Αναζήτηση όλων των εγγραφών με το δεδομένο ID στον πίνακα κατακερματισμού
int HT_GetAllEntries(HT_info *header_info, int value) {
    if (header_info == NULL || header_info->buckets <= 0) {
        return HT_ERROR;
    }

    // Υπολογισμός του bucket στο οποίο ανήκει το ID
    int bucket_index = value % header_info->buckets;

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(header_info->fileDesc, bucket_index + 1, block)); // Ανάγνωση του block του bucket
    char *data = BF_Block_GetData(block);
    HT_block_info bucketEntry;
    memcpy(&bucketEntry, data, sizeof(HT_block_info)); // Ανάγνωση πληροφοριών του bucket

    int current_block_id = bucketEntry.blockId;
    BF_Block *dataBlock;
    BF_Block_Init(&dataBlock);
    int blocks_read = 0;
    int found = 0;

    // Σάρωση όλων των blocks του bucket
    while (current_block_id != -1) {
        CALL_BF(BF_GetBlock(header_info->fileDesc, current_block_id, dataBlock));
        char *blockData = BF_Block_GetData(dataBlock);
        Record *records = (Record *)blockData;

        // Αναζήτηση των εγγραφών με το ζητούμενο ID
        for (int i = 0; i < (BF_BLOCK_SIZE / sizeof(Record)); i++) {
            if (records[i].id == value) {
                printf("Record found: ID: %d, Name: %s, Surname: %s, City: %s\n",
                       records[i].id, records[i].name, records[i].surname, records[i].city);
                found = 1;
            }
        }

        // Προχωράμε στο επόμενο block
        HT_block_info *block_info = (HT_block_info *)(blockData + BF_BLOCK_SIZE - sizeof(HT_block_info));
        current_block_id = block_info->nextBlock;
        CALL_BF(BF_UnpinBlock(dataBlock));
        blocks_read++;
    }

    if (!found) {
        printf("No record found with ID: %d\n", value);
    }

    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    BF_Block_Destroy(&dataBlock);
    
    return blocks_read;
}