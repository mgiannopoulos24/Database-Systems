#include <merge.h>
#include <stdio.h>
#include "chunk.h"
#include "record.h"
#define MAX_RECORDS_PER_BLOCK (BF_BLOCK_SIZE / sizeof(Record))

CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;
    iterator.file_desc = fileDesc;
    iterator.current = 1;  // Ξεκινάμε από το block 1, αφού το block 0 είναι για μεταδεδομένα
    iterator.lastBlocksID = blocksInChunk; 
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    int blockNum = HP_GetIdOfLastBlock(iterator->file_desc); // Αριθμός τελευταίου block στο αρχείο

    // Ενημέρωση του iterator για το επόμενο chunk
    iterator->current += iterator->blocksInChunk;   
    iterator->lastBlocksID += iterator->blocksInChunk; 

    if(iterator->current > blockNum) {
        return -1; // Τέλος αρχείου
    }

    // Προσαρμογή του τελευταίου chunk αν υπερβαίνει το μέγεθος του αρχείου
    if(iterator->lastBlocksID >= blockNum) {
        iterator->lastBlocksID = blockNum;
        chunk->to_BlockId = blockNum;
        
    }
    iterator->blocksInChunk = iterator->lastBlocksID - iterator->current + 1;


    // Ανάθεση πληροφοριών από το iterator στο chunk
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->lastBlocksID;
    chunk->recordsInChunk = ((chunk->to_BlockId - chunk->from_BlockId) * MAX_RECORDS_PER_BLOCK) + HP_GetRecordCounter(iterator->file_desc, iterator->lastBlocksID);
    chunk->blocksInChunk = iterator->blocksInChunk;

    return 0;
}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){
    if (i < 0 || i >= chunk->recordsInChunk) {
        return -1;  // Μη έγκυρη θέση
    }

    // Υπολογισμός του block και της θέσης της εγγραφής μέσα στο block
    int blockId = chunk->from_BlockId + i / MAX_RECORDS_PER_BLOCK;
    int recordPosInBlock = i % MAX_RECORDS_PER_BLOCK;


    int file_desc = chunk->file_desc;
    int result = HP_GetRecord(file_desc, blockId, recordPosInBlock, record);


    HP_Unpin(file_desc,blockId);
    return result;  // Επιστροφή του αποτελέσματος
}

int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){
    if (i < 0 || i >= chunk->recordsInChunk) {
        return -1;  // Μη έγκυρη θέση
    }

    // Υπολογισμός του block και της θέσης της εγγραφής μέσα στο block
    int blockId = chunk->from_BlockId + i / MAX_RECORDS_PER_BLOCK;
    int recordPosInBlock = i % MAX_RECORDS_PER_BLOCK;

    int file_desc = chunk->file_desc;
    int result = HP_UpdateRecord(file_desc, blockId, recordPosInBlock, record);
    HP_Unpin(file_desc,blockId);

    return result;  // Επιστροφή του αποτελέσματος του HP_UpdateRecord
}


void CHUNK_Print(CHUNK chunk) {
    printf("Printing records in Chunk from Block %d to Block %d:\n", chunk.from_BlockId, chunk.to_BlockId);

    // Εκτύπωση όλων των εγγραφών στο chunk
    for (int i = 0; i < chunk.recordsInChunk; i++) {
        Record record;
        if (CHUNK_GetIthRecordInChunk(&chunk, i, &record) == 0) {
            printRecord(record);
        }
    }

    printf("End of Chunk Print\n");
}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator iterator;

    // Αρχικοποίηση του iterator για τις εγγραφές του chunk
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;

    return iterator;
}


int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record* record) {
    // Έλεγχος αν ο cursor έχει φτάσει στο τέλος του τρέχοντος block
    if (iterator->cursor > MAX_RECORDS_PER_BLOCK - 1) {
        // Μετακίνηση στο επόμενο block
        iterator->currentBlockId++;
        iterator->cursor = 0;
    }

    if (iterator->currentBlockId > iterator->chunk.to_BlockId) {
        return -1;  // Τέλος του iterator
    }

    // Ανάκτηση της εγγραφής
    if (HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record) != 0) {
        return -1;  // Σφάλμα κατά την ανάκτηση
    }

    // Απελευθέρωση της μνήμης του block
    HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);

    // Μετακίνηση του cursor στην επόμενη εγγραφή
    iterator->cursor++;

    return 0;  // Επιτυχής ανάκτηση
}