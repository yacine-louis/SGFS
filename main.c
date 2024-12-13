#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// define structures
typedef struct Block Block;
typedef struct Meta Meta;
typedef struct MSMeta MSMeta;
typedef struct MS MS;
typedef struct Client Client;
typedef struct Product Product;

struct Block {
     char data[256];
     int next;
};

struct Meta {
     char name[12];
     int start_block; // First block of the file
     int file_size_in_blocks; 
     int file_size_in_records;
     int global_organisation_mode; // 0 for contiguous, 1 for chained, -1 when initialize
     int internal_organisation_mode; // 0 for records not sorted, 1 for records sorted, -1 when initialize
};

struct MSMeta {
     int number_of_blocks;
     int block_size;
     int block_data_size;
     int number_of_meta;
     int size_of_meta;
};

struct MS {
     MSMeta inode;
     FILE* disk;
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
MS secondary_memory; 

void initFileSystem() {

     // initialize informations about the MS
     secondary_memory.inode.number_of_blocks = 256;
     secondary_memory.inode.block_size = sizeof(Block);
     secondary_memory.inode.block_data_size = sizeof(Block) - sizeof(int);
     secondary_memory.inode.number_of_meta = 10;
     secondary_memory.inode.size_of_meta = sizeof(Meta);
     // create the file system
     secondary_memory.disk = fopen("database", "w+");

     // db.disk = malloc(db.numberOfBlocks * sizeof(Block));

    
     Block bloc;
     Block buffer;
     // initialize the allocation table
     for (int i = 0; i < secondary_memory.inode.block_data_size; i++)
     {
          bloc.data[i] = 0;
     }
     buffer = bloc;
     fwrite(&buffer, sizeof(Block), 1, secondary_memory.disk);
     
     // initialize metadata
     
     Meta inode;
     int nbr_meta_per_block = secondary_memory.inode.block_data_size / sizeof(Meta);
     Meta metabuffer[nbr_meta_per_block];
     int nbr_block = ceil((double)secondary_memory.inode.number_of_meta / nbr_meta_per_block);
     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          int j = 0;
          while (j < nbr_meta_per_block && k < secondary_memory.inode.number_of_meta)
          {
               strcpy(inode.name, "hello");
               inode.start_block = -1;
               inode.file_size_in_blocks = 0;
               inode.file_size_in_records = 0;
               inode.global_organisation_mode = -1;
               inode.internal_organisation_mode = -1;
               metabuffer[j] = inode;
               j++;
               k++;
          }
          fwrite(metabuffer, sizeof(Block), 1, secondary_memory.disk);
     }
     
     // initialize the disk blocks
     for (int i = 1; i < secondary_memory.inode.number_of_blocks; i++)
     {
          for (int j = 0; j < secondary_memory.inode.block_data_size; j++)
          {
               bloc.data[j] = 0;
          }
          bloc.next = -1;
          buffer = bloc;
          fwrite(&buffer, sizeof(Block), 1, secondary_memory.disk);
     }

}

void printFileSystem() {

     printf("MS informations: \n");
     printf("number of blocks: %d \n", secondary_memory.inode.number_of_blocks);
     printf("block size: %d \n", secondary_memory.inode.block_size);
     printf("block data size: %d \n", secondary_memory.inode.block_data_size);
     printf("number of metadata: %d \n", secondary_memory.inode.number_of_meta);

     Block buffer;

     secondary_memory.disk = fopen("database", "r");
     rewind(secondary_memory.disk);

     printf("allocation table: \n");
     fread(&buffer, sizeof(Block), 1, secondary_memory.disk);
     for (int i = 0; i < secondary_memory.inode.block_data_size; i++)
     {
          printf("%d ", buffer.data[i]);
     }
     printf("\n");

     printf("meta data: \n");
     int nbr_meta_per_block = secondary_memory.inode.block_data_size / sizeof(Meta);
     Meta metabuffer[nbr_meta_per_block];
     int nbr_block = ceil((double)secondary_memory.inode.number_of_meta / nbr_meta_per_block);

     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          fread(metabuffer, sizeof(Block), 1, secondary_memory.disk);
          int j = 0;
          while (j < nbr_meta_per_block && k < secondary_memory.inode.number_of_meta)
          {
               printf("file name: %s, startBlock: %d, FileSizeInBlocks: %d, fileSizeInRecords: %d, globalOrganisationMode: %d, internalOrganisationMode: %d", metabuffer->name, metabuffer->start_block, metabuffer->file_size_in_blocks, metabuffer->file_size_in_records, metabuffer->global_organisation_mode, metabuffer->internal_organisation_mode);
               printf("\n");
               j++;
               k++;
          }
     }
     fseek(secondary_memory.disk, (nbr_block + 1) * sizeof(Block), SEEK_SET);
     printf("blocks: \n");
     for (int i = 1; i < secondary_memory.inode.number_of_blocks; i++)
     {
          fread(&buffer, sizeof(Block), 1, secondary_memory.disk);
          printf("blocks number: %d, next block number: %d \n", i, buffer.next);
     }
}

void createFile() {
     Meta inode;
     int recordType;
     printf("Enter the name of the file: ");
     scanf("%11[^\n]", inode.name);
     printf("Enter the number of records: ");
     scanf("%d", &inode.file_size_in_records);
     do
     {
          printf("choose the type of the record: \n[0] ->  Client \n[1] -> Product\nanswer: ");
          scanf("%d", &recordType);
     } while (recordType != 0 && recordType != 1);

     int nbr_records_per_block;
     if(recordType == 0) {
          nbr_records_per_block = secondary_memory.inode.block_data_size / sizeof(Client); 
     } else {
          nbr_records_per_block = secondary_memory.inode.block_data_size / sizeof(Product);
     }
     inode.file_size_in_blocks = ceil((double)inode.file_size_in_records / nbr_records_per_block);

     do
     {
          printf("choose one of the following global organisation modes:\n[0] -> contiguous \n[1] -> chained\nanswer:");
          scanf("%d", &inode.global_organisation_mode);
     } while (inode.global_organisation_mode != 0 && inode.global_organisation_mode != 1);
     
     do
     {
          printf("choose one of the following internal organisation modes:\n[0] -> sorted \n[1] -> not sorted\nanswer:");
          scanf("%d", &inode.internal_organisation_mode);
     } while (inode.internal_organisation_mode != 0 && inode.internal_organisation_mode != 1);

     printf("name: %s, num records: %d, num blocks: %d, global mode: %d, internal mode: %d \n", inode.name, inode.file_size_in_records, inode.file_size_in_blocks, inode.global_organisation_mode, inode.internal_organisation_mode);
}

int findFreeBlock() {
     
}

int main() {
     initFileSystem(); // init system
     printFileSystem(); // print the file system
     createFile();
     printf("working");
     return 0;
}