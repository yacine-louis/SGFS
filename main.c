#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// define structures
typedef struct Block Block;
typedef struct Meta Meta;
typedef struct MS MS;

struct Block {
     char data[256];
     int next;
};

struct Meta {
     char name[12];
     int startBlock; // First block of the file
     int FileSizeInBlocks; // File size in blocks
     int FileSizeInRecords; // File size in records
     int globalOrganisationMode; // 0 for contiguous, 1 for chained, -1 if user havn't choosen yet
     int internalOrganisationMode; // 0 for records not sorted, 1 for records sorted, -1 if user havn't choosen yet
};

struct MS {
     int numberOfBlocks;
     int blockSize;
     int blockDataSize;
     int numberOfMeta;
     Block* disk;
};

// Functions associated with virtual disk
Meta* inodes;
MS db; 

void initFileSystem() {
     // initialize informations about the file system;
     db.numberOfBlocks = 256;
     db.blockSize = sizeof(Block);
     db.blockDataSize = sizeof(Block) - sizeof(int);
     db.numberOfMeta = 24;

     // initialize metadata
     inodes = malloc(db.numberOfMeta * sizeof(Meta));
     for (int i = 0; i < db.numberOfMeta; i++)
     {
          strcpy(inodes[i].name, "hello");
          inodes[i].startBlock = -1;
          inodes[i].FileSizeInBlocks = 0;
          inodes[i].FileSizeInRecords = 0;
          inodes[i].globalOrganisationMode = -1;
          inodes[i].internalOrganisationMode = -1;
     }

     // initialize MS
     db.disk = malloc(db.numberOfBlocks * sizeof(Block));
     // initialize the allocation table
     for (int i = 0; i < db.blockDataSize; i++)
     {
          db.disk[0].data[i] = 0;
     }

     // initialize the disk
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          for (int j = 0; j < db.blockDataSize; j++)
          {
               db.disk[i].data[j] = 0;
          }
     }
     
}

// functions associated with data files
void syncFileSystem() {
     // create the file
     FILE* file;
     file = fopen("database", "w+");
     rewind(file);

     int nbrBlock = 0;
     // write the allocation table in the file
     Block buffer;
     buffer = db.disk[0];
     fwrite(&buffer, sizeof(Block), 1, file);
     nbrBlock++;

     // write the metadata in the file
     Meta metabuffer;
     for (int i = 0; i < db.numberOfMeta; i++)
     {
          metabuffer = inodes[i];
          fwrite(&metabuffer, sizeof(Meta), 1, file);
     }
     int nbrMetaPerBlock = db.blockDataSize / sizeof(Meta);
     nbrBlock += ceil((double)db.numberOfMeta / nbrMetaPerBlock);

     // write the disk in file
     fseek(file, nbrBlock*db.blockSize, SEEK_SET);
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          buffer = db.disk[i];
          fwrite(&buffer, sizeof(Block), 1, file);
     }

     fclose(file);
}

void loadFileSystem() {
     
}

void printFileSystem() {

     printf("MS informations: \n");
     printf("number of blocks: %d \n", db.numberOfBlocks);
     printf("block size: %d \n", db.blockSize);
     printf("block data size: %d \n", db.blockDataSize);
     printf("number of metadata: %d \n", db.numberOfMeta);

     printf("allocation table: \n");
     for (int i = 0; i < db.blockDataSize; i++)
     {
          printf("%d ", db.disk[0].data[i]);
     }
     printf("\n");

     printf("meta data: \n");
     for (int i = 0; i < db.numberOfMeta; i++)
     {
          printf("file name: %s, startBlock: %d, FileSizeInBlocks: %d, fileSizeInRecords: %d, globalOrganisationMode: %d, internalOrganisationMode: %d", inodes[i].name, inodes[i].startBlock, inodes[i].FileSizeInBlocks, inodes[i].FileSizeInRecords, inodes[i].globalOrganisationMode, inodes[i].internalOrganisationMode);
          printf("\n");
     }
     
     printf("blocks: \n");
     for (int i = 0; i < db.numberOfBlocks; i++)
     {
          printf("blocks number: %d, next block number: %d", i, db.disk[i].next);
     }
}


int main() {
     initFileSystem(); // init system
     syncFileSystem(); // write the data to system

     printf("working");
     return 0;
}