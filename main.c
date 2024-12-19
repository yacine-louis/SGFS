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
     char name[16];
     int start_block; // First block of the file
     int file_size_in_blocks; 
     int file_size_in_records;
     int global_organisation_mode; // 0 for contiguous, 1 for chained, -1 when initialize
     int internal_organisation_mode; // 0 for records not sorted, 1 for records sorted, -1 when initialize
     int records_per_block;
};

struct MSMeta {
     int number_of_blocks;
     int block_size;
     int block_data_size;
     int number_of_meta;
     int meta_per_block;
};

struct MS {
     MSMeta inode;
     FILE* disk;
};

struct Client 
{ 
     int ID;
     char name[20];
     int age;
     float balance;
};

// Functions associated with virtual disk
 

void initFileSystem(MS* secondary_memory) {

     // initialize informations about the MS
     secondary_memory->inode.number_of_blocks = 256;
     secondary_memory->inode.block_size = sizeof(Block);
     secondary_memory->inode.block_data_size = sizeof(Block) - sizeof(int);
     secondary_memory->inode.number_of_meta = 10;
     secondary_memory->inode.meta_per_block = secondary_memory->inode.block_data_size / sizeof(Meta);
     // create the file system
     secondary_memory->disk = fopen("database", "w+");

     Block bloc;
     Block buffer;
     // initialize the allocation table
     for (int i = 0; i < secondary_memory->inode.block_data_size; i++)
     {
          bloc.data[i] = 0;
     }
     buffer = bloc;
     fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);
     
     // initialize metadata
     Meta inode;
     Meta metabuffer[secondary_memory->inode.meta_per_block];
     int nbr_block = ceil((double)secondary_memory->inode.number_of_meta / secondary_memory->inode.meta_per_block);
     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          int j = 0;
          while (j < secondary_memory->inode.meta_per_block && k < secondary_memory->inode.number_of_meta)
          {
               strcpy(inode.name, "hello");
               inode.start_block = -1;
               inode.file_size_in_blocks = 0;
               inode.file_size_in_records = 0;
               inode.global_organisation_mode = -1;
               inode.internal_organisation_mode = -1;
               inode.records_per_block = 0;
               metabuffer[j] = inode;
               j++;
               k++;
          }
          fwrite(metabuffer, sizeof(Block), 1, secondary_memory->disk);
     }
     
     // initialize the disk blocks
     for (int i = 1; i < secondary_memory->inode.number_of_blocks; i++)
     {
          for (int j = 0; j < secondary_memory->inode.block_data_size; j++)
          {
               bloc.data[j] = 0;
          }
          bloc.next = -1;
          buffer = bloc;
          fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);
     }

}

void printFileSystem(MS* secondary_memory) {

     printf("MS informations: \n");
     printf("number of blocks: %d \n", secondary_memory->inode.number_of_blocks);
     printf("block size: %d \n", secondary_memory->inode.block_size);
     printf("block data size: %d \n", secondary_memory->inode.block_data_size);
     printf("number of metadata: %d \n", secondary_memory->inode.number_of_meta);

     Block buffer;

     secondary_memory->disk = fopen("database", "r");
     rewind(secondary_memory->disk);

     printf("allocation table: \n");
     fread(&buffer, sizeof(Block), 1, secondary_memory->disk);
     for (int i = 0; i < secondary_memory->inode.block_data_size; i++)
     {
          printf("%d ", buffer.data[i]);
     }
     printf("\n");

     printf("meta data: \n");
     Meta metabuffer[secondary_memory->inode.meta_per_block];
     int nbr_block = ceil((double)secondary_memory->inode.number_of_meta / secondary_memory->inode.meta_per_block);

     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          fread(metabuffer, sizeof(Block), 1, secondary_memory->disk);
          int j = 0;
          while (j < secondary_memory->inode.meta_per_block && k < secondary_memory->inode.number_of_meta)
          {
               printf("file name: %s, startBlock: %d, FileSizeInBlocks: %d, fileSizeInRecords: %d, globalOrganisationMode: %d, internalOrganisationMode: %d", metabuffer->name, metabuffer->start_block, metabuffer->file_size_in_blocks, metabuffer->file_size_in_records, metabuffer->global_organisation_mode, metabuffer->internal_organisation_mode);
               printf("\n");
               j++;
               k++;
          }
     }
     fseek(secondary_memory->disk, (nbr_block + 1) * sizeof(Block), SEEK_SET);
     printf("blocks: \n");
     for (int i = 1; i < secondary_memory->inode.number_of_blocks; i++)
     {
          fread(&buffer, sizeof(Block), 1, secondary_memory->disk);
          printf("blocks number: %d, next block number: %d \n", i, buffer.next);
     }
}

int findFreeAdjacentBlocks(MS* secondary_memory, int file_size_in_blocks) {
     if(file_size_in_blocks > secondary_memory->inode.number_of_blocks) {
          return -1;
     } else {
          int remaining_blocks_needed;
          int exist = 0;
          rewind(secondary_memory->disk);
          Block buffer;
          fread(&buffer, sizeof(Block), 1, secondary_memory->disk);

          int i = 0;
          while (i < secondary_memory->inode.block_data_size && exist == 0)
          {
               if(buffer.data[i] == 0) {
                    int start = i;
                    remaining_blocks_needed = file_size_in_blocks;
                    while(buffer.data[i] == 0 && remaining_blocks_needed > 0) {
                         i++;
                         remaining_blocks_needed--;
                    }

                    if(remaining_blocks_needed == 0) {
                         exist = 1;
                         return start;
                    }
               } else {
                    i++;
               }
          }
          
          if(exist = 0) {
               // propose Compactage
               return -1;
          }
     }
}

void createFile(MS* secondary_memory) {
     Meta inode;

     // Prompt for the file name
     do {
          printf("Enter the name of the file (max %d characters): ", sizeof(inode.name));
          scanf("%s", inode.name);

          // Clear input buffer to remove leftover characters

          if (strlen(inode.name) > sizeof(inode.name)) {
               printf("Error: File name must not exceed %d characters.\n", sizeof(inode.name));
          } else {
               break;
        }
     } while (1);
    
     // Prompt for the number of records
     do {
          printf("Enter the number of records (positive integer): ");
          if (scanf("%d", &inode.file_size_in_records) != 1 || inode.file_size_in_records <= 0) {
               printf("Invalid input. Please enter a positive integer.\n");
          } else {
               break;
          }
     } while (1);

    // Calculate the number of records per block 
     inode.records_per_block = secondary_memory->inode.block_data_size / sizeof(Client);
     

     inode.file_size_in_blocks = (int)ceil((double)inode.file_size_in_records / inode.records_per_block);

    // Prompt for the global organization mode
     do {
          printf("Choose one of the following global organization modes:\n[0] -> Contiguous\n[1] -> Chained\nAnswer: ");
          if (scanf("%d", &inode.global_organisation_mode) != 1 || 
               (inode.global_organisation_mode != 0 && inode.global_organisation_mode != 1)) {
               printf("Invalid input. Please enter 0 or 1.\n");
          } else {
               break;
          }
     } while (1);

    // Prompt for the internal organization mode
    do {
        printf("Choose one of the following internal organization modes:\n[0] -> Sorted\n[1] -> Not Sorted\nAnswer: ");
        if (scanf("%d", &inode.internal_organisation_mode) != 1 || 
            (inode.internal_organisation_mode != 0 && inode.internal_organisation_mode != 1)) {
            printf("Invalid input. Please enter 0 or 1.\n");
        } else {
            break;
        }
    } while (1);

    // Display the file metadata
    printf("\nFile Metadata:\n");
    printf("Name: %s\n", inode.name);
    printf("Number of Records: %d\n", inode.file_size_in_records);
    printf("Number of Blocks: %d\n", inode.file_size_in_blocks);
    printf("Global Organization Mode: %s\n", 
           (inode.global_organisation_mode == 0) ? "Contiguous" : "Chained");
    printf("Internal Organization Mode: %s\n", 
           (inode.internal_organisation_mode == 0) ? "Sorted" : "Not Sorted");
}


void fillFileData(MS* secondary_memory, Meta inode) {
     fseek(secondary_memory->disk, inode.start_block * sizeof(Block), SEEK_SET);
     
     Client c;
     Client bufferC[inode.records_per_block];
     int k = 0;
     srand(time(NULL));
     Block bloc;

     for (int i = 0; i < inode.file_size_in_blocks; i++)
     {
          int j = 0;
          while (j < inode.records_per_block && k < inode.file_size_in_records)
          {
               c.ID = k;
               strcpy(c.name, "Client " + i);
               c.age = 20 + rand() % (50 + 1); // Generate a random number between 20 and 70
               c.balance = 1000 + (rand() / (float)RAND_MAX) * (5000 - 1000); // Generate a random floating-point number between 1000 and 5000
               bufferC[j] = c;
               j++;
               k++;
          }
          fwrite(bufferC, sizeof(Block), 1, secondary_memory->disk);
          fseek(secondary_memory->disk, -sizeof(Block), SEEK_CUR);
          fread(&bloc, sizeof(Block), 1, secondary_memory->disk);
          printf("%d \n",bloc.next);
     }
     

}



int main() {

     MS secondary_memory;
     initFileSystem(&secondary_memory); // init system
     printFileSystem(&secondary_memory); // print the file system

     int nbr_free_blocks = sizeof(secondary_memory.inode.block_data_size) - 1;

     createFile(&secondary_memory);
     Meta inode;
     fillFileData(&secondary_memory, inode);
     printf("working");
     return 0;
}