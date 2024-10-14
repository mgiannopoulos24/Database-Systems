#ifndef HP_FILE_H
#define HP_FILE_H

#include "bf.h"
#include "record.h"

#define HP_MAX_FILENAME_LENGTH 256

#define MAX_MATCHES 1000

#define HP_ERROR -1
#define HP_OK 0

typedef struct {
    int fileDesc;
    char fileName[HP_MAX_FILENAME_LENGTH];
    int lastBlockId;
    int recordsPerBlock;
} HP_info;

typedef struct {
    int numRecords;
    int nextBlock;
} HP_block_info;

int HP_CreateFile(char *fileName);
HP_info* HP_OpenFile(char *fileName, int *file_desc);
int HP_CloseFile(int file_desc, HP_info *hp_info);
int HP_InsertEntry(int file_desc, HP_info *hp_info, Record record);
int HP_GetAllEntries(int file_desc, HP_info *hp_info, int value);

#endif // HP_FILE_H
