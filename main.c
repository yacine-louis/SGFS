#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// define structures
typedef struct Block Block;
typedef struct Meta Meta;
typedef struct Virtualdisk Virtualdisk;

struct Block {
     char data[256];
     int next;
};

struct Meta {
     char name[16];
     int startBlock; // First block of the file
     int FileSizeInBlocks; // File size in blocks
     int FileSizeInRecords; // File size in records
     int globalOrganisationMode; // 0 for contiguous, 1 for chained, -1 if user havn't choosen yet
     int internalOrganisationMode; // 0 for records not sorted, 1 for records sorted, -1 if user havn't choosen yet
};

struct Virtualdisk {
     int numberOfBlocks;
     int blockSize;
     int numberOfMeta;
     Block* disk;
};



int main() {

     return 0;
}