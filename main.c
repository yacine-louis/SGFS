#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// define structures
typedef struct Block Block;
typedef struct Meta Meta;
typedef struct MS MS;
typedef struct Client Client;
typedef struct Product Product;

struct Block {
     char data[256];
     int next;
     
};

struct Meta { // sizeof(Meta) = 32 Byte
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

struct Client 
{ // sizeof(Client) = 32 Byte
     int ID;
     char name[20];
     int age;
     float balance;
};

struct Product
{ // sizeof(Product) = 24 Byte
     int ID;
     char name[16];
     float price;
};

// Functions associated with virtual disk
Meta* inodes;
MS db; 

void initFileSystem() {
     // initialize informations about the MS
     db.numberOfBlocks = 256;
     db.blockSize = sizeof(Block);
     db.blockDataSize = sizeof(Block) - sizeof(int);
     db.numberOfMeta = 24;
     db.disk = malloc(db.numberOfBlocks * sizeof(Block));

     // create the file system
     FILE* MSFile;
     MSFile = fopen("database", "w+");

     // initialize the allocation table
     for (int i = 0; i < db.blockDataSize; i++)
     {
          db.disk[0].data[i] = 0;
     }

     // write the allocation table in the file
     Block buffer;
     buffer = db.disk[0];
     fwrite(&buffer, sizeof(Block), 1, MSFile);

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
          fwrite(&metabuffer, sizeof(Block), 1, MSFile);
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

     // write the disk in file
     for (int i = 1; i < db.numberOfBlocks; i++)
     {
          buffer = db.disk[i];
          fwrite(&buffer, sizeof(Block), 1, MSFile);
     }

     fclose(MSFile);
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

void createDataFile() {
     Meta inode;
     int recordType;
     printf("Enter the name of the file: ");
     scanf("%11[^\n]", inode.name);
     printf("Enter the number of records: ");
     scanf("%d", &inode.FileSizeInRecords);
     do
     {
          printf("choose the type of the record: \n[0] ->  Client \n[1] -> Product\nanswer: ");
          scanf("%d", &recordType);
     } while (recordType != 1 && recordType != 0);

     int nbrRecordsPerBlock;
     switch (recordType)
     {
     case 0:
          nbrRecordsPerBlock = db.blockDataSize / sizeof(Client);
          inode.FileSizeInBlocks = ceil((double)inode.FileSizeInRecords / nbrRecordsPerBlock);
          break;
     case 1:
          nbrRecordsPerBlock = db.blockDataSize / sizeof(Product);
          inode.FileSizeInBlocks = ceil((double)inode.FileSizeInRecords / nbrRecordsPerBlock);
          break;
     default:
          break;
     }

     do
     {
          printf("choose one of the following global organisation modes:\n[0] -> contiguous \n[1] -> chained\nanswer:");
          scanf("%d", &inode.globalOrganisationMode);
     } while (inode.globalOrganisationMode != 1 && inode.globalOrganisationMode != 0);
     
     do
     {
          printf("choose one of the following global organisation modes:\n[0] -> fixed size \n[1] -> variable size\nanswer:");
          scanf("%d", &inode.internalOrganisationMode);
     } while (inode.internalOrganisationMode != 1 && inode.internalOrganisationMode != 0);

     printf("name: %s, num records: %d, num blocks: %d, global mode: %d, internal mode: %d \n", inode.name, inode.FileSizeInRecords, inode.FileSizeInBlocks, inode.globalOrganisationMode, inode.internalOrganisationMode);
}

int findFreeBlock() {
     
}

int main() {
     initFileSystem(); // init system
     loadFileSystem(); // read the data from file system
     printFileSystem(); // print the file system
     createDataFile();
     printf("working");
     return 0;
}