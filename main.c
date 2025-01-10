#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// definitions
/*---------------------------------------------------------------------------------*/
#define NUM_OF_BLOCKS 8            
#define FB 8
#define NUM_OF_META 10
#define MAX_NAME_LENGTH 16
/*---------------------------------------------------------------------------------*/


// Structures
/*---------------------------------------------------------------------------------*/
typedef struct {
    int ID;
    char name[MAX_NAME_LENGTH];
    int age;
    float balance;
    int LD; // used for logical deletion, 1 for logical delete and 0 for not 
} Client;

typedef struct {
    Client entries[FB];
    int next;
} Block;

typedef struct {
    char name[MAX_NAME_LENGTH];
    int start_block; // First block of the file
    int file_size_in_blocks; 
    int file_size_in_records;
    int global_organisation_mode; // 0 for contiguous, 1 for chained
    int internal_organisation_mode; // 0 for records not sorted, 1 for records sorted
} Meta;

typedef struct {
    int block_index;
    int index;
} Position;
/*---------------------------------------------------------------------------------*/


// global variables
/*---------------------------------------------------------------------------------*/
const int block_data_size = sizeof(Block) - sizeof(int);
const int records_per_block = (sizeof(Block) - sizeof(int)) / sizeof(Client);

const int meta_per_block = (sizeof(Block) - sizeof(int)) / sizeof(Meta);
const int nbr_meta_blocks = ceil((double)NUM_OF_META / ((sizeof(Block) - sizeof(int)) / sizeof(Meta)));

const int first_data_region_block = ceil((double)NUM_OF_META / ((sizeof(Block) - sizeof(int)) / sizeof(Meta))) + 1;

int nbr_free_blocks = NUM_OF_BLOCKS;
int nbr_free_metas = NUM_OF_META;
/*---------------------------------------------------------------------------------*/


// error types
/*---------------------------------------------------------------------------------*/
char NO_FREE_BLOCKS_ERROR_MSG[] = "Error: No free blocks available to store this file";
bool is_storage_full = false;

char FILE_TOO_BIG_ERROR_MSG[] = "Error: File is too big";
bool is_file_too_big = false;

char CONTIGUOUS_SPACE_ERROR_MSG[] = "Error: Not enough contiguous space to store this file";
bool contiguous_space_issue = false;

char NO_FREE_META_ERROR_MSG[] = "Error: No free metadata blocks available to allocate for this file.";
bool is_metadata_full = false;

char FILE_NOT_FOUND_ERROR_MSG[] = "Error: File does not exist";
bool file_not_found_flag = false;
/*---------------------------------------------------------------------------------*/


// functions declarations
/*---------------------------------------------------------------------------------*/
void initFileSystem(FILE* MS);
void printFileSystem(FILE* MS);
void initBlock(Block* bloc);
void printBlocks(FILE* MS);
void compactage(FILE* MS);
void EmptyDisk(FILE* MS);
void readAllocationTable(FILE* MS, int allocation_table_buffer[]);
void writeAllocationTable(FILE* MS, int allocation_table_buffer[]);
void initAllocationTable(FILE* MS, int allocation_table_buffer[]);
int findFreeAdjacentBlock(FILE* MS, int file_size_in_blocks);
void printAllocationTable(FILE* MS, int allocation_table_buffer[]);
void initMetadata(FILE* MS, Meta metabuffer[]);
void displayInode(Meta inode);
void promptInode(FILE* MS, Meta *inode);
void fillStartBlock(FILE* MS, Meta* inode);
Position searchMetadata(FILE* MS, char name[]);
Client generateClient(int id);
void fillFileDataAndMeta(FILE* MS, Meta inode);
void displayFileData(FILE* MS, Meta inode);
void createFile(FILE* MS);
void deleteFile(FILE* MS, char* filename);
void initInode(Meta* inode);
void insertRecord(FILE* MS, char* filename);
void renameFile(FILE* MS, char* old_filename, char* new_filename);
Position searchRecord(FILE* MS, char* filename, int id);
int binarySearch(Client clients[], int left, int right, int val);
void validateConstraints(int file_size_in_blocks);
void displayErrors();
void resetFlags();
void displayMenu();

/*---------------------------------------------------------------------------------*/



// functions implementations
/*---------------------------------------------------------------------------------*/
void initFileSystem(FILE* MS) {
    // initialize the allocation table
    int allocation_table_buffer[NUM_OF_BLOCKS];
    initAllocationTable(MS, allocation_table_buffer);

    // initialize the metadata
    Meta metabuffer[meta_per_block];
    initMetadata(MS, metabuffer);

    // initialize blocs
    Block bloc;
    Block buffer;

    initBlock(&bloc);
    buffer = bloc;
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        fwrite(&buffer, sizeof(Block), 1, MS);
    }
}

void printFileSystem(FILE* MS) {
    printf("Allocation Table: \n");
    int allocation_table_buffer[NUM_OF_BLOCKS];
    readAllocationTable(MS, allocation_table_buffer);
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        printf("%d ", allocation_table_buffer[i]);
    }
    printf("\n");
    
    printf("Meta Data: \n");
    Meta metabuffer[meta_per_block];
    int nbr_block = ceil((double)NUM_OF_META / meta_per_block);
    
    int k = 0;
    for (int i = 0; i < nbr_block; i++)
    {
        fseek(MS, (i+1)*sizeof(Block), SEEK_SET);
        fread(metabuffer, sizeof(Meta), meta_per_block, MS);
        int j = 0;
        while (j < meta_per_block && k < NUM_OF_META)
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
        bloc->entries[i].LD = 0;
    }
    bloc->next = -1;
}

void initInode(Meta* inode) {
    strcpy(inode->name, "");
    inode->start_block = -1;
    inode->file_size_in_blocks = 0;
    inode->file_size_in_records = 0;
    inode->global_organisation_mode = -1;
    inode->internal_organisation_mode = -1;
}

void printBlocks(FILE* MS) {
    printf("File System Blocks: \n");
    fseek(MS, first_data_region_block * sizeof(Block), SEEK_SET);
    Block buffer;
    
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        fread(&buffer, sizeof(Block), 1, MS);
        printf("block: %d, next: %d\n", i, buffer.next);
    }
}

void EmptyDisk(FILE* MS) {

    MS = fopen("MS.bin", "wb+");

    // initialize the allocation table
    int allocation_table_buffer[NUM_OF_BLOCKS];
    initAllocationTable(MS, allocation_table_buffer);
    
    // initialize metadata
    Meta metabuffer[meta_per_block];
    initMetadata(MS, metabuffer);
}

void compact_disk(FILE* MS) {
        
    // // read the allocation table
    // int allocation_table_buffer[NUM_OF_BLOCKS];
    // readAllocationTable(MS, allocation_table_buffer);

    // // read files metadata
    // Meta metabuffer[meta_per_block];
    // Meta inode;
    // for (int i = 0; i < nbr_meta_blocks; i++)
    // {
    //     fread(metabuffer, sizeof(Meta), meta_per_block, MS);
    //     for (int j = 0; j < meta_per_block; j++)
    //     {
            
    //     }
        


    // }
    
    
    
    // for (int i = 0; i < meta_per_block; i++)
    // {
    //     /* code */
    // }
    
    // int k = 0;

    // for (int i = 0; i < nbr_meta_blocks; i++)
    // {
    //     fread(metabuffer, sizeof(Block), 1, MS);
    //     int j = 0;
    //     while (j < meta_per_block && k < NUM_OF_META)
    //     {
            
    //         j++;
    //         k++;
    //     }
    // }

    // int write_index = 0; // Pointer to the next free block in the compacted disk

    // for (int i = 0; i < file_count; i++) {
    //     if (files[i].is_chained) {
    //         // Compact chained file
    //         int current = files[i].start;
    //         int previous = -1;
    //         while (current != -1) {
    //             if (current != write_index) {
    //                 allocation_table[write_index] = 1;
    //                 allocation_table[current] = 0;
    //                 blocks[write_index] = blocks[current];

    //                 if (previous == -1) {
    //                     files[i].start = write_index;
    //                 } else {
    //                     blocks[previous].next = write_index;
    //                 }

    //                 previous = write_index;
    //                 write_index++;
    //             } else {
    //                 previous = current;
    //                 write_index++;
    //             }

    //             current = blocks[previous].next;
    //         }
    //         blocks[previous].next = -1;
    //     } else {
    //         // Compact contiguous file
    //         if (files[i].start != write_index) {
    //             for (int j = 0; j < files[i].size; j++) {
    //                 allocation_table[write_index + j] = 1;
    //                 allocation_table[files[i].start + j] = 0;
    //             }
    //             files[i].start = write_index;
    //         }
    //         write_index += files[i].size;
    //     }
    // }

    // // Mark remaining blocks as free
    // for (int i = write_index; i < num_blocks; i++) {
    //     allocation_table[i] = 0;
    // }
}


void validateConstraints(int file_size_in_blocks) {
    if(nbr_free_blocks == 0) {
        is_storage_full = true;
    } else if(file_size_in_blocks > nbr_free_blocks){
        is_file_too_big = true;
    }
    if(nbr_free_metas == 0) {
        is_metadata_full = true;
    }
    
}

void displayErrors() {
    char buffer[256];
    strcpy(buffer, "");

    if(is_storage_full) {
        strcat(buffer, NO_FREE_BLOCKS_ERROR_MSG);
        strcat(buffer, "\n");
    } else if(is_file_too_big) {
        strcat(buffer, FILE_TOO_BIG_ERROR_MSG);
        strcat(buffer, "\n");
    }

    if(is_metadata_full) {
        strcat(buffer, NO_FREE_META_ERROR_MSG);
        strcat(buffer, "\n");
    }

    if(contiguous_space_issue) {
        strcat(buffer, CONTIGUOUS_SPACE_ERROR_MSG);
        strcat(buffer, "\n");
    }

    if(file_not_found_flag) {
        strcat(buffer, FILE_NOT_FOUND_ERROR_MSG);
        strcat(buffer, "\n");
    }

    printf("%s", buffer);
}

void resetFlags() {
    if(nbr_free_blocks != 0) {
        is_storage_full = false;
    }
    if(nbr_free_metas != 0) {
        bool is_metadata_full = false;
    }
    bool is_file_too_big = false;
    bool contiguous_space_issue = false;
    bool file_not_found_flag = false;
}


// allocation table functions 
void readAllocationTable(FILE* MS, int allocation_table_buffer[]) {
    rewind(MS);
    fread(allocation_table_buffer, sizeof(int), NUM_OF_BLOCKS, MS);
    fseek(MS, sizeof(Block), SEEK_SET);
}

void writeAllocationTable(FILE* MS, int allocation_table_buffer[]) {
    rewind(MS);
    fwrite(allocation_table_buffer, sizeof(int), NUM_OF_BLOCKS, MS);
    fseek(MS, sizeof(Block), SEEK_SET);
}

void initAllocationTable(FILE* MS, int allocation_table_buffer[]) {
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        allocation_table_buffer[i] = 0;
    }
    writeAllocationTable(MS, allocation_table_buffer);
}

int findFreeAdjacentBlock(FILE* MS, int file_size_in_blocks) {
    // Read the allocation table
    int allocation_table_buffer[NUM_OF_BLOCKS];
    readAllocationTable(MS, allocation_table_buffer);

    // Search for contiguous free blocks
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        if (allocation_table_buffer[i] == 0) {
            int start = i;
            int blocks_needed = file_size_in_blocks;

            // Check for contiguous free blocks
            while (allocation_table_buffer[i] == 0 && blocks_needed > 0) {
                i++;
                blocks_needed--;
            }

            // If enough contiguous blocks are found
            if (blocks_needed == 0) {
                // Mark the blocks as allocated
                for (int j = start; j < start + file_size_in_blocks; j++) {
                    allocation_table_buffer[j] = 1;
                    nbr_free_blocks--;
                }

                // Write the updated allocation table back to the file
                writeAllocationTable(MS, allocation_table_buffer);
                return start; // Return the starting index of the allocated blocks
            }
        }
    }

    // If no contiguous blocks are found
    contiguous_space_issue = true;
    return -1;
}

void printAllocationTable(FILE* MS, int allocation_table_buffer[]) {
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        printf("%d ", allocation_table_buffer[i]);
    }
}





// meta data functions
void initMetadata(FILE* MS, Meta metabuffer[]) {
    Meta inode;
    int k = 0;
    for (int i = 0; i < nbr_meta_blocks; i++)
    {
        int j = 0;
        while (j < meta_per_block && k < NUM_OF_META)
        {
            initInode(&inode);
            metabuffer[j] = inode;
            j++;
            k++;
        }
        fseek(MS, (i+1)*sizeof(Block), SEEK_SET);
        fwrite(metabuffer, meta_per_block * sizeof(Meta), 1, MS);
    }
}

void promptInode(FILE* MS, Meta *inode) {
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
        printf("Choose one of the following internal organization modes:\n[0] -> Not Sorted\n[1] -> Sorted\nAnswer: ");
        if (scanf("%d", &inode->internal_organisation_mode) != 1 || 
            (inode->internal_organisation_mode != 0 && inode->internal_organisation_mode != 1)) {
            printf("Invalid input. Please enter 0 or 1.\n");
        } else {
            break;
        }
    } while (1);


}

void fillStartBlock(FILE* MS, Meta* inode) {
    int start_block;

    validateConstraints(inode->file_size_in_blocks);
    if(is_storage_full || is_file_too_big || is_metadata_full) {
        inode->start_block = -1;
        return;
    }

    if(inode->global_organisation_mode == 0) {
        // mode contigue
        start_block = findFreeAdjacentBlock(MS, inode->file_size_in_blocks);

        // the case of there is contingues space issue
        if(start_block < 0) {
            inode->start_block = -1;
            return;
        }

        inode->start_block = start_block;

    } else {
        // chained mode
        int allocation_table_buffer[NUM_OF_BLOCKS];
        readAllocationTable(MS, allocation_table_buffer);
        for (int m = 0; m < NUM_OF_BLOCKS; m++) {
            if (allocation_table_buffer[m] == 0) {
                allocation_table_buffer[m] = 1;
                nbr_free_blocks--;
                start_block = m;
                break;
            }
        }
        
        inode->start_block = start_block;
        writeAllocationTable(MS, allocation_table_buffer);
        return;
    }
}

void displayInode(Meta inode) {
    // Display the file metadata
    printf("\nFile Metadata:\n");
    printf("Name: %s\n", inode.name);
    printf("Number of Records: %d\n", inode.file_size_in_records);
    printf("Number of Blocks: %d\n", inode.file_size_in_blocks);
    printf("Global Organization Mode: %s\n", 
        (inode.global_organisation_mode == 0) ? "Contiguous" : "Chained");
    printf("Internal Organization Mode: %s\n", 
        (inode.internal_organisation_mode == 0) ? "Not Sorted" : "Sorted");
    printf("First Block: %d\n", inode.start_block);
}

Position searchMetadata(FILE* MS, char name[]) {

    fseek(MS, sizeof(Block), SEEK_SET);

    Meta metabuffer[meta_per_block];
    Position p;
    int k = 0;
    for (int i = 0; i < nbr_meta_blocks; i++)
    {
        fread(metabuffer, sizeof(metabuffer), 1, MS);
        int j = 0;
        while (j < meta_per_block && k < NUM_OF_META)
        {
            if(strcmp(metabuffer[j].name, name) == 0) {
                p.block_index = i + 1; // +1 because the allocation table is in the first block
                p.index = j;
                return p;
            }
            j++;
            k++;
        }
        fseek(MS, (i+1) * sizeof(Block), SEEK_SET);
    }

    p.block_index = -1;
    p.index = -1;
    file_not_found_flag = true;
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
    client.LD = 0;
    return client;
}

void fillFileDataAndMeta(FILE* MS, Meta inode) {
    int start_block;

    // Fill metadata in MS
    Position meta_place = searchMetadata(MS, "");


    // Move the file cursor to the block where the metadata is located
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);

    // Read the metadata block into metabuffer
    Meta metabuffer[meta_per_block];
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);
    
    // Update only the specific metadata entry at meta_place.index
    strcpy(metabuffer[meta_place.index].name, inode.name);
    metabuffer[meta_place.index].file_size_in_blocks = inode.file_size_in_blocks;
    metabuffer[meta_place.index].file_size_in_records = inode.file_size_in_records;
    metabuffer[meta_place.index].global_organisation_mode = inode.global_organisation_mode;
    metabuffer[meta_place.index].internal_organisation_mode = inode.internal_organisation_mode;
    metabuffer[meta_place.index].start_block = inode.start_block;
    nbr_free_metas--;

    // Write back the modified block containing the metadata
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fwrite(metabuffer, sizeof(Meta), meta_per_block, MS);


    Block bloc;
    Block buffer;
    int k = 0;

    if(inode.global_organisation_mode == 0) {
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
        // chained mode
        int next_block;
        // read the allocation table 
        int allocation_table_buffer[NUM_OF_BLOCKS];
        readAllocationTable(MS, allocation_table_buffer);

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
                for (int m = 0; m < NUM_OF_BLOCKS; m++)
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

        writeAllocationTable(MS, allocation_table_buffer);
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
    // strcpy(inode.name, "test");
    // inode.file_size_in_blocks = 5;
    // inode.file_size_in_records = 33;
    // inode.global_organisation_mode = 0;
    // inode.internal_organisation_mode = 0;
    // inode.start_block = 0;
    promptInode(MS, &inode);
    fillStartBlock(MS, &inode);

    if(inode.start_block != -1) {
        displayInode(inode);
        fillFileDataAndMeta(MS, inode);
        displayFileData(MS, inode);
        
    } else {
        displayErrors();
        resetFlags();
    }
    
}

void deleteFile(FILE* MS, char* filename) {

    Position meta_place = searchMetadata(MS, filename);

    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return;
    }



    // Move the file cursor to the block where the metadata is located
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);

    // Read the metadata block into metabuffer
    Meta metabuffer[meta_per_block];
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);
    
    Meta inodebuffer;
    strcpy(inodebuffer.name, metabuffer[meta_place.index].name);
    inodebuffer.file_size_in_blocks = metabuffer[meta_place.index].file_size_in_blocks;
    inodebuffer.file_size_in_records = metabuffer[meta_place.index].file_size_in_records;
    inodebuffer.global_organisation_mode = metabuffer[meta_place.index].global_organisation_mode;
    inodebuffer.internal_organisation_mode = metabuffer[meta_place.index].internal_organisation_mode;
    inodebuffer.start_block = metabuffer[meta_place.index].start_block;

    initInode(&metabuffer[meta_place.index]);
    fseek(MS, (long) (-1) * meta_per_block * sizeof(Meta), SEEK_CUR);
    fwrite(metabuffer, sizeof(Meta), meta_per_block, MS);
    nbr_free_metas++;



    int allocation_table_buffer[NUM_OF_BLOCKS];
    readAllocationTable(MS, allocation_table_buffer);

    Block buffer;
    int next_block = inodebuffer.start_block;
    fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);

    for (int i = 0; i < inodebuffer.file_size_in_blocks; i++)
    {
        allocation_table_buffer[next_block] = 0;

        fread(&buffer, sizeof(Block), 1, MS);
        next_block = buffer.next;
        initBlock(&buffer);
        fseek(MS, (long) -sizeof(Block), SEEK_CUR);
        fwrite(&buffer, sizeof(Block), 1, MS);

        fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);
    }
    
    writeAllocationTable(MS, allocation_table_buffer);

}

void insertRecord(FILE* MS, char* filename) {
    Position meta_place = searchMetadata(MS, filename);

    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return;
    }

    // Move the file cursor to the block where the metadata is located
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);

    // Read the metadata block into metabuffer
    Meta metabuffer[meta_per_block];
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);
    
    Meta* inode = &metabuffer[meta_place.index];

    int allocation_table_buffer[NUM_OF_BLOCKS];
    readAllocationTable(MS, allocation_table_buffer);

    if(inode->file_size_in_records % FB == 0) {
        // block is full
        if(inode->global_organisation_mode == 0) {
            // if the next block is free 
            if(allocation_table_buffer[inode->start_block + inode->file_size_in_blocks] == 0) {
                allocation_table_buffer[inode->start_block + inode->file_size_in_blocks] = 1;
                inode->file_size_in_blocks++;
                inode->file_size_in_records++;
                
                Client client = generateClient(inode->file_size_in_records);
                
                Block bloc;
                Block buffer;
                // if records are not sorted
                if(inode->internal_organisation_mode == 0) {
                    // get the last block of the file and update the next
                    fseek(MS, (first_data_region_block + inode->start_block + inode->file_size_in_blocks - 1 ) * sizeof(Block), SEEK_SET);
                    fread(&buffer, sizeof(Block), 1, MS);
                    buffer.next = inode->start_block + inode->file_size_in_blocks;
                    fseek(MS, (long) -sizeof(Block), SEEK_CUR);
                    fwrite(&buffer, sizeof(Block), 1, MS);

                    initBlock(&bloc);
                    bloc.entries[0] = client;
                    bloc.next = -1;
                    buffer = bloc;
                    fwrite(&buffer, sizeof(Block), 1, MS);
                } else {
                    Position record_index = searchRecord(MS, filename, client.ID);
                    if(record_index.block_index != -1) {
                        fseek(MS, (first_data_region_block + record_index.block_index) * sizeof(Block), SEEK_SET);
                        Block buffer;
                        
                        int k = records_per_block - 1;
                        fread(&buffer, sizeof(Block), 1, MS);
                            
                        Client temp = buffer.entries[records_per_block - 1];
                        for (k = records_per_block - 2; k > record_index.index; k--)
                        {
                            buffer.entries[k + 1] = buffer.entries[k];
                        }
                        buffer.entries[record_index.index] = client;

                        Client temp2;

                        while(buffer.next != -1) {
                            fseek(MS, (first_data_region_block + buffer.next) * sizeof(Block), SEEK_SET);
                            fread(&buffer, sizeof(Block), 1, MS);
                            temp2 = buffer.entries[records_per_block - 1];
                            for (k = records_per_block - 2; k > record_index.index; k--)
                            {
                                buffer.entries[k + 1] = buffer.entries[k];
                            }
                            buffer.entries[0] = temp;
                            temp = temp2;
                        }
                    }
                    
                    
                }
            } else {
                // the next block is not free
                int start_block = findFreeAdjacentBlock(MS, inode->file_size_in_blocks);

                if(start_block < 0) {
                    contiguous_space_issue = true;
                    return;
                }
                inode->start_block = start_block;
                int previous_start_block = inode->start_block;
                Block buffer1, buffer2;
                for (int i = 0; i < inode->file_size_in_blocks; i++)
                {
                    fseek(MS, (previous_start_block + first_data_region_block) * sizeof(Block), SEEK_SET);
                    fread(&buffer1, sizeof(Block), 1, MS);
                    allocation_table_buffer[previous_start_block] = 0;
                    previous_start_block = buffer1.next;
                    buffer2 = buffer1;
                    fseek(MS, (start_block + first_data_region_block) * sizeof(Block), SEEK_SET);
                    allocation_table_buffer[start_block] = 1;
                    if(i == inode->file_size_in_blocks - 1) {
                        buffer2.next = -1;
                    } else {
                        buffer2.next = i + 1;
                    }
                    start_block = buffer2.next;
                    fwrite(&buffer2, sizeof(Block), 1, MS);
                }
            }
        }
    } else {

    }



    writeAllocationTable(MS, allocation_table_buffer);
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fwrite(metabuffer, sizeof(Meta), meta_per_block, MS);

}

void renameFile(FILE* MS, char* old_filename, char* new_filename) {
        
    Position meta_place = searchMetadata(MS, old_filename);

    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return;
    }
    if(strlen(new_filename) > MAX_NAME_LENGTH) {
        printf("Error: the new file name is too big");
        return;
    }

    Meta metabuffer[meta_per_block];
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fread(&metabuffer, sizeof(Meta), meta_per_block, MS);
    Meta inode = metabuffer[meta_place.index];
    strcpy(inode.name, new_filename);
    metabuffer[meta_place.index] = inode;
    fseek(MS, (long) -sizeof(Meta) * meta_per_block, SEEK_CUR);
    fwrite(&metabuffer, sizeof(Meta), meta_per_block, MS);

}

Position searchRecord(FILE* MS, char* filename, int id) {

    Position p;
    p.block_index = -1;
    p.index = -1;

    Position meta_place = searchMetadata(MS, filename);

    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return p;
    }
    if(strlen(filename) > MAX_NAME_LENGTH) {
        printf("Error: the new file name is too big");
        return p;
    }

    Meta metabuffer[meta_per_block];
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);
    
    Meta* inode = &metabuffer[meta_place.index];

    fseek(MS, (first_data_region_block + inode->start_block) * sizeof(Block), SEEK_SET);
    Client data[inode->file_size_in_records];
    Block buffer;

    for (int i = 0; i < inode->file_size_in_blocks; i++)
    {
        fread(&buffer, sizeof(Block), 1, MS);
        for (int j = 0; i < records_per_block; j++)
        {
            data[i] = buffer.entries[j];
        }
        fseek(MS, (first_data_region_block + buffer.next) * sizeof(Block), SEEK_SET);
    }

    if(inode->global_organisation_mode == 0) {
        for (int i = 0; i < inode->file_size_in_records; i++)
        {
            if(data[i].ID == id) {
                p.block_index = i / records_per_block;
                p.index = i % records_per_block;
                return p;
            }
        }
    } else {
        int left = 0;
        int right = inode->file_size_in_records - 1;
        int index = binarySearch(data, left, right, id);
        if(index != -1) {
            p.block_index = index / records_per_block;
            p.index = index % records_per_block;
        }
    }
    
    return p;


}

int binarySearch(Client clients[], int left, int right, int val) {
    while (left <= right) {
        int mid_index = left + (right - left) / 2;

        if (clients[mid_index].ID == val)
            return mid_index;

        if (clients[mid_index].ID < val)
            left = mid_index + 1;

        else
            right = mid_index - 1;
    }

    // If we reach here, then element was not present
    return -1;
}

void displayMenu() {
    printf("\n===== Main Menu =====\n");
    printf("1. Initialize the file system\n");
    printf("2. Print file system\n");
    printf("3. Print Blocks\n");
    printf("4. Empty disk\n");
    printf("5. Print Allocation table\n");
    printf("6. Create file\n");
    printf("7. Delete file\n");
    printf("8. Rename file\n");
    printf("9. Insert record\n");
    printf("0. Exit\n");
    printf("=====================\n");
    printf("Enter your choice: ");
}



int main() {
    srand(time(NULL));


    int choice;
    FILE* MS = fopen("MS.bin", "wb+");


    do {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                initFileSystem(MS);
                printf("the file system initialized successfully");
                break;
            case 2:
                printFileSystem(MS);

                break;
            case 3:
                printBlocks(MS);
                break;
            case 4:
                EmptyDisk(MS);;
                break;
            case 5:
                int allocation_table_buffer[NUM_OF_BLOCKS];
                readAllocationTable(MS, allocation_table_buffer);
                printAllocationTable(MS, allocation_table_buffer);
                break;
            case 6:
                createFile(MS);
                resetFlags();
                break;
            case 7:
                char filename[MAX_NAME_LENGTH];
                printf("Enter the file name that you want to delete: ");
                scanf(" %s", filename);
                deleteFile(MS, filename);
                resetFlags();
                break;
            case 8:
                char old_filename[MAX_NAME_LENGTH];
                printf("Enter the file name that you want to rename: \n");
                scanf(" %s", old_filename);
                char new_filename[MAX_NAME_LENGTH];
                printf("Enter the new name: ");
                scanf(" %s", new_filename);
                renameFile(MS, old_filename, new_filename);
                resetFlags();
                break;
            case 9:
                char filename2[MAX_NAME_LENGTH];
                printf("Enter the file name that you want to insert record in: ");
                scanf(" %s", filename2);
                insertRecord(MS, filename2);
                resetFlags();
            case 0:
                printf("Exiting the program.");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);



    return 0;
}





