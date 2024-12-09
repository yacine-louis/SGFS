#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
     int numberOfMeta;
     Block* disk;
};

// Functions associated with virtual disk
Meta* inodes;
MS db; 

void initFileSystem() {
     // initialize informations about the file system;
     db.numberOfBlocks = 256;
     db.blockSize = sizeof(Block) - sizeof(int);
     db.numberOfMeta = 20;

     // initialize metadata
     inodes = malloc(db.numberOfMeta * sizeof(Meta));
     for (int i = 0; i < db.numberOfMeta; i++)
     {
          srtcpy(inodes[i].name, "");
          inodes[i].startBlock = -1;
          inodes[i].FileSizeInBlocks = 0;
          inodes[i].FileSizeInRecords = 0;
          inodes[i].globalOrganisationMode = -1;
          inodes[i].internalOrganisationMode = -1;
     }

     // initialize MS
     db.disk = malloc(db.numberOfBlocks * sizeof(Block));
     // initialize the allocation table
     for (int i = 0; i < db.blockSize; i++)
     {
          db.disk[0].data[i] = 0;
     }

     // initialize the disk
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          for (int j = 0; j < db.blockSize; j++)
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

     // write the allocation table in the file
     Block buffer;
     buffer = db.disk[0];
     fwrite(&buffer, sizeof(Block), 1, file);
     fclose(file);

     // write the metadata in the file
     Meta metabuffer;
     for (int i = 0; i < db.numberOfMeta; i++)
     {
          metabuffer = inodes[i];
          fwrite(&metabuffer, sizeof(Meta), 1, file);
     }

     // write the disk in file
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          buffer = db.disk[i];
          fwrite(&buffer, sizeof(Block), 1, file);
     }
     
}




int main() {

     return 0;
}