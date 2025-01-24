#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// definitions
/*---------------------------------------------------------------------------------*/
#define NUM_OF_BLOCKS 4            
#define FB 3
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
    int nbr_records;
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
const int records_per_block = (sizeof(Block) - (2 * sizeof(int))) / sizeof(Client);

const int meta_per_block = (sizeof(Block) - (2 * sizeof(int))) / sizeof(Meta);
const int nbr_meta_blocks = ceil((double)NUM_OF_META / ((sizeof(Block) - (2 * sizeof(int))) / sizeof(Meta)));

const int first_data_region_block = ceil((double)NUM_OF_META / ((sizeof(Block) - (2 * sizeof(int))) / sizeof(Meta))) + 1;

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

char FILE_NAME_ALREADY_EXIST_ERROR_MSG[] = "Error: File name already exists";
bool is_file_name_duplicate = false;

char RECORD_NOT_EXIST_ERROR_MSG[] = "Error: Record does not exist";
bool record_not_found_flag = false;

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
void displayInode(FILE* MS,char* filename);
void promptInode(FILE* MS, Meta *inode);
void fillStartBlock(FILE* MS, Meta* inode);
Position searchMetadata(FILE* MS, char name[]);
Client generateClient(int id);
void fillFileDataAndMeta(FILE* MS, Meta inode);
void displayFileData(FILE* MS, char* filename);
void createFile(FILE* MS);
void deleteFile(FILE* MS, char* filename);
void initInode(Meta* inode);
void insertRecord(FILE* MS, char* filename);
void renameFile(FILE* MS, char* old_filename, char* new_filename);
bool searchRecord(FILE* MS, char* filename, int id, Position* p);
int binarySearch(Client clients[], int left, int right, int val);
int binarySearchInsertion(Client clients[], int left, int right, int val);
void validation(int file_size_in_blocks);
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

    printf("The file system initialized successfully");
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
    bloc->nbr_records = 0;
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
        printf("block: %d, number of records: %d, next: %d\n", i, buffer.nbr_records, buffer.next);
    }
}

void EmptyDisk(FILE* MS) {

    MS = fopen("MS.bin", "wb+");

    initFileSystem(MS);
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


void validation(int file_size_in_blocks) {
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
    char buffer[512];
    strcpy(buffer, "");

    if(is_storage_full) {
        strcat(buffer, NO_FREE_BLOCKS_ERROR_MSG);
        strcat(buffer, "\n");
    } else {
        if(is_file_too_big) {
            strcat(buffer, FILE_TOO_BIG_ERROR_MSG);
            strcat(buffer, "\n");
        }
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

    if(is_file_name_duplicate) {
        strcat(buffer, FILE_NAME_ALREADY_EXIST_ERROR_MSG);
        strcat(buffer, "\n");
    }

    if(record_not_found_flag) {
        strcat(buffer, RECORD_NOT_EXIST_ERROR_MSG);
        strcat(buffer, "\n");
    }

    printf("%s", buffer);
}

void resetFlags() {
    if(nbr_free_blocks != 0) {
        is_storage_full = false;
    }
    if(nbr_free_metas != 0) {
        is_metadata_full = false;
    }
    is_file_too_big = false;
    contiguous_space_issue = false;
    file_not_found_flag = false;
    is_file_name_duplicate = false;
    record_not_found_flag = false;
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
    printf("allocation table: \n");
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
    fseek(MS, first_data_region_block * sizeof(Block), SEEK_SET);
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
    
    fseek(MS, sizeof(Block), SEEK_SET);

    for (int i = 0; i < nbr_meta_blocks; i++)
    {
        Meta metabuffer[meta_per_block];
        fread(metabuffer, sizeof(Meta), meta_per_block, MS);

        for (int j = 0; j < meta_per_block; j++)
        {
            if(strcmp(metabuffer[j].name, inode->name) == 0) {
                // if the same name
                is_file_name_duplicate = true;
                return;
            }
        }
        
    }

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

    validation(inode->file_size_in_blocks);
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

void displayInode(FILE* MS,char* filename) {
    // Display the file metadata
    Position p = searchMetadata(MS, filename);
    if(p.block_index < 0) {
        file_not_found_flag = true;
        return;
    }
    Meta metabuffer[meta_per_block];
    fseek(MS, (p.block_index) * sizeof(Block), SEEK_SET);
    fread(&metabuffer, sizeof(Meta), meta_per_block, MS);
    Meta inode = metabuffer[p.index];
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
    // move the file curosr to the metadata region
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
        for (int i = 0; i < inode.file_size_in_blocks; i++)
        {
            initBlock(&bloc);
            int j = 0;
            while (j < records_per_block && k < inode.file_size_in_records)
            {
                bloc.entries[j] = generateClient(k);
                bloc.nbr_records++;
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
                bloc.nbr_records++;
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

void displayFileData(FILE* MS, char* filename) {

    Position p = searchMetadata(MS, filename);
    if(p.block_index == -1) {
        file_not_found_flag = true;
        return;
    } else {
        Meta metabuffer[meta_per_block];
        fseek(MS, (p.block_index) * sizeof(Block), SEEK_SET);
        fread(metabuffer, sizeof(Meta), meta_per_block, MS);
        Meta inode = metabuffer[p.index];

        Block buffer;
        int next_block = inode.start_block;
        int k = 0;  // Record counter

        printf("Filename: %s\n", inode.name);

        for (int i = 0; i < inode.file_size_in_blocks; i++) {
            // Move the file cursor to the current block
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
    if(is_file_name_duplicate == true) {
        return;
    } else {
        if(inode.file_size_in_blocks > nbr_free_blocks) {
            is_file_too_big = true;
        } else {
            fillStartBlock(MS, &inode);

            if(inode.start_block != -1) {
                displayInode(MS, inode.name);
                fillFileDataAndMeta(MS, inode);
                displayFileData(MS, inode.name);
                
            }
        }

    }
}

bool search(FILE* MS, char* filename, int id, Position* p) {
    Position meta_place = searchMetadata(MS, filename);
    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return false;
    }

    Meta metabuffer[meta_per_block];
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);

    Meta inode = metabuffer[meta_place.index];


    Block buffer;
    if(inode.internal_organisation_mode == 0) {
        // not sorted mode

        int k = 0;
        int j = 0;
        int next_block = inode.start_block;
        for (int i = 0; i < inode.file_size_in_blocks; i++)
        {
            int j = 0;

            fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);
            fread(&buffer, sizeof(Block), 1, MS);
            while(j < records_per_block && k < inode.file_size_in_records) {
                if(buffer.entries[j].ID == id) {
                    p->block_index = next_block;
                    p->index = j;
                    return true;
                } else {
                    j++;
                    k++;    
                }
                
            }
            next_block = buffer.next;
        }

        p->block_index = k / FB;
        p->index = j + 1;
        return false;

    } else {
        // sorted mode
        if(inode.global_organisation_mode == 0) {
            // contingues mode
            int middle;
            int start = inode.start_block;
            int end = inode.start_block + inode.file_size_in_records - 1;
            
            int index;

            while(start < end) {
                middle = (start + end) / 2;

                fseek(MS, (first_data_region_block + middle) * sizeof(Block), SEEK_SET);
                fread(&buffer, sizeof(Block), 1, MS);

                if(id >= buffer.entries[0].ID && id <= buffer.entries[buffer.nbr_records - 1].ID) {
                    // binary search inside the block
                    index = binarySearchInsertion(buffer.entries, 0, buffer.nbr_records - 1, id);
                    if(index == -1) {
                        record_not_found_flag = true;
                        return false;
                    } else {
                        p->block_index = middle;
                        p->index = index;
                        return true;
                    }

                } else {
                    if(id < buffer.entries[0].ID) {
                        end = middle - 1;
                    } else {
                        start = middle + 1;
                    }
                }
            }

            // if the id is not found
            p->block_index = start;
            p->index = index;
            return false;

        } else {
            // chained mode
            int next_block = inode.start_block;
            for (int i = 0; i < inode.file_size_in_blocks; i++)
            {
                fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);
                // Read the block
                fread(&buffer, sizeof(Block), 1, MS);

                int index = binarySearch(buffer.entries, 0, buffer.nbr_records - 1, id);


                p->block_index = next_block;
                p->index = index;
                return true;

                next_block = buffer.next;
                return false;
            }
        }
    }
}

bool searchRecord(FILE* MS, char* filename, int id, Position* p) {

    Position meta_place = searchMetadata(MS, filename);
    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return false;
    }

    Meta metabuffer[meta_per_block];
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);

    Meta inode = metabuffer[meta_place.index];


    Block buffer;
    if(inode.internal_organisation_mode == 0) {
        // not sorted mode

        int k = 0;
        int next_block = inode.start_block;
        for (int i = 0; i < inode.file_size_in_blocks; i++)
        {
            int j = 0;
            
            fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);
            fread(&buffer, sizeof(Block), 1, MS);
            while(j < records_per_block && k < inode.file_size_in_records) {
                if(buffer.entries[j].ID == id) {
                    p->block_index = next_block;
                    p->index = j;
                    return true;
                } else {
                    j++;
                    k++;
                }
            }

            next_block = buffer.next;
            if(next_block == -1) {
                record_not_found_flag = true;
                return false; // not found
            }
        }
    } else {
        // sorted mode
        if(inode.global_organisation_mode == 0) {
            // contingues mode
            int middle;
            int start = inode.start_block;
            int end = inode.start_block + inode.file_size_in_records - 1;

            while(start < end) {
                middle = (start + end) / 2;

                fseek(MS, (first_data_region_block + middle) * sizeof(Block), SEEK_SET);
                fread(&buffer, sizeof(Block), 1, MS);

                if(id >= buffer.entries[0].ID && id <= buffer.entries[buffer.nbr_records - 1].ID) {
                    // binary search inside the block
                    int index = binarySearch(buffer.entries, 0, buffer.nbr_records - 1, id);
                    if(index == -1) {
                        record_not_found_flag = true;
                        return false;
                    } else {
                        p->block_index = middle;
                        p->index = index;
                        return true;
                    }

                } else {
                    if(id < buffer.entries[0].ID) {
                        end = middle - 1;
                    } else {
                        start = middle + 1;
                    }
                }
            }

            // if the id is not found
            record_not_found_flag = true;
            return false;

        } else {
            // chained mode
            int next_block = inode.start_block;
            for (int i = 0; i < inode.file_size_in_blocks; i++)
            {
                fseek(MS, (first_data_region_block + next_block) * sizeof(Block), SEEK_SET);
                // Read the block
                fread(&buffer, sizeof(Block), 1, MS);

                int index = binarySearch(buffer.entries, 0, buffer.nbr_records - 1, id);

                if(index != -1) {
                    p->block_index = next_block;
                    p->index = index;
                    return true;
                }
                
                next_block = buffer.next;
                if(next_block == -1) {
                    record_not_found_flag = true;
                    return false;  // not found
                } 
            }
        }
    }
}

void insertRecord(FILE* MS, char* filename) {

    Position meta_place = searchMetadata(MS, filename);
    if(meta_place.block_index == -1) {
        file_not_found_flag = true;
        return;
    }

    Meta metabuffer[meta_per_block];
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fread(metabuffer, sizeof(Meta), meta_per_block, MS);

    Meta inode = metabuffer[meta_place.index];

    Position p;
    bool record_position = search(MS, filename, inode.file_size_in_records, &p);

    int allocation_table_buffer[NUM_OF_BLOCKS];
    readAllocationTable(MS, allocation_table_buffer);


    if(inode.global_organisation_mode == 0) {
        // mode contigues
        Block bloc;
        Block buffer;
        Client client = generateClient(inode.file_size_in_records);
        if(inode.file_size_in_records % FB == 0) {
            // if the last block is full
            if(allocation_table_buffer[inode.start_block + inode.file_size_in_blocks] == 0) {
                // if the next block is free
                nbr_free_blocks--;
                allocation_table_buffer[inode.start_block + inode.file_size_in_blocks] = 1;

                initBlock(&bloc);
                bloc.nbr_records = 1;

                if(inode.internal_organisation_mode == 0) {
                    // if records are not sorted 

                    fseek(MS, (first_data_region_block + inode.start_block + inode.file_size_in_blocks - 1) * sizeof(Block), SEEK_SET);
                    fread(&buffer, sizeof(Block), 1, MS);

                    buffer.next = inode.start_block + inode.file_size_in_blocks; 

                    fseek(MS, (long) -sizeof(Block), SEEK_CUR);
                    fwrite(&buffer, sizeof(Block), 1, MS);

                    bloc.entries[0] = client;
                    buffer = bloc;
                    fwrite(&buffer, sizeof(Block), 1, MS);
                } else {
                    // if records are sorted
                    Position p;
                    bool found_place = searchRecord(MS, inode.name, client.ID, &p);
                    if(found_place == false) {
                        return;
                    } else {
                        fseek(MS, (first_data_region_block + p.block_index) * sizeof(Block), SEEK_SET);
                        fread(&buffer, sizeof(Block), 1, MS);

                        Client x = buffer.entries[buffer.nbr_records - 1];
                        int k = buffer.nbr_records - 1;

                        while (k > p.index)
                        {
                            buffer.entries[k] = buffer.entries[k-1];
                            k--;
                        }

                        buffer.entries[p.index] = client;

                        fseek(MS, (long) -sizeof(Block), SEEK_CUR);
                        fwrite(&buffer, sizeof(Block), 1, MS);
                        
                        fread(&buffer, sizeof(Block), 1, MS);

                        while(buffer.next != -1) {
                            Client temp1 = buffer.entries[buffer.nbr_records - 1];
                            k = buffer.nbr_records - 1;

                            while (k > 0)
                            {
                                buffer.entries[k] = buffer.entries[k-1];
                                k--;
                            }
                            buffer.entries[0] = x;
                            x = temp1;
                            fread(&buffer, sizeof(Block), 1, MS);
                        }
                        //we have now buffer.next = -1;
                        buffer.next = inode.start_block + inode.file_size_in_blocks; 
                        bloc.entries[0] = client; 
                        buffer = bloc;
                        fwrite(&buffer, sizeof(Block), 1, MS);
                    }
                }

                inode.file_size_in_blocks++;
                inode.file_size_in_records++;
            } else {
                // if the next block is not free
                int new_start_block = findFreeAdjacentBlock(MS, inode.file_size_in_blocks + 1);
                if(new_start_block < 0) {
                    contiguous_space_issue = true;
                    return;
                } else {
                    nbr_free_blocks--;
                    inode.start_block = new_start_block;
                    Block buffer1, buffer2;
                    int old_next_block = inode.start_block;
                    for (int i = 0; i < inode.file_size_in_blocks; i++)
                    {
                        fseek(MS, (first_data_region_block + old_next_block) * sizeof(Block), SEEK_SET);
                        fread(&buffer1, sizeof(Block), 1, MS);
                        allocation_table_buffer[old_next_block] = 0;
                        
                        buffer2 = buffer1;

                        initBlock(&buffer1);
                        fseek(MS, (long) -sizeof(Block), SEEK_CUR);
                        fwrite(&buffer1, sizeof(Block), 1, MS);

                        fseek(MS, (first_data_region_block + new_start_block) * sizeof(Block), SEEK_SET);
                        fwrite(&buffer2, sizeof(Block), 1, MS);
                        // no need to allocation_table_buffer[new_start_block] = 1; becasue it is already done
                        old_next_block++;
                        new_start_block++;
                    }
                    initBlock(&bloc);

                    bloc.entries[0] = client;
                    bloc.nbr_records = 1;
                    buffer = bloc;
                    fwrite(&buffer, sizeof(Block), 1, MS);
                    
                }



            }
        } else {
            // if the last block is not full
            fseek(MS, (first_data_region_block + inode.start_block + inode.file_size_in_blocks - 1)* sizeof(Block), SEEK_SET);
            fread(&buffer, sizeof(Block), 1, MS);
            buffer.entries[buffer.nbr_records] = client;
            fseek(MS, (long) -sizeof(Block), SEEK_CUR);
            fwrite(&buffer, sizeof(Block), 1, MS);
            inode.file_size_in_records++;
        }

    }
    
    writeAllocationTable(MS, allocation_table_buffer);
    metabuffer[meta_place.index] = inode;
    fseek(MS, (meta_place.block_index) * sizeof(Block), SEEK_SET);
    fwrite(metabuffer, sizeof(Meta), meta_per_block, MS);


    //         } else {
    //             // the next block is not free
    //             int start_block = findFreeAdjacentBlock(MS, inode->file_size_in_blocks);

    //             if(start_block < 0) {
    //                 contiguous_space_issue = true;
    //                 return;
    //             }
    //             inode->start_block = start_block;
    //             int previous_start_block = inode->start_block;
    //             Block buffer1, buffer2;
    //             for (int i = 0; i < inode->file_size_in_blocks; i++)
    //             {
    //                 fseek(MS, (previous_start_block + first_data_region_block) * sizeof(Block), SEEK_SET);
    //                 fread(&buffer1, sizeof(Block), 1, MS);
    //                 allocation_table_buffer[previous_start_block] = 0;
    //                 previous_start_block = buffer1.next;
    //                 buffer2 = buffer1;
    //                 fseek(MS, (start_block + first_data_region_block) * sizeof(Block), SEEK_SET);
    //                 allocation_table_buffer[start_block] = 1;
    //                 if(i == inode->file_size_in_blocks - 1) {
    //                     buffer2.next = -1;
    //                 } else {
    //                     buffer2.next = i + 1;
    //                 }
    //                 start_block = buffer2.next;
    //                 fwrite(&buffer2, sizeof(Block), 1, MS);
    //             }
    //         }
    //     }
    // } else {

    // }


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

int binarySearchInsertion(Client clients[], int left, int right, int val) {
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
    return left;
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
    printf("10. Search record\n");
    printf("11. Display file data\n");
    printf("0. Exit\n");
    printf("=====================\n");
    printf("Enter your choice: ");
}

int main() {
    srand(time(NULL));


    int choice;
    FILE* MS = fopen("MS.bin", "wb+");
    
    char filename[MAX_NAME_LENGTH];
    int id;
    char oldname[MAX_NAME_LENGTH];
    int allocation_table_buffer[NUM_OF_BLOCKS];
    do {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                initFileSystem(MS);
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
                
                readAllocationTable(MS, allocation_table_buffer);
                printAllocationTable(MS, allocation_table_buffer);
                break;
            case 6:
                createFile(MS);
                if(is_file_name_duplicate || is_file_too_big) {
                    displayErrors();
                    resetFlags();
                }
                
                break;
            case 7:
                memset(filename, 0, sizeof(filename));
                printf("Enter the file name that you want to delete: ");
                scanf(" %s", filename);
                deleteFile(MS, filename);
                resetFlags();
                break;
            case 8:
                memset(filename, 0, sizeof(filename));
                memset(oldname, 0, sizeof(oldname));
                printf("Enter the file name that you want to rename: \n");
                scanf(" %s", oldname);
                printf("Enter the new name: ");
                scanf(" %s", filename);
                renameFile(MS, oldname, filename);
                displayErrors();
                resetFlags();
                break;
            case 9:
                memset(filename, 0, sizeof(filename));
                printf("Enter the file name that you want to insert record in: ");
                scanf(" %s", filename);
                insertRecord(MS, filename);
                displayErrors();
                resetFlags();
                break;

            case 10:
                memset(filename, 0, sizeof(filename));
                printf("Enter the file name that you want to search record in: ");
                scanf(" %s", filename);
                id = 0;
                printf("Enter id: ");
                scanf("%d", &id);
                Position p;
                bool check = searchRecord(MS, filename, id, &p);

                if(check) {
                    printf("record position: \n");
                    printf("Block: %d, record: %d", p.block_index, p.index);
                } else {
                    displayErrors();
                    resetFlags();
                }
                break;
            case 11:
                memset(filename, 0, sizeof(filename));
                printf("Enter the file name: ");
                scanf(" %s", filename);
                displayFileData(MS, filename);
                displayErrors();
                resetFlags();
                break;
            case 0:
                printf("Exiting the program.");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);



    return 0;
}





