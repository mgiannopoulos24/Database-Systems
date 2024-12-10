#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "bp_file.h"
#include "bp_datanode.h"
#include "bp_indexnode.h"

#define RECORDS_NUM 200 // you can change it if you want
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)         \
	{                             \
		BF_ErrorCode code = call; \
		if (code != BF_OK)        \
		{                         \
			BF_PrintError(code);  \
			exit(code);           \
		}                         \
	}

void insertEntries();
void findEntries();

int main()
{

	insertEntries();
	findEntries();

	////////////////////////////////////////////////
}

void insertEntries()
{
	BF_Init(LRU);
	BP_CreateFile(FILE_NAME);
	int file_desc;
	BPLUS_INFO *info = BP_OpenFile(FILE_NAME, &file_desc);
	Record record;
	for (int i = 0; i < RECORDS_NUM; i++)
	{
		record = randomRecord();
		BP_InsertEntry(file_desc, info, record);
	}
	record.id = 161;
	strcpy(record.name, "Dimitrios");
	strcpy(record.surname, "Skondras");
	strcpy(record.city, "Piraeus");

	BP_InsertEntry(file_desc, info, record);
	BP_CloseFile(file_desc, info);
	BF_Close();
}

void findEntries()
{
	int file_desc;
	BPLUS_INFO *info;

	BF_Init(LRU);
	info = BP_OpenFile(FILE_NAME, &file_desc);

	Record tmpRec; // Αντί για malloc
	Record *result = &tmpRec;

	int id = 159;
	int id2 = 161;
	int id3 = 1000;
	printf("Searching for: %d\n", id);
	if (BP_GetEntry(file_desc, info, id, &result) == -1) {
		printf("Cannot find an entry for id = %d\n", id);
	} else {
		if (result != NULL)
			printRecord(*result);
	}
	printf("Searching for: %d\n", id2);
	if (BP_GetEntry(file_desc, info, id2, &result) == -1) {
		printf("Cannot find an entry for id = %d\n", id2);
	} else {
		if (result != NULL)
			printRecord(*result);
	}
	printf("Searching for: %d\n", id3);
	if (BP_GetEntry(file_desc, info, id3, &result) == -1) {
		printf("Cannot find an entry for id = %d\n", id3);
	} else {
		if (result != NULL)
			printRecord(*result);
	}

	BP_CloseFile(file_desc, info);
	BF_Close();
}
