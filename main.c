#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_OF_BLOCKS 64
#define FB 8
// define structures
/* -------------------------------------------------- */
typedef struct Client Client;
typedef struct Block Block;
typedef struct Meta Meta;
typedef struct MSMeta MSMeta;
typedef struct MS MS;
typedef struct Position Position;

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

struct Position {
     int block_index;
     int index;
};
/* -------------------------------------------------- */

// functions
/* -------------------------------------------------- */
void initFileSystem(MS* secondary_memory);
void printFileSystem(MS* secondary_memory);
void readAllocationTable(MS* secondary_memory, int allocation_table_buffer[]);
void writeAllocationTable(MS* secondary_memory, int allocation_table_buffer[]);
Client generateClient(int id);
void fillFileData(MS* secondary_memory, Meta inode);
int findFreeAdjacentBlock(MS* secondary_memory, int file_size_in_blocks);
int findFreeChainedBlock(MS* secondary_memory, int file_size_in_block);
void fillMetadata(MS* secondary_memory, Meta *inode);
void displayMetadata(Meta *inode);
void createFile(MS* secondary_memory);
void compactage(MS* secondary_memory);
Position searchMetadata(MS* secondary_memory, char name[]);
void EmptyDisk(MS* secondary_memory);
void initAllocationTable(MS* secondary_memory, int allocation_table_buffer[]);
void initMetadata(MS* secondary_memory, Meta metabuffer[]);
/* -------------------------------------------------- */

// Global variables
int nbr_free_blocks = NUM_OF_BLOCKS;

// functions related to File System
/* -------------------------------------------------- */

void initFileSystem(MS* secondary_memory) {

     // initialize informations about the MS
     secondary_memory->inode.number_of_blocks = NUM_OF_BLOCKS;
     secondary_memory->inode.block_size = sizeof(Block);
     secondary_memory->inode.block_data_size = sizeof(Block) - sizeof(int);
     secondary_memory->inode.number_of_meta = 10;
     secondary_memory->inode.meta_per_block = secondary_memory->inode.block_data_size / sizeof(Meta);
     secondary_memory->inode.records_per_block = secondary_memory->inode.block_data_size / sizeof(Client);
     secondary_memory->inode.first_data_region_block = ceil((double)secondary_memory->inode.number_of_meta / secondary_memory->inode.meta_per_block);
     
     // create the file system
     secondary_memory->disk = fopen("database.txt", "w+");

     // initialize the allocation table
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     initAllocationTable(secondary_memory, allocation_table_buffer);
     
     // initialize metadata
     Meta metabuffer[secondary_memory->inode.meta_per_block];
     initMetadata(secondary_memory, metabuffer);

     // initialize blocs
     Block bloc;
     Block buffer;
     for (int i = 0; i < secondary_memory->inode.number_of_blocks; i++)
     {
          for (int j = 0; j < secondary_memory->inode.records_per_block; j++)
          {
               bloc.entries[j].ID = 0;
               strcpy(bloc.entries[j].name, ""); 
               bloc.entries[j].age = 0;
               bloc.entries[j].balance = 0;
          }
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

     printf("allocation table: \n");
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     readAllocationTable(secondary_memory, allocation_table_buffer);
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

void compactage(MS* secondary_memory) {

     // read the allocation table
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     readAllocationTable(secondary_memory, allocation_table_buffer);

     // read files metadata
     Meta metabuffer[secondary_memory->inode.meta_per_block];
     int nbr_block = ceil((double)secondary_memory->inode.number_of_meta / secondary_memory->inode.meta_per_block);

     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          fread(metabuffer, sizeof(Block), 1, secondary_memory->disk);
          int j = 0;
          while (j < secondary_memory->inode.meta_per_block && k < secondary_memory->inode.number_of_meta)
          {
               
               j++;
               k++;
          }
     }

     // Block buffer;
     // fseek(secondary_memory->disk, secondary_memory->inode.first_data_region_block * sizeof(Block), SEEK_SET);
     // int i = 0;
     // for (int j = 0; j < secondary_memory->inode.number_of_blocks; j++)
     // {
     //      if(allocation_table_buffer[j] != 0) {
     //           fseek(secondary_memory->disk, (secondary_memory->inode.first_data_region_block + j) * sizeof(Block), SEEK_SET);
     //           fread(&buffer, sizeof(Block), 1, secondary_memory->disk);

     //           fseek(secondary_memory->disk, (secondary_memory->inode.first_data_region_block + i) * sizeof(Block), SEEK_SET);
     //           fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);

     //           // update the allocation table
     //           allocation_table_buffer[i] = allocation_table_buffer[j];
     //           i++;
     //      }
     // }


     writeAllocationTable(secondary_memory, allocation_table_buffer);
     
}

void EmptyDisk(MS* secondary_memory) {
     // initialize the allocation table
     int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
     initAllocationTable(secondary_memory, allocation_table_buffer);
     
     // initialize metadata
     Meta metabuffer[secondary_memory->inode.meta_per_block];
     initMetadata(secondary_memory, metabuffer);
}
/* -------------------------------------------------- */

// functions related to allocation table
/* -------------------------------------------------- */
void initAllocationTable(MS* secondary_memory, int allocation_table_buffer[]) {
     for (int i = 0; i < secondary_memory->inode.number_of_blocks; i++)
     {
          allocation_table_buffer[i] = 0;
     }
     writeAllocationTable(secondary_memory, allocation_table_buffer);
}

void readAllocationTable(MS* secondary_memory, int allocation_table_buffer[]) {
     rewind(secondary_memory->disk);
     fread(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);
}

void writeAllocationTable(MS* secondary_memory, int allocation_table_buffer[]) {
     rewind(secondary_memory->disk);
     fwrite(allocation_table_buffer, sizeof(Block), 1, secondary_memory->disk);
}
/* -------------------------------------------------- */

// functions related to meta data
/* -------------------------------------------------- */
void initMetadata(MS* secondary_memory, Meta metabuffer[]) {
     fseek(secondary_memory->disk, sizeof(Block), SEEK_SET);
     Meta inode;
     int nbr_block = ceil((double)secondary_memory->inode.number_of_meta / secondary_memory->inode.meta_per_block);
     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          int j = 0;
          while (j < secondary_memory->inode.meta_per_block && k < secondary_memory->inode.number_of_meta)
          {
               strcpy(inode.name, "a");
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


     int start_block;
     if(inode->global_organisation_mode == 0) {
          start_block = findFreeAdjacentBlock(secondary_memory, inode->file_size_in_blocks);
          if(start_block != -1) {
               inode->start_block = start_block;
          } else {
               printf("There isn't enough space to store this file\n");
          }
     } else {
          start_block = findFreeChainedBlock(secondary_memory, inode->file_size_in_blocks);
          if(start_block != -1) {
               inode->start_block = start_block;
          } else {
               printf("There isn't enough space to store this file\n");
          }

     }
}


/* -------------------------------------------------- */

// functions related to data files
/* -------------------------------------------------- */
Client generateClient(int id) {
     Client client;

     client.ID = id;
     snprintf(client.name, sizeof(client.name), "Client %d", rand() % 100);
     client.age = 20 + rand() % (40 + 1); // Generate a random number between 20 and 60
     client.balance = (rand() / (float)RAND_MAX) * (4000) + 1000; // Generate a random floating-point number between 1000 and 5000
     client.balance = roundf(client.balance * 100) / 100;
     return client;
}

void fillFileData(MS* secondary_memory, Meta inode) {
     // variables
     Block bloc;
     Block buffer;

     // move the file cursor to the start block of the file
     fseek(secondary_memory->disk, (inode.start_block + secondary_memory->inode.first_data_region_block) * sizeof(Block), SEEK_SET);
     int k = 0;
     int next_block = inode.start_block;

     if(inode.global_organisation_mode == 0) {
          // contiguous mode
          for (int i = 0; i < inode.file_size_in_blocks; i++)
          {
               int j = 0;
               while (j < secondary_memory->inode.records_per_block && k < inode.file_size_in_records)
               {
                    bloc.entries[j] = generateClient(k);
                    j++;
                    k++;
               }
               bloc.next = (i == inode.file_size_in_blocks - 1) ? -1 : next_block + 1;
               buffer = bloc;
               fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);
               next_block++;
          }
          
     } else {
          // chained mode
          for (int i = 0; i < inode.file_size_in_blocks; i++)
          {
               int j = 0;
               while (j < secondary_memory->inode.records_per_block && k < inode.file_size_in_records)
               {
                    bloc.entries[j] = generateClient(k);
                    j++;
                    k++;
               }
               int new_block = findFreeChainedBlock(secondary_memory, inode.file_size_in_blocks);

               bloc.next = (i == inode.file_size_in_blocks - 1) ? -1 : new_block;
               buffer = bloc;
               fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);
               if (bloc.next != -1) {
                fseek(secondary_memory->disk, 
                      (bloc.next + secondary_memory->inode.first_data_region_block) * sizeof(Block), SEEK_SET);
               }
          }
          
     }

     // fseek(secondary_memory->disk, (next_block + secondary_memory->inode.first_data_region_block) * sizeof(Block), SEEK_SET);
     // int j = 0;
     // while (j < secondary_memory->inode.records_per_block && k < inode.file_size_in_records)
     // {
     //      buffer.entries[j] = generateClient(k);
     //      j++;
     //      k++;
     // }
     // buffer.next = -1;
     // fwrite(&buffer, sizeof(Block), 1, secondary_memory->disk);
}

int findFreeAdjacentBlock(MS* secondary_memory, int file_size_in_blocks) {
     if(file_size_in_blocks > nbr_free_blocks) {
          return -1;
     } else {
          rewind(secondary_memory->disk);
          int remaining_blocks_needed;
          int exist = 0;

          int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
          readAllocationTable(secondary_memory, allocation_table_buffer);

          int i = 0;
          while (i < secondary_memory->inode.number_of_blocks && exist == 0)
          {
               if(allocation_table_buffer[i] == 0) {
                    int start = i;
                    remaining_blocks_needed = file_size_in_blocks;
                    while(allocation_table_buffer[i] == 0 && remaining_blocks_needed > 0) {
                         i++;
                         remaining_blocks_needed--;
                    }

                    if(remaining_blocks_needed == 0) {
                         exist = 1;
                         nbr_free_blocks -= file_size_in_blocks;
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

int findFreeChainedBlock(MS* secondary_memory, int file_size_in_block) {
     if(file_size_in_block > nbr_free_blocks) {
          return -1;
     } else {
          int allocation_table_buffer[secondary_memory->inode.number_of_blocks];
          readAllocationTable(secondary_memory, allocation_table_buffer);
          int i = 0;
          while (allocation_table_buffer[i] != 0 && i < secondary_memory->inode.number_of_blocks)
          {
               i++;
          }

          return i;
     }
}

void displayFileData(MS* secondary_memory, Meta inode) {
     Block buffer;
    int next_block = inode.start_block;
    int k = 0;  // Record counter

    printf("Filename: %s\n", inode.name);

    for (int i = 0; i < inode.file_size_in_blocks; i++) {
        // Move the file cursor to the current block
        if (next_block < 0) {
            fprintf(stderr, "Error: Invalid block index encountered.\n");
            return;
        }
        fseek(secondary_memory->disk, 
              (secondary_memory->inode.first_data_region_block + next_block) * sizeof(Block), SEEK_SET);

        // Read the block
        if (fread(&buffer, sizeof(Block), 1, secondary_memory->disk) != 1) {
            fprintf(stderr, "Error: Failed to read block %d.\n", next_block);
            return;
        }

        // Display entries in the block
        int j = 0;  // Entry index within the block
        while (j < secondary_memory->inode.records_per_block && k < inode.file_size_in_records) {
            printf("id: %d, name: %s, age: %d, balance: %.2f\n", 
                   buffer.entries[j].ID, 
                   buffer.entries[j].name, 
                   buffer.entries[j].age, 
                   buffer.entries[j].balance);
            j++;
            k++;
        }

        // Move to the next block
        next_block = buffer.next;
        if (next_block == -1) {
            // Reached the end of the chain
            break;
        }
    }

    if (k < inode.file_size_in_records) {
        fprintf(stderr, "Warning: Expected %d records but displayed only %d.\n", 
                inode.file_size_in_records, k);
    }
}

void createFile(MS* secondary_memory) {
     Meta inode;

     fillMetadata(secondary_memory, &inode);
     displayMetadata(&inode);
     fillFileData(secondary_memory, inode);
     displayFileData(secondary_memory, inode);
}
/* -------------------------------------------------- */


int main() {
     srand(time(NULL));
     MS secondary_memory;

     initFileSystem(&secondary_memory); // init system
     printFileSystem(&secondary_memory); // print the file system
     createFile(&secondary_memory);
     // Meta inode;
     // fillFileData(&secondary_memory, inode);
     printf("working");
     return 0;
}