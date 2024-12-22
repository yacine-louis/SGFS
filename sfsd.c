#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_OF_BLOCKS 64
#define FB 8
#define NUM_OF_META 10

typedef struct Client Client;
typedef struct Block Block;
typedef struct Meta Meta;
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
     char name[12];
     int start_block; // First block of the file
     int file_size_in_blocks; 
     int file_size_in_records;
     int global_organisation_mode; // 0 for contiguous, 1 for chained, -1 when initialize
     int internal_organisation_mode; // 0 for records not sorted, 1 for records sorted, -1 when initialize
};

struct Position {
     int block_index;
     int index;
};

// global variables
const int number_of_blocks = NUM_OF_BLOCKS;
const int block_size = sizeof(Block);
const int block_data_size = sizeof(Block) - sizeof(int);
const int number_of_meta = NUM_OF_META;
const int meta_per_block = (sizeof(Block) - sizeof(int)) / sizeof(Meta);
const int records_per_block = (sizeof(Block) - sizeof(int)) / sizeof(Client);
const int first_data_region_block = ceil((double)NUM_OF_META / ((sizeof(Block) - sizeof(int)) / sizeof(Meta))) + 1;
int nbr_free_blocks = NUM_OF_BLOCKS;
int nbr_free_metas = NUM_OF_META;

// functions
void initFileSystem(FILE* MS);
void printFileSystem(FILE* MS);
void readAllocationTable(FILE* MS, int allocation_table_buffer[], int size);
void writeAllocationTable(FILE* MS, int allocation_table_buffer[], int size);
Client generateClient(int id);
void fillFileData(FILE* MS, Meta inode);
int findFreeAdjacentBlock(FILE* MS, int file_size_in_blocks);
void fillMetadata(FILE* MS, Meta *inode);
void displayMetadata(Meta inode);
void createFile(FILE* MS);
void compactage(FILE* MS);
Position searchMetadata(FILE* MS, char name[]);
void EmptyDisk(FILE* MS);
void initAllocationTable(FILE* MS, int allocation_table_buffer[], int size);
void initMetadata(FILE* MS, Meta metabuffer[], int size);
void printBlocks(FILE* MS);
void initBlock(Block* bloc);
void printAllocationTable(FILE* MS, int allocation_table_buffer[], int size);

void initFileSystem(FILE* MS) {

     // initialize the allocation table
     int allocation_table_buffer[number_of_blocks];
     initAllocationTable(MS, allocation_table_buffer, number_of_blocks);

     // initialize the metadata
     Meta metabuffer[meta_per_block];
     initMetadata(MS, metabuffer, meta_per_block);

     // initialize blocs
     Block bloc;
     Block buffer;
     for (int i = 0; i < number_of_blocks; i++)
     {
          initBlock(&bloc);
          buffer = bloc;
          fwrite(&buffer, sizeof(Block), 1, MS);
     }
}

void printFileSystem(FILE* MS) {
     
     // printf("MS informations: \n");
     // printf("number of blocks: %d \n", number_of_blocks);
     // printf("block size: %d \n", block_size);
     // printf("block data size: %d \n", block_data_size);
     // printf("number of metadata: %d \n", number_of_meta);

     printf("allocation table: \n");
     int allocation_table_buffer[number_of_blocks];
     readAllocationTable(MS, allocation_table_buffer, number_of_blocks);
     for (int i = 0; i < number_of_blocks; i++)
     {
          printf("%d ", allocation_table_buffer[i]);
     }
     printf("\n");
     
     printf("meta data: \n");
     Meta metabuffer[meta_per_block];
     int nbr_block = ceil((double)number_of_meta / meta_per_block);
     
     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          fseek(MS, (i+1)*sizeof(Block), SEEK_SET);
          fread(metabuffer, sizeof(metabuffer), 1, MS);
          int j = 0;
          while (j < meta_per_block && k < number_of_meta)
          {
               printf("file name: %s, startBlock: %d, FileSizeInBlocks: %d, fileSizeInRecords: %d, globalOrganisationMode: %d, internalOrganisationMode: %d", metabuffer[j].name, metabuffer[j].start_block, metabuffer[j].file_size_in_blocks, metabuffer[j].file_size_in_records, metabuffer[j].global_organisation_mode, metabuffer[j].internal_organisation_mode);
               printf("\n");
               j++;
               k++;
          }
     }
}

void initBlock(Block* bloc) {
     for (int i = 0; i < records_per_block; i++)
     {
          bloc->entries[i].ID = 0;
          strcpy(bloc->entries[i].name, ""); 
          bloc->entries[i].age = 0;
          bloc->entries[i].balance = 0;
     }
     bloc->next = -1;
}

void compactage(FILE* MS) {

     // read the allocation table
     int allocation_table_buffer[number_of_blocks];
     readAllocationTable(MS, allocation_table_buffer, number_of_blocks);

     // read files metadata
     Meta metabuffer[meta_per_block];
     int nbr_block = ceil((double)number_of_meta / meta_per_block);

     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          fread(metabuffer, sizeof(Block), 1, MS);
          int j = 0;
          while (j < meta_per_block && k < number_of_meta)
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


     writeAllocationTable(MS, allocation_table_buffer, number_of_blocks);
     
}

void EmptyDisk(FILE* MS) {
     // initialize the allocation table
     int allocation_table_buffer[number_of_blocks];
     initAllocationTable(MS, allocation_table_buffer, number_of_blocks);
     
     // initialize metadata
     Meta metabuffer[meta_per_block];
     initMetadata(MS, metabuffer, meta_per_block);
}


// Allocation table functions
void readAllocationTable(FILE* MS, int allocation_table_buffer[], int size) {
     rewind(MS);
     fread(allocation_table_buffer, size * sizeof(int), 1, MS);
     fseek(MS, sizeof(Block), SEEK_SET);
}

void writeAllocationTable(FILE* MS, int allocation_table_buffer[], int size) {
     rewind(MS);
     fwrite(allocation_table_buffer, size * sizeof(int), 1, MS);
     fseek(MS, sizeof(Block), SEEK_SET);
}

void initAllocationTable(FILE* MS, int allocation_table_buffer[], int size) {
     for (int i = 0; i < size; i++)
     {
          allocation_table_buffer[i] = 0;
     }
     writeAllocationTable(MS, allocation_table_buffer, size);
}

int findFreeAdjacentBlock(FILE* MS, int file_size_in_blocks) {
     if(file_size_in_blocks > nbr_free_blocks) {
          printf("error: File is too big\n");
          return -1;
     } else {
          // read the allocation table
          int allocation_table_buffer[number_of_blocks];
          readAllocationTable(MS, allocation_table_buffer, number_of_blocks);
          int blocks_needed;
          for (int i = 0; i < number_of_blocks; i++)
          {
               if(allocation_table_buffer[i] == 0) {
                    int start = i;
                    blocks_needed = file_size_in_blocks;
                    while (allocation_table_buffer[i] == 0 && blocks_needed > 0)
                    {
                         i++;
                         blocks_needed--;
                    }
                    
                    if(blocks_needed == 0) {
                         // update the allocation table and the number of free blocks
                         for (int j = start; j < i; j++)
                         {
                              allocation_table_buffer[j] = 1;
                              nbr_free_blocks--;
                         }
                         
                         writeAllocationTable(MS, allocation_table_buffer, number_of_blocks);
                         return start;
                    }

               }
          }
          
          if(blocks_needed != 0) {
               // propose compactage
          }
     }
}

void printAllocationTable(FILE* MS, int allocation_table_buffer[], int size) {
     for (int i = 0; i < size; i++)
     {
          printf("%d ", allocation_table_buffer[i]);
     }
}

// Meta data functions
void initMetadata(FILE* MS, Meta metabuffer[], int size) {
     Meta inode;
     int nbr_block = ceil((double)number_of_meta / meta_per_block);
     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          int j = 0;
          while (j < meta_per_block && k < number_of_meta)
          {
               strcpy(inode.name, "");
               inode.start_block = -1;
               inode.file_size_in_blocks = 0;
               inode.file_size_in_records = 0;
               inode.global_organisation_mode = -1;
               inode.internal_organisation_mode = -1;
               metabuffer[j] = inode;
               j++;
               k++;
          }
          fseek(MS, (i+1)*sizeof(Block), SEEK_SET);
          fwrite(metabuffer, size * sizeof(Meta), 1, MS);
     }
}

void displayMetadata(Meta inode) {
     // Display the file metadata
     printf("\nFile Metadata:\n");
     printf("Name: %s\n", inode.name);
     printf("Number of Records: %d\n", inode.file_size_in_records);
     printf("Number of Blocks: %d\n", inode.file_size_in_blocks);
     printf("Global Organization Mode: %s\n", 
          (inode.global_organisation_mode == 0) ? "Contiguous" : "Chained");
     printf("Internal Organization Mode: %s\n", 
          (inode.internal_organisation_mode == 0) ? "Sorted" : "Not Sorted");
     printf("First Block: %d\n", inode.start_block);
}

void fillMetadata(FILE* MS, Meta *inode) {
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
     inode->file_size_in_blocks = (int)ceil((double)inode->file_size_in_records / records_per_block);

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
          start_block = findFreeAdjacentBlock(MS, inode->file_size_in_blocks);
          if(start_block != -1) {
               inode->start_block = start_block;
          } else {
               printf("There isn't enough space to store this file\n");
          }
     } else {
          int allocation_table_buffer[number_of_blocks];
          readAllocationTable(MS, allocation_table_buffer, number_of_blocks);

          // go through the allocation table and find empty slot
          for (int m = 0; m < number_of_blocks; m++)
          {
               if(allocation_table_buffer[m] == 0) {
                    // update the allocation table
                    allocation_table_buffer[m] = 1;
                    nbr_free_blocks--;
                    start_block = m;
                    break;
               }
          }
          writeAllocationTable(MS, allocation_table_buffer, number_of_blocks);
          if(start_block != -1) {
               inode->start_block = start_block;
          } else {
               printf("There isn't enough space to store this file\n");
          }

     }


}

Position searchMetadata(FILE* MS, char name[]) {
     fseek(MS, sizeof(Block), SEEK_SET);
     Meta metabuffer[meta_per_block];
     int nbr_block = ceil((double)number_of_meta / meta_per_block);
     Position p;
     int k = 0;
     for (int i = 0; i < nbr_block; i++)
     {
          fread(metabuffer, sizeof(Block), 1, MS);
          int j = 0;
          while (j < meta_per_block && k < number_of_meta)
          {
               if(strcmp(metabuffer[j].name, name) == 0) {
                    p.block_index = i + 1; // +1 because the allocation table is in the first block
                    p.index = j;
                    return p;
               }
               j++;
               k++;
          }
     }

     p.block_index = -1;
     p.index = -1;
     return p;
}



// data files functions
Client generateClient(int id) {
     Client client;

     client.ID = id;
     snprintf(client.name, sizeof(client.name), "Client %d", rand() % 100);
     client.age = 20 + rand() % (40 + 1); // Generate a random number between 20 and 60
     client.balance = (rand() / (float)RAND_MAX) * (4000) + 1000; // Generate a random floating-point number between 1000 and 5000
     client.balance = roundf(client.balance * 100) / 100;
     return client;
}

void fillFileData(FILE* MS, Meta inode) {
     if(inode.file_size_in_blocks > nbr_free_blocks) {
          printf("error: File is too big\n");
          return;
     } else {

          // Fill metadata in MS
          Position meta_place = searchMetadata(MS, "");

          // Move the file cursor to the block where the metadata is located
          fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);

          // Read the metadata block into metabuffer
          Meta* metabuffer = malloc(meta_per_block * sizeof(Meta));
          fread(metabuffer, meta_per_block * sizeof(Meta), 1, MS);
          

          // Update only the specific metadata entry at meta_place.index
          strcpy(metabuffer[meta_place.index].name, inode.name);
          metabuffer[meta_place.index].file_size_in_blocks = inode.file_size_in_blocks;
          metabuffer[meta_place.index].file_size_in_records = inode.file_size_in_records;
          metabuffer[meta_place.index].global_organisation_mode = inode.global_organisation_mode;
          metabuffer[meta_place.index].internal_organisation_mode = inode.internal_organisation_mode;
          metabuffer[meta_place.index].start_block = inode.start_block;

          // Write back the modified block containing the metadata
          fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
          fwrite(metabuffer, meta_per_block * sizeof(Meta), 1, MS);
          free(metabuffer);
          
          Block bloc;
          Block buffer;

          int k = 0;

          if(inode.global_organisation_mode == 0) {
               // contiguous mode
               // move the file cursor to the first block
               fseek(MS, (first_data_region_block + inode.start_block) * sizeof(Block), SEEK_SET);

               int next_block = inode.start_block;
               initBlock(&bloc);
               for (int i = 0; i < inode.file_size_in_blocks; i++)
               {
                    int j = 0;
                    while (j < records_per_block && k < inode.file_size_in_records)
                    {
                         bloc.entries[j] = generateClient(k);
                         j++;
                         k++;
                    }
                    
                    if(i == inode.file_size_in_blocks - 1) {
                         next_block = -1;
                    } else {
                         next_block = next_block + 1;
                    }
                    bloc.next = next_block;
                    buffer = bloc;
                    fwrite(&buffer, sizeof(Block), 1, MS);
               }
          } else {
               // chainned mode

               int next_block;
               // read the allocation table 
               int allocation_table_buffer[number_of_blocks];
               readAllocationTable(MS, allocation_table_buffer, number_of_blocks);

               // move the file cursor to the first block
               fseek(MS, (first_data_region_block + inode.start_block) * sizeof(Block), SEEK_SET);
               
               // fill the blocks
               for (int i = 0; i < inode.file_size_in_blocks; i++)
               {
                    initBlock(&bloc);
                    int j = 0;
                    while (j < records_per_block && k < inode.file_size_in_records)
                    {
                         bloc.entries[j] = generateClient(k);
                         j++;
                         k++;
                    }

                    // check if we are on the the last block of the file
                    if(i == inode.file_size_in_blocks - 1) {
                         bloc.next = -1;
                    } else {

                         // go through the allocation table and find empty slot
                         for (int m = 0; m < number_of_blocks; m++)
                         {
                              if(allocation_table_buffer[m] == 0) {
                                   // update the allocation table
                                   allocation_table_buffer[m] = 1;
                                   nbr_free_blocks--;
                                   next_block = m;
                                   break;
                              }
                         }

                         bloc.next = next_block;
                    }

                    buffer = bloc;
                    fwrite(&buffer, sizeof(Block), 1, MS);
                    fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);
               }
               // write the allocation table
               writeAllocationTable(MS, allocation_table_buffer, number_of_blocks);
          }
     }
}

void displayFileData(FILE* MS, Meta inode) {
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
          fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);

          // Read the block
          fread(&buffer, sizeof(Block), 1, MS);

          // Display entries in the block
          int j = 0;  // Entry index within the block
          while (j < records_per_block && k < inode.file_size_in_records) {
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
}

void createFile(FILE* MS) {
     Meta inode;
     // strcpy(inode.name, "test1");
     // inode.file_size_in_blocks = 3;
     // inode.file_size_in_records = 17;
     // inode.global_organisation_mode = 0;
     // inode.internal_organisation_mode = 0;
     // inode.start_block = 0;
     fillMetadata(MS, &inode);
     displayMetadata(inode);
     fillFileData(MS, inode);
     displayFileData(MS, inode);

     // strcpy(inode.name, "test2");
     // inode.file_size_in_blocks = 4;
     // inode.file_size_in_records = 27;
     // inode.global_organisation_mode = 0;
     // inode.internal_organisation_mode = 0;
     // inode.start_block = 3;
     // // fillMetadata(MS, &inode);
     // displayMetadata(inode);
     // fillFileData(MS, inode);
     // displayFileData(MS, inode);


}

void printBlocks(FILE* MS) {
     fseek(MS, first_data_region_block * sizeof(Block), SEEK_SET);
     Block buffer;
     
     for (int i = 0; i < NUM_OF_BLOCKS; i++)
     {
          fread(&buffer, sizeof(Block), 1, MS);
          printf("block: %d, next: %d\n", i, buffer.next);
     }
     
}

int main() {
     srand(time(NULL));
     FILE* MS = fopen("MS.bin", "rb+");


     initFileSystem(MS);
     createFile(MS);
     createFile(MS);
     printFileSystem(MS);
     printBlocks(MS);
     printf("working\n");
     return 0;
}
