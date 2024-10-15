#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define FILE_NAME "data.db" // Το όνομα του αρχείου της βάσης δεδομένων

// Μακροεντολή για τον έλεγχο σφαλμάτων κατά την εκτέλεση συναρτήσεων της BF
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main(int argc, char *argv[]) {
    // Έλεγχος των ορισμάτων της γραμμής εντολών. Αναμένουμε ακριβώς ένα όρισμα (το ID)
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ID>\n", argv[0]);
        return 1;
    }

    // Μετατροπή του ορίσματος ID σε ακέραιο
    int id = atoi(argv[1]);
    if (id <= 0) {
        // Έλεγχος αν το ID είναι έγκυρο (πρέπει να είναι θετικός ακέραιος)
        fprintf(stderr, "Invalid ID: %d. ID must be a positive integer.\n", id);
        return 1;
    }

    // Αρχικοποίηση του Buffer Manager με την πολιτική LRU
    BF_Init(LRU);

    // Άνοιγμα του αρχείου πίνακα κατακερματισμού
    printf("Opening Hash Table file...\n");
    HT_info *info = HT_OpenFile(FILE_NAME);
    if (info == NULL) {
        fprintf(stderr, "Failed to open hash table file.\n");
        return 1;
    }

    // Αναζήτηση και εκτύπωση των εγγραφών για το δοσμένο ID
    printf("Fetching entries for ID: %d\n", id);
    int blocks_read = HT_GetAllEntries(info, id);
    if (blocks_read == HT_ERROR) {
        // Έλεγχος για σφάλμα κατά την αναζήτηση των εγγραφών
        fprintf(stderr, "Error while fetching entries for ID: %d\n", id);
        HT_CloseFile(info); // Κλείσιμο του αρχείου πίνακα κατακερματισμού
        BF_Close(); // Κλείσιμο του Buffer Manager
        return 1;
    }

    // Εκτύπωση του συνολικού αριθμού των μπλοκ που διαβάστηκαν
    printf("Total blocks read: %d\n", blocks_read);

    // Κλείσιμο του αρχείου πίνακα κατακερματισμού
    printf("Closing Hash Table file...\n");
    HT_CloseFile(info);

    // Κλείσιμο του Buffer Manager
    printf("Closing Buffer Manager...\n");
    BF_Close();

    // Ολοκλήρωση προγράμματος
    printf("Program completed successfully.\n");
    return 0;
}