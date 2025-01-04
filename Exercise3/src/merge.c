#include "merge.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "chunk.h"  // Include the necessary header file for CHUNK operations


void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    int totalBlocks = HP_GetIdOfLastBlock(input_FileDesc); 
    int totalChunks = (int)ceil((double)totalBlocks / chunkSize); // Αριθμός chunks
    int newFileChunkNum = (int)ceil((double)totalChunks / bWay);
    int newFileChunkSize = chunkSize * bWay; // Μέγεθος κάθε chunk στο νέο αρχείο
    int totalRecords = 0;

    CHUNK_Iterator chunk_iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    CHUNK chunk;    

    // Αρχικοποίηση των iterators για κάθε chunk
    CHUNK_RecordIterator rec_iterators[totalChunks];
    Record records[bWay];

    // Δημιουργία του πρώτου chunk και του iterator για τις εγγραφές
    chunk.blocksInChunk = chunkSize;
    chunk.file_desc = input_FileDesc;
    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.recordsInChunk = MAX_RECORDS_PER_BLOCK * chunkSize;
    rec_iterators[0] = CHUNK_CreateRecordIterator(&chunk);
    totalRecords += chunk.recordsInChunk;


    // Δημιουργία των iterators για τα υπόλοιπα chunks
    for (int i = 1; i < totalChunks; i++) {

        if (CHUNK_GetNext(&chunk_iterator, &chunk) != 0) {
            printf("Error retrieving chunk!\n");
            return;
        };
        rec_iterators[i] = CHUNK_CreateRecordIterator(&chunk);
        totalRecords += chunk.recordsInChunk;
    }

    // Λογική συγχώνευσης
    int insertedRecords = 0;
    for(int k = 0; k < newFileChunkNum; k++) {  // Για κάθε chunk στο νέο αρχείο
        for(int j = 0; j < newFileChunkSize * MAX_RECORDS_PER_BLOCK; j++) { // Για κάθε εγγραφή στο chunk
            if(insertedRecords == totalRecords) {
                break;
            }

            if(j == 0) { // Φόρτωση των νέων chunks για b-way συγχώνευση
                for (int i = 0; i < bWay; i++) {
                    if(i + (k*bWay) >= totalChunks) {
                        Record dummy;
                        dummy.id = -1;
                        records[i] = dummy;
                        continue;
                    }
                    if (CHUNK_GetNextRecord(&rec_iterators[i + (k*bWay)], &records[i]) != 0) {
                        return;
                    }
                }
            }

            // Εύρεση της μικρότερης εγγραφής μεταξύ των φορτωμένων
            int minIndex = -1;
            for (int i = 0; i < bWay; i++) {
                if(records[i].id == -1) continue;
                if (minIndex == -1 || shouldSwap(&records[i], &records[minIndex])) {
                    minIndex = i;
                }            
            }

            // Εισαγωγή της εγγραφής στο νέο αρχείο
            if(HP_InsertEntry(output_FileDesc,records[minIndex]) == -1) {
                printf("Error inserting entry. Returning.\n");
                return;
            }
            insertedRecords++;

            // Απελευθέρωση μνήμης
            HP_Unpin(output_FileDesc,HP_GetIdOfLastBlock(output_FileDesc));

            // Ανάκτηση της επόμενης εγγραφής από το chunk. Αν τελειώσει το chunk, αντικατάσταση με dummy record
            if (CHUNK_GetNextRecord(&rec_iterators[minIndex + (k*bWay)], &records[minIndex]) != 0) {
                Record dummy;
                dummy.id = -1;
                records[minIndex] = dummy;

                // Απελευθέρωση μνήμης του chunk αν έχει φτάσει στο τέλος
                for(int i = rec_iterators[minIndex + (k*bWay)].chunk.from_BlockId; i <= rec_iterators[minIndex + (k*bWay)].chunk.to_BlockId; i++) {
                    HP_Unpin(input_FileDesc,i);
                }
            }
        }
    }
}