<h1>Simplified simulator of a file management system</h1>

<h2>Made by: Bouchefra yassine</h2>
<h3>Section: B</h3>
<h3>Group 2</h3>
<h3>Matricule: 232331388118</h3>


<h1>Report</h1>
<h2>Structures</h2>
<h3>Client</h3>
<ul>
  <li>int ID: A unique identifier for the client.</li>
  <li>char name[MAX_NAME_LENGTH]: The name of the client (up to MAX_NAME_LENGTH characters).</li>
  <li>int age: The age of the client.</li>
  <li>float balance: The account balance of the client.</li>
  <li>int LD: Logical deletion flag (1 for logically deleted, 0 for active).</li>
</ul>

<h3>Block</h3>
<ul>
  <li>Client entries[FB]: An array of Client records stored in the block, where FB is the maximum number of records per block.</li>
  <li>int next: A pointer to the next block (used for chaining files). If no chaining, it is set to -1.</li>
</ul>

<h3>Meta</h3>
<ul>
  <li>char name[MAX_NAME_LENGTH]: The name of the file.</li>
  <li>int start_block: The index of the first block of the file.</li>
  <li>int file_size_in_blocks: The total number of blocks occupied by the file.</li>
  <li>int file_size_in_records: The total number of records in the file.</li>
  <li>int global_organisation_mode: Organization mode of the file:</li>
  <ul>
    <li>0 for contiguous organization.</li>
    <li>1 for chained organization.</li>
  </ul>
  <li>int internal_organisation_mode: Organization mode of records within blocks:</li>
  <ul>
    <li>0 for unsorted records.</li>
    <li>1 for sorted records.</li>
  </ul>
</ul>

<h3>Position</h3>
<h4>This struct represents a specific position within the file system, such as a metadata entry or a record.</h4>
<ul>
  <li>int block_index: The index of the block containing the desired entry.</li>
  <li>int index: The index within the block (e.g., for a Client record).</li>
</ul>




<hr>
<h2>Functions</h2>
<h3>initFileSystem(FILE* MS):</h3>
<h4>Initializes the file system by setting up essential structures:</h4>
<ul>
  <li>Allocation table: Marks all blocks as free.</li>
  <li>Metadata: Resets metadata entries (inodes) to their initial state.</li>
  <li>Data blocks: Initializes each block with default values for records and sets the next pointer to -1.</li>
  <li>Writes the initialized data to the simulated file system (MS).</li>
</ul>
  

<h3>printFileSystem(FILE* MS):</h3>
<h4>Displays the current state of the file system, including:</h4>
<ul>
  <li>Allocation table: Shows which blocks are free or occupied.</li>
  <li>Metadata: Lists all metadata entries (e.g., file name, size, and organization modes).</li>
</ul>

<h3>initBlock(Block* bloc):</h3>
<ul>
  <li>Resets the data in a block:</li>
  <li>Initializes all entries in the block to default values (ID=0, name="", etc.).</li>
  <li>Sets the next pointer to -1, indicating no chaining.</li>
</ul>

<h3>initInode(Meta* inode):</h3>
<ul>
  <li>Initializes a metadata entry (inode) to default values:</li>
  <li>Sets file name to an empty string.</li>
  <li>Resets block indices, file size, and organization modes to invalid values.</li>
</ul>

<h3>printBlocks(FILE* MS):</h3>
<ul>
  <li>Displays all blocks in the file system, including:</li>
  <li>The next pointer of each block (useful for chained files).</li>
  <li>Helps visualize the logical structure of the blocks.</li>
</ul>

<h3>EmptyDisk(FILE* MS):</h3>
<ul>
  <li>Clears the entire file system:</li>
  <li>Re-initializes the allocation table, metadata, and blocks.</li>
  <li>Effectively resets the disk to its initial state.</li>
</ul>

<h3>validateConstraints(int file_size_in_blocks):</h3>
<ul> 
  <li>Checks if the file system can accommodate a file of the given size.</li>
  <li>Validates the following constraints: <ul> <li><b>Storage Full:</b> If there are no free blocks, sets the <code>is_storage_full</code> flag to <code>true</code>.</li> <li><b>File Too Big:</b> If the file size in blocks exceeds the number of free blocks, sets the <code>is_file_too_big</code> flag to <code>true</code>.</li> <li><b>No Free Metadata:</b> If there are no free metadata entries, sets the <code>is_metadata_full</code> flag to <code>true</code>.</li> </ul> </li> <li>Prepares the system to handle errors if constraints are violated.</li> </ul> <h3>displayErrors():</h3> <ul> <li>Displays all active error messages based on the current state of the system flags.</li> <li>Error messages include: <ul> <li><b>No Free Blocks:</b> Displays <code>NO_FREE_BLOCKS_ERROR_MSG</code> if storage is full.</li> <li><b>File Too Big:</b> Displays <code>FILE_TOO_BIG_ERROR_MSG</code> if the file size exceeds available space.</li> <li><b>No Free Metadata:</b> Displays <code>NO_FREE_META_ERROR_MSG</code> if no free metadata slots are available.</li> <li><b>Contiguous Space Issue:</b> Displays <code>CONTIGUOUS_SPACE_ERROR_MSG</code> if contiguous blocks for a file cannot be allocated.</li> <li><b>File Not Found:</b> Displays <code>FILE_NOT_FOUND_ERROR_MSG</code> if a requested file does not exist.</li> </ul> </li> <li>Prints all collected error messages to standard output.</li> </ul> <h3>resetFlags():</h3> <ul> <li>Resets all error flags to their default state.</li> <li>Conditions for resetting: <ul> <li><b>Storage Full:</b> Resets <code>is_storage_full</code> if free blocks are available.</li> <li><b>No Free Metadata:</b> Resets <code>is_metadata_full</code> if metadata entries are available.</li> <li><b>File Too Big:</b> Resets <code>is_file_too_big</code> unconditionally after processing.</li> <li><b>Contiguous Space Issue:</b> Resets <code>contiguous_space_issue</code> unconditionally after processing.</li> <li><b>File Not Found:</b> Resets <code>file_not_found_flag</code> unconditionally after processing.</li> </ul> </li> <li>Prepares the system for the next operation by clearing errors from previous operations.</li> </ul>



<h3>readAllocationTable(FILE* MS, int allocation_table_buffer[]):</h3> <ul> <li>Reads the allocation table from the file system into a buffer.</li> <li>Steps: <ul> <li>Moves the file pointer to the start of the file system.</li> <li>Reads the allocation table (an array of integers) from the file into <code>allocation_table_buffer</code>.</li> <li>Repositions the file pointer to the end of the allocation table for further operations.</li> </ul> </li> </ul> <h3>writeAllocationTable(FILE* MS, int allocation_table_buffer[]):</h3> <ul> <li>Writes the allocation table from a buffer back to the file system.</li> <li>Steps: <ul> <li>Moves the file pointer to the start of the file system.</li> <li>Writes the contents of <code>allocation_table_buffer</code> to the file.</li> <li>Repositions the file pointer to the end of the allocation table for further operations.</li> </ul> </li> </ul> <h3>initAllocationTable(FILE* MS, int allocation_table_buffer[]):</h3> <ul> <li>Initializes the allocation table, marking all blocks as free.</li> <li>Steps: <ul> <li>Sets all elements of <code>allocation_table_buffer</code> to <code>0</code>, indicating all blocks are unallocated.</li> <li>Writes the initialized allocation table to the file system using <code>writeAllocationTable</code>.</li> </ul> </li> </ul> <h3>findFreeAdjacentBlock(FILE* MS, int file_size_in_blocks):</h3> <ul> <li>Finds a series of contiguous free blocks to allocate a file of the specified size.</li> <li>Steps: <ul> <li>Reads the allocation table into a buffer using <code>readAllocationTable</code>.</li> <li>Searches for a sequence of free blocks of length <code>file_size_in_blocks</code>.</li> <li>If found: <ul> <li>Marks the blocks as allocated in the buffer.</li> <li>Updates the allocation table in the file system using <code>writeAllocationTable</code>.</li> <li>Returns the index of the first block in the sequence.</li> </ul> </li> <li>If not found: <ul> <li>Sets the <code>contiguous_space_issue</code> flag to <code>true</code>.</li> <li>Returns <code>-1</code>.</li> </ul> </li> </ul> </li> </ul> <h3>printAllocationTable(FILE* MS, int allocation_table_buffer[]):</h3> <ul> <li>Displays the current state of the allocation table.</li> <li>Steps: <ul> <li>Iterates through the <code>allocation_table_buffer</code>.</li> <li>Prints the status of each block (e.g., <code>0</code> for free, <code>1</code> for allocated).</li> </ul> </li> </ul>
    


<h3>initMetadata(FILE* MS, Meta metabuffer[]):</h3> <ul> <li>Initializes the metadata blocks in the file system to prepare for storing file metadata.</li> <li>Steps: <ul> <li>Loops through all metadata entries, initializing each entry (inode) to default values using <code>initInode</code>.</li> <li>Writes the initialized metadata to the metadata region of the file system.</li> <li>Ensures all metadata blocks are cleared and ready for use.</li> </ul> </li> </ul> <h3>promptInode(FILE* MS, Meta* inode):</h3> <ul> <li>Collects input from the user to populate an inode structure with file metadata.</li> <li>Steps: <ul> <li>Prompts the user for: <ul> <li>File name (ensures it does not exceed <code>MAX_NAME_LENGTH</code>).</li> <li>Number of records in the file.</li> <li>Global organization mode (<code>0</code> for contiguous, <code>1</code> for chained).</li> <li>Internal organization mode (<code>0</code> for unsorted, <code>1</code> for sorted).</li> </ul> </li> <li>Calculates the required number of blocks for the file based on the number of records.</li> <li>Populates the provided <code>inode</code> structure with the gathered data.</li> </ul> </li> </ul> <h3>fillStartBlock(FILE* MS, Meta* inode):</h3> <ul> <li>Allocates the starting block(s) for a file based on its organization mode and size.</li> <li>Steps: <ul> <li>Validates constraints (e.g., free blocks, metadata availability) using <code>validateConstraints</code>.</li> <li>If storage or metadata constraints are violated, sets <code>start_block</code> to <code>-1</code>.</li> <li>If the global organization mode is: <ul> <li><b>Contiguous:</b> Attempts to find contiguous free blocks using <code>findFreeAdjacentBlock</code>.</li> <li><b>Chained:</b> Allocates blocks individually and sets up chaining between them.</li> </ul> </li> <li>Updates the allocation table to reflect the newly allocated blocks.</li> </ul> </li> </ul> <h3>displayInode(Meta inode):</h3> <ul> <li>Displays the metadata information for a file.</li> <li>Steps: <ul> <li>Prints details such as: <ul> <li>File name.</li> <li>Number of records and blocks.</li> <li>Global organization mode (e.g., contiguous or chained).</li> <li>Internal organization mode (e.g., sorted or unsorted).</li> <li>Index of the first block allocated to the file.</li> </ul> </li> </ul> </li> </ul> <h3>searchMetadata(FILE* MS, char name[]):</h3> <ul> <li>Searches for a file's metadata entry (inode) by its name.</li> <li>Steps: <ul> <li>Iterates through all metadata entries in the file system.</li> <li>Compares the file name in each inode with the provided <code>name</code>.</li> <li>If found: <ul> <li>Returns a <code>Position</code> struct containing the block index and index within the block.</li> </ul> </li> <li>If not found: <ul> <li>Sets the <code>file_not_found_flag</code> to <code>true</code>.</li> <li>Returns a <code>Position</code> with <code>block_index</code> and <code>index</code> set to <code>-1</code>.</li> </ul> </li> </ul> </li> </ul>





<h3>generateClient(int id):</h3> <ul> <li>Generates a random client record with the specified ID.</li> <li>Steps: <ul> <li>Assigns the provided <code>id</code> to the <code>ID</code> field.</li> <li>Generates a random name (e.g., "Client X") where <code>X</code> is a random number.</li> <li>Sets a random age between 20 and 60.</li> <li>Generates a random balance between 1000 and 5000 (rounded to two decimal places).</li> <li>Sets the logical deletion flag (<code>LD</code>) to <code>0</code> (active).</li> </ul> </li> <li>Returns the generated <code>Client</code> struct.</li> </ul> <h3>fillFileDataAndMeta(FILE* MS, Meta inode):</h3> <ul> <li>Populates the file system with file data and metadata based on the provided <code>inode</code>.</li> <li>Steps: <ul> <li>Searches for an empty metadata entry using <code>searchMetadata</code>.</li> <li>Writes the file’s metadata to the appropriate location in the metadata region of the file system.</li> <li>For file data: <ul> <li>If <b>Contiguous Mode:</b> <ul> <li>Allocates and fills the required number of contiguous blocks with randomly generated clients using <code>generateClient</code>.</li> <li>Sets the <code>next</code> pointer in each block to link them sequentially.</li> </ul> </li> <li>If <b>Chained Mode:</b> <ul> <li>Allocates blocks individually, updates the allocation table, and sets the <code>next</code> pointer for chaining.</li> <li>Fills each block with randomly generated clients.</li> </ul> </li> </ul> </li> </ul> </li> </ul> <h3>displayFileData(FILE* MS, Meta inode):</h3> <ul> <li>Displays the data of a file stored in the file system.</li> <li>Steps: <ul> <li>Starts at the <code>start_block</code> of the file as defined in the <code>inode</code>.</li> <li>Reads each block sequentially (or follows the <code>next</code> pointers in chained mode).</li> <li>Prints the details of each client record in the blocks (e.g., ID, name, age, balance).</li> <li>Stops when the file’s total records are displayed or the end of the chain is reached.</li> </ul> </li> </ul> <h3>createFile(FILE* MS):</h3> <ul> <li>Creates a new file in the file system with user-defined properties.</li> <li>Steps: <ul> <li>Prompts the user for file metadata using <code>promptInode</code>.</li> <li>Attempts to allocate blocks for the file using <code>fillStartBlock</code>.</li> <li>If allocation is successful: <ul> <li>Stores the metadata in the metadata region.</li> <li>Populates the file data and metadata using <code>fillFileDataAndMeta</code>.</li> <li>Displays the file’s metadata and data using <code>displayInode</code> and <code>displayFileData</code>.</li> </ul> </li> <li>If allocation fails: <ul> <li>Displays relevant error messages using <code>displayErrors</code>.</li> </ul> </li> </ul> </li> </ul> <h3>deleteFile(FILE* MS, char* filename):</h3> <ul> <li>Deletes a file and frees its associated metadata and blocks in the file system.</li> <li>Steps: <ul> <li>Searches for the file’s metadata using <code>searchMetadata</code>.</li> <li>If found: <ul> <li>Reads and clears the metadata entry, marking it as free.</li> <li>Iterates through the file’s blocks (contiguous or chained) and: <ul> <li>Marks each block as free in the allocation table.</li> <li>Resets the block’s data to its initial state using <code>initBlock</code>.</li> </ul> </li> <li>Updates the allocation table to reflect the freed blocks.</li> </ul> </li> <li>If not found: <ul> <li>Sets the <code>file_not_found_flag</code> to <code>true</code>.</li> </ul> </li> </ul> </li> </ul>





<h3>insertRecord(FILE* MS, char* filename):</h3> <ul> <li>Inserts a new record into an existing file in the file system.</li> <li>Steps: <ul> <li>Searches for the file's metadata using <code>searchMetadata</code>.</li> <li>If the file is found: <ul> <li>Reads the metadata and determines the file’s current block and record capacity.</li> <li>If a new block is needed: <ul> <li>Attempts to allocate a new block using the allocation table.</li> <li>If contiguous mode, checks for adjacent free blocks; if chained mode, allocates and links blocks dynamically.</li> </ul> </li> <li>Inserts the new record into the appropriate block (sorted or unsorted based on the file’s internal organization mode).</li> <li>Updates the allocation table and file’s metadata accordingly.</li> </ul> </li> <li>If the file is not found: <ul> <li>Sets the <code>file_not_found_flag</code> to <code>true</code> and exits.</li> </ul> </li> </ul> </li> </ul> <h3>renameFile(FILE* MS, char* old_filename, char* new_filename):</h3> <ul> <li>Renames an existing file in the file system.</li> <li>Steps: <ul> <li>Searches for the file's metadata using <code>searchMetadata</code> with the old file name.</li> <li>If the file is found: <ul> <li>Validates the length of the new file name to ensure it does not exceed <code>MAX_NAME_LENGTH</code>.</li> <li>Updates the file’s metadata with the new name.</li> <li>Writes the updated metadata back to the file system.</li> </ul> </li> <li>If the file is not found: <ul> <li>Sets the <code>file_not_found_flag</code> to <code>true</code> and exits.</li> </ul> </li> </ul> </li> </ul> <h3>searchRecord(FILE* MS, char* filename, int id):</h3> <ul> <li>Searches for a specific record in a file by its ID.</li> <li>Steps: <ul> <li>Searches for the file’s metadata using <code>searchMetadata</code>.</li> <li>If the file is found: <ul> <li>Reads the file’s blocks sequentially (or follows the <code>next</code> pointers in chained mode).</li> <li>If the file is unsorted: <ul> <li>Performs a linear search through all blocks and records.</li> </ul> </li> <li>If the file is sorted: <ul> <li>Collects all records and performs a binary search using <code>binarySearch</code>.</li> </ul> </li> <li>Returns a <code>Position</code> struct indicating the block and index if the record is found.</li> </ul> </li> <li>If the file or record is not found: <ul> <li>Sets the <code>file_not_found_flag</code> to <code>true</code> and returns an invalid position.</li> </ul> </li> </ul> </li> </ul> <h3>binarySearch(Client clients[], int left, int right, int val):</h3> <ul> <li>Performs a binary search on a sorted array of <code>Client</code> records to find a specific ID.</li> <li>Steps: <ul> <li>Calculates the middle index of the array.</li> <li>Compares the ID of the middle element with the target value: <ul> <li>If the middle ID matches, returns the index.</li> <li>If the middle ID is greater, repeats the search in the left subarray.</li> <li>If the middle ID is smaller, repeats the search in the right subarray.</li> </ul> </li> <li>Returns <code>-1</code> if the target ID is not found.</li> </ul> </li> </ul> <h3>displayMenu():</h3> <ul> <li>Displays the main menu for interacting with the file system.</li> <li>Steps: <ul> <li>Prints a list of available options, such as initializing the file system, creating or deleting files, and more.</li> <li>Prompts the user to enter their choice.</li> <li>Guides the program flow based on the user’s selection.</li> </ul> </li> </ul>












