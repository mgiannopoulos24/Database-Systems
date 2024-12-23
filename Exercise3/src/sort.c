#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

/* Determines if two records should be swapped during sorting, returning true if rec1 > rec2 based on name and surname. */
bool shouldSwap(Record* rec1, Record* rec2) {
    int name_cmp = strcmp(rec1->name, rec2->name);
    if (name_cmp > 0) {
        return true;
    } else if (name_cmp == 0) {
        return strcmp(rec1->surname, rec2->surname) > 0;
    }
    return false;
}

/* Sorts the contents of a file identified by 'file_desc' in chunks, where each chunk contains 'numBlocksInChunk' blocks. */
void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    int lastBlockId = HP_GetIdOfLastBlock(file_desc);
    if (lastBlockId < 0) {
        fprintf(stderr, "Error retrieving the last block ID.\n");
        return;
    }

    int recordsPerBlock = HP_GetMaxRecordsInBlock(file_desc);
    if (recordsPerBlock <= 0) {
        fprintf(stderr, "Error retrieving records per block.\n");
        return;
    }

    for (int startBlockId = 1; startBlockId <= lastBlockId; startBlockId += numBlocksInChunk) {
        int endBlockId = startBlockId + numBlocksInChunk - 1;
        if (endBlockId > lastBlockId) {
            endBlockId = lastBlockId;
        }

        CHUNK chunk = {
            .file_desc = file_desc,
            .from_BlockId = startBlockId,
            .to_BlockId = endBlockId,
            .recordsInChunk = (endBlockId - startBlockId + 1) * recordsPerBlock,
            .blocksInChunk = endBlockId - startBlockId + 1
        };

        sort_Chunk(&chunk);
    }
}

/* Sorts records within a CHUNK in ascending order based on the name and surname of each person. */
void sort_Chunk(CHUNK* chunk) {
    for (int blockId = chunk->from_BlockId; blockId <= chunk->to_BlockId; blockId++) {
        for (int i = 0; i < chunk->recordsInChunk - 1; i++) {
            for (int j = 0; j < chunk->recordsInChunk - i - 1; j++) {
                Record rec1, rec2;

                if (HP_GetRecord(chunk->file_desc, blockId, j, &rec1) != 0 ||
                    HP_GetRecord(chunk->file_desc, blockId, j + 1, &rec2) != 0) {
                    fprintf(stderr, "Error reading records from block %d.\n", blockId);
                    return;
                }

                if (shouldSwap(&rec1, &rec2)) {
                    if (HP_UpdateRecord(chunk->file_desc, blockId, j, rec2) != 1 ||
                        HP_UpdateRecord(chunk->file_desc, blockId, j + 1, rec1) != 1) {
                        fprintf(stderr, "Error updating records in block %d.\n", blockId);
                        return;
                    }
                }
            }
        }
    }
}