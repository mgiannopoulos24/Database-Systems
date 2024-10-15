#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define FILE_NAME "data.db"
#define NUM_BUCKETS 50 // Number of buckets for hash table

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);

  printf("Creating Hash Table file with %d buckets\n", NUM_BUCKETS);
  CALL_OR_DIE(HT_CreateFile(FILE_NAME, NUM_BUCKETS));

  printf("Opening Hash Table file...\n");
  HT_info* info = HT_OpenFile(FILE_NAME);
  if (info == NULL) {
      fprintf(stderr, "Failed to open hash table file.\n");
      return 1;
  }

  printf("Opened Hash Table file with %d buckets\n", info->buckets);

  Record record;
  srand(time(NULL)); // Seeding the random number generator
  printf("Inserting Entries...\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    // printf("Inserting Record with ID: %d\n", record.id);
    HT_InsertEntry(info, record);
  }

  printf("Running PrintAllEntries...\n");
  int id = rand() % RECORDS_NUM;
  printf("Printing entries for ID: %d\n", id);
  HT_GetAllEntries(info, id);

  printf("Closing Hash Table file...\n");
  HT_CloseFile(info);

  printf("Closing Buffer Manager...\n");
  BF_Close();

  printf("Program completed successfully.\n");

  return 0;
}
