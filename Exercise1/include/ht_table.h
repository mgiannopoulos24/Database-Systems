#ifndef HT_TABLE_H
#define HT_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "record.h"

#define HT_ERROR -1
#define HT_OK 0

#define HT_BUCKET_SIZE 50  // Number of records per bucket block

typedef struct {
    int fileDesc;               // File descriptor
    char fileName[256];         // File name
    int buckets;                // Number of buckets
} HT_info;

typedef struct {
    int blockId;        // Block ID containing records
    int nextBlock;      // ID of the next block in the chain (-1 if none)
} HT_block_info;

int HT_CreateFile(char *fileName, int buckets);
HT_info* HT_OpenFile(char *fileName);
int HT_CloseFile(HT_info *header_info);
int HT_InsertEntry(HT_info *header_info, Record record);
int HT_GetAllEntries(HT_info *header_info, int value);

#endif /* HT_TABLE_H */
