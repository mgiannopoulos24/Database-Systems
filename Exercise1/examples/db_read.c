#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define FILE_NAME "data.db"
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ID>\n", argv[0]);
        return 1;
    }

    int id = atoi(argv[1]);
    if (id <= 0) {
        fprintf(stderr, "Invalid ID: %d. ID must be a positive integer.\n", id);
        return 1;
    }

    // Initialize the Buffer Manager
    BF_Init(LRU);

    // Open the hash table file
    printf("Opening Hash Table file...\n");
    HT_info *info = HT_OpenFile(FILE_NAME);
    if (info == NULL) {
        fprintf(stderr, "Failed to open hash table file.\n");
        return 1;
    }

    // Fetch and print the entries with the given ID
    printf("Fetching entries for ID: %d\n", id);
    int blocks_read = HT_GetAllEntries(info, id);
    if (blocks_read == HT_ERROR) {
        fprintf(stderr, "Error while fetching entries for ID: %d\n", id);
        HT_CloseFile(info);
        BF_Close();
        return 1;
    }

    printf("Total blocks read: %d\n", blocks_read);

    // Close the hash table file
    printf("Closing Hash Table file...\n");
    HT_CloseFile(info);

    // Close the Buffer Manager
    printf("Closing Buffer Manager...\n");
    BF_Close();

    printf("Program completed successfully.\n");
    return 0;
}