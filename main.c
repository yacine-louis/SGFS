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
          db.disk[i].next = -1;
     }
     
}

// functions associated with data files
void syncFileSystem() {
     // create the file
     FILE* file;
     file = fopen("database", "w+");
     rewind(file);

     // write the allocation table in the file
     Block buffer;
     buffer = db.disk[0];
     fwrite(&buffer, sizeof(buffer), 1, file);

     // write the metadata in the file
     int nbrMetaPerBlock = db.blockDataSize / sizeof(Meta);
     Meta metabuffer[nbrMetaPerBlock];
     int nbrBlock = ceil((double)db.numberOfMeta / nbrMetaPerBlock);
     int k = 0;
     int j;
     for (int i = 0; i < nbrBlock; i++)
     {
          j = 0;
          while(j < nbrMetaPerBlock && k < db.numberOfMeta) {
               metabuffer[j] = inodes[k];
               j++;
               k++;
          }
          fwrite(&metabuffer, sizeof(metabuffer) + sizeof(int), 1, file);
     }

     // write the disk in file
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          buffer = db.disk[i];
          fwrite(&buffer, sizeof(Block), 1, file);
     }

     fclose(file);
}

void loadFileSystem() {
     FILE* file;
     file = fopen("database", "r");
     rewind(file);

     // read allocation table from the file
     Block buffer;
     fread(&buffer, sizeof(buffer), 1, file);
     db.disk[0] = buffer;

     // read meta data from the file
     int nbrMetaPerBlock = db.blockDataSize / sizeof(Meta);
     Meta metabuffer[nbrMetaPerBlock];
     int nbrBlock = ceil((double)db.numberOfMeta / nbrMetaPerBlock);
     int k = 0;
     int j;
     for (int i = 0; i < nbrBlock; i++)
     {
          fread(&metabuffer, sizeof(metabuffer) + sizeof(int), 1, file);
          j = 0;
          while(j < nbrMetaPerBlock && k < db.numberOfMeta) {
               inodes[k] = metabuffer[j];
               j++;
               k++;
          }
     }

     // read data blocks from the file
     for (int i = 0; i < db.numberOfBlocks; i++)
     {
          fread(&buffer, sizeof(buffer), 1, file);
          db.disk[i] = buffer;
     }
     
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
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          printf("blocks number: %d, next block number: %d \n", i, db.disk[i].next);
     }
}


int main() {
     initFileSystem(); // init system
     syncFileSystem(); // write the data to system
     loadFileSystem();
     printFileSystem();
     printf("working");
     return 0;
}