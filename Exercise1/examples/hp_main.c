#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 1000 // Αριθμός εγγραφών
#define FILE_NAME "data.db" // Όνομα αρχείου

// Μακροεντολή για τον έλεγχο σφαλμάτων
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU); // Αρχικοποίηση Buffer Manager

  // Δημιουργία και άνοιγμα αρχείου σωρού
  CALL_OR_DIE(HP_CreateFile(FILE_NAME));
  int file_desc;
  HP_info *hp_info = HP_OpenFile(FILE_NAME, &file_desc);
  if (hp_info == NULL) {
    fprintf(stderr, "Αποτυχία ανοίγματος αρχείου σωρού.\n");
    BF_Close();
    return EXIT_FAILURE;
  }

  Record record;
  srand(12569874); // Seed για τυχαίες εγγραφές

  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord(); // Δημιουργία τυχαίας εγγραφής
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

  // Κλείσιμο αρχείου σωρού
  if (HP_CloseFile(file_desc, hp_info) == -1) {
    fprintf(stderr, "Failed to close heap file.\n");
    BF_Close();
    return EXIT_FAILURE;
  }

  BF_Close(); // Κλείσιμο Buffer Manager
  return EXIT_SUCCESS;
}
