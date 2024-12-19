#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define NUM_OF_BLOCKS 64
#define FB 8

// define structures
typedef struct Client Client;
typedef struct Block Block;
typedef struct Meta Meta;
typedef struct MSMeta MSMeta;
typedef struct MS MS;


struct Client 
{ 
     int ID;
     char name[20];
     int age;
     float balance;
};

struct Block {
     Client entries[FB];
     int next;
};

struct Meta {
     char name[16];
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
     int meta_per_block;
     int records_per_block;
     int first_data_region_block;
};

struct MS {
     MSMeta inode;
     FILE* disk;
};


// Global variables


void initFileSystem(MS* secondary_memory) {

     // initialize informations about the MS
     secondary_memory->inode.number_of_blocks = NUM_OF_BLOCKS;
     secondary_memory->inode.block_size = sizeof(Block);
     secondary_memory->inode.block_data_size = sizeof(Block) - sizeof(int);
     secondary_memory->inode.number_of_meta = 10;
     secondary_memory->inode.meta_per_block = secondary_memory->inode.block_data_size / sizeof(Meta);
     secondary_memory->inode.records_per_block = secondary_memory->inode.block_data_size / sizeof(Client);
     secondary_memory->inode.first_data_region_block = 1 + secondary_memory->inode.number_of_meta;
     // create the file system
     secondary_memory->disk = fopen("database", "r+");

     // initialize the allocation table
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];

     for (int i = 0; i < secondary_memory->inode.number_of_blocks; i++)
     {
          allocation_table_buffer[i] = 0;
     }
     fwrite(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);
     
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
               metabuffer[j] = inode;
               j++;
               k++;
          }
          fwrite(metabuffer, sizeof(Block), 1, secondary_memory->disk);
     }
}

void printFileSystem(MS* secondary_memory) {
     
     printf("MS informations: \n");
     printf("number of blocks: %d \n", secondary_memory->inode.number_of_blocks);
     printf("block size: %d \n", secondary_memory->inode.block_size);
     printf("block data size: %d \n", secondary_memory->inode.block_data_size);
     printf("number of metadata: %d \n", secondary_memory->inode.number_of_meta);

     rewind(secondary_memory->disk);

     printf("allocation table: \n");
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     fread(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);
     for (int i = 0; i < secondary_memory->inode.number_of_blocks; i++)
     {
          printf("%d ", allocation_table_buffer[i]);
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
}

Client GenerateClient(int id) {
     Client client;
     srand(time(NULL));

     client.ID = id;
     strcpy(client.name, "Client " + rand() % 100);
     client.age = 20 + rand() % (40 + 1); // Generate a random number between 20 and 60
     client.balance = 1000 + (rand() / (float)RAND_MAX) * (5000 - 1000); // Generate a random floating-point number between 1000 and 5000
}

int getNextBlock(MS* secondary_memory) {
     rewind(secondary_memory->disk);
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     fread(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);
}

void fillFileData(MS* secondary_memory, Meta inode) {
     int nbr_meta_blocks = secondary_memory->inode.number_of_meta / secondary_memory->inode.meta_per_block;
     fseek(secondary_memory->disk, (inode.start_block + nbr_meta_blocks + 1) * sizeof(Block), SEEK_SET);

     Block bloc;
     Block buffer;

     int k = 0;
     
     for (int i = 0; i < inode.file_size_in_blocks; i++)
     {
          int j = 0;
          while (j < secondary_memory->inode.records_per_block && k < inode.file_size_in_records)
          {
               buffer.entries[j] = GenerateClient(k);
               j++;
               k++;
          }
          fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);
     }
}

int findFreeAdjacentBlocks(MS* secondary_memory, int file_size_in_blocks) {
     if(file_size_in_blocks > secondary_memory->inode.number_of_blocks) {
          return -1;
     } else {
          rewind(secondary_memory->disk);
          int remaining_blocks_needed;
          int exist = 0;

          int *AT_buffer;
          fread(AT_buffer, sizeof(Block), 1, secondary_memory->disk);

          int i = 0;
          while (i < secondary_memory->inode.number_of_blocks && exist == 0)
          {
               if(AT_buffer[i] == 0) {
                    int start = i;
                    remaining_blocks_needed = file_size_in_blocks;
                    while(AT_buffer[i] == 0 && remaining_blocks_needed > 0) {
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

void fillMetadata(MS* secondary_memory, Meta *inode) {
     // Prompt for the file name
     do {
          printf("Enter the name of the file (max %d characters): ", sizeof(inode->name));
          scanf("%s", inode->name);

          if (strlen(inode->name) > sizeof(inode->name)) {
               printf("Error: File name must not exceed %d characters.\n", sizeof(inode->name));
          } else {
               break;
        }
     } while (1);
    
     // Prompt for the number of records
     do {
          printf("Enter the number of records (positive integer): ");
          scanf("%d", &inode->file_size_in_records);

          if (inode->file_size_in_records <= 0) {
               printf("Invalid input. Please enter a positive integer.\n");
          } else {
               break;
          }
     } while (1);

    // Calculate file size in blocks
     inode->file_size_in_blocks = (int)ceil((double)inode->file_size_in_records / secondary_memory->inode.records_per_block);

    // Prompt for the global organization mode
     do {
          printf("Choose one of the following global organization modes:\n[0] -> Contiguous\n[1] -> Chained\nAnswer: ");
          scanf("%d", &inode->global_organisation_mode);

          if (inode->global_organisation_mode != 0 && inode->global_organisation_mode != 1) {
               printf("Invalid input. Please enter 0 or 1.\n");
          } else {
               break;
          }
     } while (1);

    // Prompt for the internal organization mode
     do {
          printf("Choose one of the following internal organization modes:\n[0] -> Sorted\n[1] -> Not Sorted\nAnswer: ");
          if (scanf("%d", &inode->internal_organisation_mode) != 1 || 
               (inode->internal_organisation_mode != 0 && inode->internal_organisation_mode != 1)) {
               printf("Invalid input. Please enter 0 or 1.\n");
          } else {
               break;
          }
     } while (1);
}

void displayMetadata(Meta *inode) {
     // Display the file metadata
     printf("\nFile Metadata:\n");
     printf("Name: %s\n", inode->name);
     printf("Number of Records: %d\n", inode->file_size_in_records);
     printf("Number of Blocks: %d\n", inode->file_size_in_blocks);
     printf("Global Organization Mode: %s\n", 
          (inode->global_organisation_mode == 0) ? "Contiguous" : "Chained");
     printf("Internal Organization Mode: %s\n", 
          (inode->internal_organisation_mode == 0) ? "Sorted" : "Not Sorted");
}

void createFile(MS* secondary_memory) {
     Meta inode;

     fillMetadata(secondary_memory, &inode);
     displayMetadata(&inode);
}

void compactage(MS* secondary_memory) {
     rewind(secondary_memory->disk);
     // read the allocation table
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     fread(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);

     Block buffer;
     fseek(secondary_memory->disk, secondary_memory->inode.first_data_region_block * sizeof(Block), SEEK_SET);
     int i = 0;
     for (int j = 0; j < secondary_memory->inode.number_of_blocks; j++)
     {
          if(allocation_table_buffer[j] != 0) {
               fseek(secondary_memory->disk, (secondary_memory->inode.first_data_region_block + j) * sizeof(Block), SEEK_SET);
               fread(&buffer, sizeof(Block), 1, secondary_memory->disk);

               fseek(secondary_memory->disk, (secondary_memory->inode.first_data_region_block + i) * sizeof(Block), SEEK_SET);
               fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);

               // update the allocation table
               allocation_table_buffer[i] = allocation_table_buffer[j];
               i++;
          }
     }

     rewind(secondary_memory->disk);
     fwrite(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);
     
}

int main() {

     MS secondary_memory;
     initFileSystem(&secondary_memory); // init system
     printFileSystem(&secondary_memory); // print the file system

     int nbr_free_blocks = sizeof(secondary_memory.inode.block_data_size) - 1;

     createFile(&secondary_memory);
     // Meta inode;
     // fillFileData(&secondary_memory, inode);
     printf("working");
     return 0;
}