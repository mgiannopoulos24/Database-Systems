#include "ht_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "record.h"

#define CALL_BF(call)                             \
  do {                                            \
    BF_ErrorCode code = call;                     \
    if (code != BF_OK) {                          \
      BF_PrintError(code);                        \
      return HT_ERROR;                            \
    }                                             \
  } while (0)

int HT_CreateFile(char *fileName, int buckets) {
    CALL_BF(BF_CreateFile(fileName));

    int fileDesc;
    CALL_BF(BF_OpenFile(fileName, &fileDesc));

    HT_info ht_info;
    ht_info.fileDesc = fileDesc;
    strncpy(ht_info.fileName, fileName, 255);
    ht_info.buckets = buckets;

    BF_Block *block;
    BF_Block_Init(&block);

    CALL_BF(BF_AllocateBlock(fileDesc, block));
    char *data = BF_Block_GetData(block);
    memcpy(data, &ht_info, sizeof(HT_info));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

    HT_block_info bucketEntry;
    bucketEntry.blockId = -1;
    for (int i = 0; i < buckets; ++i) {
        CALL_BF(BF_AllocateBlock(fileDesc, block));
        data = BF_Block_GetData(block);
        memcpy(data, &bucketEntry, sizeof(HT_block_info));
        BF_Block_SetDirty(block);
        CALL_BF(BF_UnpinBlock(block));
    }

    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(fileDesc));

    return HT_OK;
}

HT_info* HT_OpenFile(char *fileName) {
    int fileDesc;
    CALL_BF(BF_OpenFile(fileName, &fileDesc));

    int num_blocks;
    CALL_BF(BF_GetBlockCounter(fileDesc, &num_blocks));
    if (num_blocks == 0) {
        CALL_BF(BF_CloseFile(fileDesc));
        return NULL;
    }

    HT_info *ht_info = malloc(sizeof(HT_info));
    if (ht_info == NULL) {
        CALL_BF(BF_CloseFile(fileDesc));
        return NULL;
    }

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(fileDesc, 0, block));
    char *data = BF_Block_GetData(block);
    memcpy(ht_info, data, sizeof(HT_info));
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    return ht_info;
}

int HT_CloseFile(HT_info *header_info) {
    CALL_BF(BF_CloseFile(header_info->fileDesc));
    free(header_info);
    return HT_OK;
}

int HT_InsertEntry(HT_info *header_info, Record record) {
    if (header_info == NULL) {
        return HT_ERROR;
    }

    int bucket_index = record.id % header_info->buckets;
    printf("Calculated bucket_index = %d for record.id = %d\n", bucket_index, record.id);

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(header_info->fileDesc, bucket_index + 1, block)); // bucket_index + 1 because first block is metadata
    char *data = BF_Block_GetData(block);
    HT_block_info bucketEntry;
    memcpy(&bucketEntry, data, sizeof(HT_block_info));

    int current_block_id = bucketEntry.blockId;
    BF_Block *dataBlock;
    BF_Block_Init(&dataBlock);

    while (current_block_id != -1) {
        CALL_BF(BF_GetBlock(header_info->fileDesc, current_block_id, dataBlock));
        char *blockData = BF_Block_GetData(dataBlock);
        Record *records = (Record *)blockData;

        for (int i = 0; i < (BF_BLOCK_SIZE / sizeof(Record)); i++) {
            if (records[i].id == -1) {
                memcpy(&records[i], &record, sizeof(Record));
                BF_Block_SetDirty(dataBlock);
                CALL_BF(BF_UnpinBlock(dataBlock));
                BF_Block_Destroy(&dataBlock);
                CALL_BF(BF_UnpinBlock(block));
                BF_Block_Destroy(&block);
                return bucket_index;
            }
        }

        HT_block_info *block_info = (HT_block_info *)(blockData + BF_BLOCK_SIZE - sizeof(HT_block_info));
        current_block_id = block_info->nextBlock;
        CALL_BF(BF_UnpinBlock(dataBlock));
    }

    CALL_BF(BF_AllocateBlock(header_info->fileDesc, dataBlock));
    int new_block_id;
    CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &new_block_id));
    new_block_id -= 1;

    char *newBlockData = BF_Block_GetData(dataBlock);
    Record *new_records = (Record *)newBlockData;
    memcpy(&new_records[0], &record, sizeof(Record));

    for (int i = 1; i < (BF_BLOCK_SIZE / sizeof(Record)); i++) {
        new_records[i].id = -1;
    }

    HT_block_info *new_block_info = (HT_block_info *)(newBlockData + BF_BLOCK_SIZE - sizeof(HT_block_info));
    new_block_info->blockId = new_block_id;
    new_block_info->nextBlock = -1;

    if (bucketEntry.blockId == -1) {
        bucketEntry.blockId = new_block_id;
        memcpy(data, &bucketEntry, sizeof(HT_block_info));
        BF_Block_SetDirty(block);
    } else {
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

int HT_GetAllEntries(HT_info *header_info, int value) {
    if (header_info == NULL || header_info->buckets <= 0) {
        return HT_ERROR;
    }

    int bucket_index = value % header_info->buckets;

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(header_info->fileDesc, bucket_index + 1, block)); // bucket_index + 1 because first block is metadata
    char *data = BF_Block_GetData(block);
    HT_block_info bucketEntry;
    memcpy(&bucketEntry, data, sizeof(HT_block_info));

    int current_block_id = bucketEntry.blockId;
    BF_Block *dataBlock;
    BF_Block_Init(&dataBlock);

    while (current_block_id != -1) {
        CALL_BF(BF_GetBlock(header_info->fileDesc, current_block_id, dataBlock));
        char *blockData = BF_Block_GetData(dataBlock);
        Record *records = (Record *)blockData;

        for (int i = 0; i < (BF_BLOCK_SIZE / sizeof(Record)); i++) {
            if (records[i].id == value) {
                printf("Record found: ID: %d, Name: %s, Surname: %s, City: %s\n",
                       records[i].id, records[i].name, records[i].surname, records[i].city);
            }
        }

        HT_block_info *block_info = (HT_block_info *)(blockData + BF_BLOCK_SIZE - sizeof(HT_block_info));
        current_block_id = block_info->nextBlock;
        CALL_BF(BF_UnpinBlock(dataBlock));
    }

    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    BF_Block_Destroy(&dataBlock);
    return HT_OK;
}
