#ifndef HT_TABLE_H
#define HT_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "record.h"

#define HT_ERROR -1 // Κωδικός σφάλματος
#define HT_OK 0     // Κωδικός επιτυχίας

#define HT_BUCKET_SIZE 50  // Αριθμός εγγραφών ανά bucket block

/* Η δομή HT_info κρατάει μεταδεδομένα που σχετίζονται με το αρχείο πίνακα κατακερματισμού */
typedef struct {
    int fileDesc;               // Αναγνωριστικό αρχείου
    char fileName[256];         // Όνομα αρχείου
    int buckets;                // Αριθμός buckets
} HT_info;

/* Η δομή HT_block_info κρατάει πληροφορίες για το block του πίνακα κατακερματισμού */
typedef struct {
    int blockId;        // Αναγνωριστικό του block που περιέχει εγγραφές
    int nextBlock;      // Αναγνωριστικό του επόμενου block στην αλυσίδα (-1 αν δεν υπάρχει)
} HT_block_info;

/* Η συνάρτηση HT_CreateFile δημιουργεί ένα αρχείο πίνακα κατακερματισμού με το δοσμένο αριθμό buckets */
int HT_CreateFile(char *fileName, int buckets);

/* Η συνάρτηση HT_OpenFile ανοίγει το αρχείο πίνακα κατακερματισμού και επιστρέφει τη δομή HT_info */
HT_info* HT_OpenFile(char *fileName);

/* Η συνάρτηση HT_CloseFile κλείνει το αρχείο πίνακα κατακερματισμού και απελευθερώνει τους πόρους */
int HT_CloseFile(HT_info *header_info);

/* Η συνάρτηση HT_InsertEntry εισάγει μια εγγραφή στον πίνακα κατακερματισμού */
int HT_InsertEntry(HT_info *header_info, Record record);

/* Η συνάρτηση HT_GetAllEntries αναζητά και εκτυπώνει όλες τις εγγραφές με την τιμή του πεδίου-κλειδί value */
int HT_GetAllEntries(HT_info *header_info, int value);

#endif /* HT_TABLE_H */
