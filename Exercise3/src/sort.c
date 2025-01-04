#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

int compareRecords(const void* a, const void* b) {
    if(strcmp(((Record*)a)->name,((Record*)b)->name) == 0){
        if (strcmp(((Record*)a)->surname,((Record*)b)->surname) == 0) {
            return ((Record*)a)->id - ((Record*)b)->id;
        } else {
            return strcmp(((Record*)a)->surname,((Record*)b)->surname);
        };
    }
    return strcmp(((Record*)a)->name,((Record*)b)->name);
}

bool shouldSwap(Record* rec1, Record* rec2) {
    if(compareRecords(rec1,rec2) < 0) return true; // Επιστροφή true αν η rec1 πρέπει να προηγείται της rec2
    return false;
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk){

    // Δημιουργία iterator για τα chunks
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK chunk;

    chunk.blocksInChunk = numBlocksInChunk;
    chunk.file_desc = file_desc;
    chunk.from_BlockId = 1;
    chunk.to_BlockId = numBlocksInChunk;
    chunk.recordsInChunk = ((chunk.to_BlockId - chunk.from_BlockId) * MAX_RECORDS_PER_BLOCK) + HP_GetRecordCounter(file_desc, chunk.to_BlockId);
    sort_Chunk(&chunk); // Ταξινόμηση του πρώτου chunk
    
    for(int i = 1; i <= chunk.to_BlockId; i++) {
        HP_Unpin(file_desc,i);
    }
    
    // Επανάληψη για τα υπόλοιπα chunks
    while (CHUNK_GetNext(&iterator, &chunk) == 0) {
        sort_Chunk(&chunk);
        for(int i = chunk.from_BlockId; i <= chunk.to_BlockId; i++) {
            HP_Unpin(file_desc,i); // Απελευθέρωση των blocks από τον buffer
        }
    }

}

void sort_Chunk(CHUNK* chunk) {
    
    Record* records = (Record*)malloc(chunk->recordsInChunk * sizeof(Record));
    for (int i = 0; i < chunk->recordsInChunk; i++) {
        HP_GetRecord(chunk->file_desc, chunk->from_BlockId + i / MAX_RECORDS_PER_BLOCK, i % MAX_RECORDS_PER_BLOCK, &records[i]);
    }

    // Ταξινόμηση των εγγραφών με Merge Sort
    mergeSort(records, 0, chunk->recordsInChunk - 1);

    // Εγγραφή των ταξινομημένων εγγραφών πίσω στο αρχείο
    for (int i = 0; i < chunk->recordsInChunk; i++) {
        HP_UpdateRecord(chunk->file_desc, chunk->from_BlockId + i / MAX_RECORDS_PER_BLOCK, i % MAX_RECORDS_PER_BLOCK, records[i]);
    }

    // Απελευθέρωση μνήμης
    free(records);
}


// Συνάρτηση για τη συγχώνευση δύο ταξινομημένων υποπινάκων
void mergeFunc(Record* arr, int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Δημιουργία προσωρινών πινάκων
    Record* L = (Record*)malloc(n1 * sizeof(Record));
    Record* R = (Record*)malloc(n2 * sizeof(Record));

    // Αντιγραφή δεδομένων στους προσωρινούς πίνακες
    memcpy(L, &arr[left], n1 * sizeof(Record));
    memcpy(R, &arr[mid + 1], n2 * sizeof(Record));

    // Συγχώνευση των δύο υποπινάκων πίσω στον arr[]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (compareRecords(&L[i], &R[j]) <= 0) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Αντιγραφή των υπόλοιπων στοιχείων του L[], αν υπάρχουν
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Αντιγραφή των υπόλοιπων στοιχείων του R[], αν υπάρχουν
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    // Απελευθέρωση μνήμης
    free(L);
    free(R);
}

// Υλοποίηση του Merge Sort
void mergeSort(Record* arr, int left, int right) {
    if (left < right) {
        
        int mid = left + (right - left) / 2; // Υπολογισμός του μέσου σημείου για αποφυγή υπερχείλισης

        // Ταξινόμηση του πρώτου και του δεύτερου μισού
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        // Συγχώνευση των ταξινομημένων μισών
        mergeFunc(arr, left, mid, right);
    }
}