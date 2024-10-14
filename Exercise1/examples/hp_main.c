#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define FILE_NAME "data.db"

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

  // Create and open heap file
  CALL_OR_DIE(HP_CreateFile(FILE_NAME));
  int file_desc;
  HP_info *hp_info = HP_OpenFile(FILE_NAME, &file_desc);
  if (hp_info == NULL) {
    fprintf(stderr, "Failed to open heap file.\n");
    BF_Close();
    return EXIT_FAILURE;
  }

  Record record;
  srand(12569874);

  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    if (HP_InsertEntry(file_desc, hp_info, record) == -1) {
      fprintf(stderr, "Failed to insert entry with id %d.\n", record.id);
    }
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n", id);
  if (HP_GetAllEntries(file_desc, hp_info, id) == -1) {
      fprintf(stderr, "Failed to get entries for id %d.\n", id);
  }


  // Close heap file and clean up
  if (HP_CloseFile(file_desc, hp_info) == -1) {
    fprintf(stderr, "Failed to close heap file.\n");
    BF_Close();
    return EXIT_FAILURE;
  }

  BF_Close();
  return EXIT_SUCCESS;
}
