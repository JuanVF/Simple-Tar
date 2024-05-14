#include "tar.h"
#include "logs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HEADER_SIZE 1024 * 1024 * 2 // Header Size of 2MB
#define BLOCK_SIZE 1024 * 256           // 256 KB Block Size
#define MAX_FILES 10000

struct posix_file_info {
  char filename[176];
  char blockAddress[12];
  char size[12];
};

struct posix_header {
  struct posix_file_info files[sizeof(struct posix_file_info) *
                               10000]; // List of Files max of HEADER SIZE
};

struct block_data {
  char next[12];
  char isFree[12];
  char data[BLOCK_SIZE - 12 * 2];
};

/**
 * ------------------------------------------
 *          CREATE COMMAND
 * ------------------------------------------
 */

/**
 * @description: will add files to a new tar file
 * @parameter: (input_files) the input files to be added to a tar file
 * @parameter: (num_files) the amount of input files received
 * @parameter: (output_file) the tar file received
 * @output: the exit code error
 */
int create(char *input_files[], int num_files, char *output_file) {
  char message[100];

  FILE *output;

  if (num_files < 1) {
    logError("no files to add...");
    return 1;
  }

  if (num_files > MAX_FILES) {
    logVerbose(
        "star only supports up to 10k files. Since it has a 2MB FAT Table");
    num_files = MAX_FILES;
  }

  snprintf(message, 100, "output of tar file to stdout");
  if (output_file) {
    snprintf(message, 100, "starting to create %s", output_file);
    output = fopen(output_file, "wb");
  }
  logVerbose(message);

  // Creates the File Header
  struct posix_header *file_header = malloc(sizeof(struct posix_header));

  if (!file_header) {
    logError("memory allocation failed");
    return 1;
  }
  memset(file_header, 0, sizeof(struct posix_header));

  createHeader(file_header, num_files, input_files);

  fwrite(file_header, MAX_HEADER_SIZE, 1, output);

  // Now it will create the blocks for each file
  int result = createFATBlocks(file_header, output, num_files, input_files);

  free(file_header);
  fclose(output);

  return result;
}

/**
 * @description: creates the header for the tar file using FAT table
 * @parameter: (file_header) the header or FAT table to be set.
 * @parameter: (num_files) the amount of input files received
 * @parameter: (input_files) the files to be written.
 * @output: n/a
 */
void createHeader(struct posix_header *file_header, int num_files,
                  char *input_files[]) {
  char message[100];

  // pre-calc of the block to be placed
  int blocksCreated = 0;

  for (int i = 0; i < num_files; i++) {
    FILE *input = fopen(input_files[i], "rb");

    long file_size = 0;
    fseek(input, 0, SEEK_END);
    file_size = ftell(input);
    fseek(input, 0, SEEK_SET);

    const char *filename = get_filename(input_files[i]);

    int numBlocks = (file_size + BLOCK_SIZE - 12 - 1) / (BLOCK_SIZE - 12);

    struct posix_file_info file_info;

    snprintf(file_info.filename, sizeof(file_info.filename), "%s", filename);

    // size is stored in octal
    snprintf(file_info.size, 12, "%011lo", (unsigned long)file_size);
    snprintf(file_info.blockAddress, 12, "%011lo",
             (unsigned long)blocksCreated);

    snprintf(message, sizeof(message), "Adding file %s to header", filename);
    logVerbose(message);

    file_header->files[i] = file_info;

    blocksCreated += numBlocks;

    fclose(input);
  }
}

/**
 * @description: create the FAT Blocks for the tar file
 * @parameter: (file_header) the header or FAT table to be used.
 * @parameter: (output) the tar FILE to be written.
 * @parameter: (input_files) the files to be written.
 * @parameter: (num_files) the amount of input files received
 * @parameter: (input_files) the files to be written.
 * @output: n/a
 */
int createFATBlocks(struct posix_header *file_header, FILE *output,
                    int num_files, char *input_files[]) {
  char message[100];
  int blocksCreated = 0;

  for (int i = 0; i < num_files; i++) {
    struct posix_file_info fileInfo = file_header->files[i];

    int numBlocks = (octal_to_size_t(fileInfo.size) + BLOCK_SIZE - 12 - 1) /
                    (BLOCK_SIZE - 12);

    snprintf(message, sizeof(message),
             "num blocks [%d] for [%s] because of size [%d / %d]", numBlocks,
             fileInfo.filename, octal_to_size_t(fileInfo.size),
             BLOCK_SIZE - 12);
    logVerbose(message);

    FILE *inputFile = fopen(input_files[i], "rb");

    size_t bytes_to_read = BLOCK_SIZE - 12;

    for (int b = 0; b < numBlocks; b++) {
      struct block_data *block = malloc(sizeof(struct block_data));

      if (!block) {
        logError("Memory allocation for block failed");
        fclose(inputFile);
        free(file_header);
        fclose(output);
        return 1;
      }

      size_t read = fread(block->data, 1, BLOCK_SIZE - 12 * 2, inputFile);

      // Zero-fill the rest of the block
      if (read < BLOCK_SIZE - 12 * 2) {
        memset(block->data + read, 0, (BLOCK_SIZE - 12 * 2) - read);
      }

      int nextBlock = b + 1 < numBlocks ? blocksCreated + b + 1 : 0;

      snprintf(message, sizeof(message), "block #%d", blocksCreated + b);
      logVerbose(message);

      snprintf(block->next, sizeof(block->next), "%011lo",
               (unsigned long)nextBlock);
      snprintf(block->isFree, sizeof(block->isFree), "%011lo",
               (unsigned long)0);

      // Write block to output file
      fwrite(block, sizeof(struct block_data), 1, output);

      free(block);
    }

    blocksCreated += numBlocks;

    fclose(inputFile);
  }

  return 0;
}

/**
 * ------------------------------------------
 *          EXTRACT COMMAND
 * ------------------------------------------
 */

/**
 * @description: will extract the files in the current directory
 * @parameter: (filename) the file to be used to extract.
 * @output: the exit code.
 */
int extract(char *filename) {
  char message[100];
  FILE *archive = fopen(filename, "rb");
  if (!archive) {
    logError("Failed to open tar archive file. Double check if the input file "
             "exists.");
    return 1;
  }

  struct posix_header *header = malloc(sizeof(struct posix_header));

  if (!header) {
    logError("Memory allocation for header failed.");
    fclose(archive);
    return 1;
  }

  if (fread(header, MAX_HEADER_SIZE, 1, archive) != 1) {
    logError("Failed to read header.");
    free(header);
    fclose(archive);
    return 1;
  }

  extractFilesByTarFile(header, archive);

  free(header);
  fclose(archive);
  return 0;
}

/**
 * @description: extract all the files out of a tar file
 * @parameter: (header) the FAT header of the tar file
 * @parameter: (archive) the tar file to be read.
 * @output: n/a
 */
void extractFilesByTarFile(struct posix_header *header, FILE *archive) {
  char message[100];

  for (int i = 0; i < MAX_FILES && strlen(header->files[i].filename) > 0; i++) {
    struct posix_file_info fileInfo = header->files[i];

    extractFileByTarFile(archive, &fileInfo);
  }
}

/**
 * @description: extracts a single file from the tar file
 * @parameter: (header) the FAT header of the tar file
 * @parameter: (archive) the tar file to be read.
 * @parameter: (fileInfo) the info of the specific file to be extracted
 * @output: n/a
 */
void extractFileByTarFile(FILE *archive, struct posix_file_info *fileInfo) {

  char message[100];

  size_t fileSize = octal_to_size_t(fileInfo->size);
  size_t filePosition = octal_to_size_t(fileInfo->blockAddress);

  FILE *outputFile = fopen(fileInfo->filename, "wb");

  if (!outputFile) {
    snprintf(message, sizeof(message), "Failed to create file %s",
             fileInfo->filename);
    logError(message);
    return; // Continue with next files
  }

  snprintf(message, sizeof(message), "starting to create %s",
           fileInfo->filename);
  logVerbose(message);

  size_t currentBlockIndex = filePosition;
  size_t totalBytesWritten = 0;

  while (totalBytesWritten < fileSize) {
    snprintf(message, sizeof(message), "reading block #%d", currentBlockIndex);
    logVerbose(message);

    // Seek to the current block in the archive file
    fseek(archive, MAX_HEADER_SIZE + currentBlockIndex * BLOCK_SIZE, SEEK_SET);

    struct block_data block;

    if (fread(&block, BLOCK_SIZE, 1, archive) != 1) {
      logError("Failed to read block data.");
      break;
    }

    // Calculate the size of data to write to the output file
    size_t writeSize = BLOCK_SIZE - 12 * 2;
    size_t remainingSize = fileSize - totalBytesWritten;

    // Adjust write size for the last portion of data
    if (writeSize > remainingSize) {
      writeSize = remainingSize;
    }

    fwrite(block.data, 1, writeSize, outputFile);
    totalBytesWritten += writeSize;

    // Convert the next block index from octal to size_t
    size_t nextBlockIndex = octal_to_size_t(block.next);

    if (nextBlockIndex == 0) {
      break;
    }
    currentBlockIndex = nextBlockIndex;
  }

  fclose(outputFile);
}

/**
 * ------------------------------------------
 *          LIST COMMAND
 * ------------------------------------------
 */

/**
 * @description: will list all the filenames
 * @parameter: (filename) the tar filename to be listed
 * @output: the exit code
 */
int list(char *filename) { char message[100];
  FILE *archive = fopen(filename, "rb");
  if (!archive) {
    logError("Failed to open tar archive file. Double check if the input file "
             "exists.");
    return 1;
  }

  struct posix_header *header = malloc(sizeof(struct posix_header));

  if (!header) {
    logError("Memory allocation for header failed.");
    fclose(archive);
    return 1;
  }

  if (fread(header, MAX_HEADER_SIZE, 1, archive) != 1) {
    logError("Failed to read header.");
    free(header);
    fclose(archive);
    return 1;
  }

  listFilesByTarFile(header, archive);

  free(header);
  fclose(archive);
  return 0;
  }
  /**
 * @description: list all the files out of a tar file
 * @parameter: (header) the FAT header of the tar file
 * @parameter: (archive) the tar file to be read.
 * @output: n/a
 */
void listFilesByTarFile(struct posix_header *header, FILE *archive) {
  char message[100];

  for (int i = 0; i < MAX_FILES && strlen(header->files[i].filename) > 0; i++) {

    //struct posix_file_info fileInfo = header->files[i];
    snprintf(message, 100, "this is a file present: %s", header->files[i].filename);
    logVerbose(message);
    //extractFileByTarFile(archive, &fileInfo);
  }
}

/**
 * ------------------------------------------
 *          DELETE COMMAND
 * ------------------------------------------
 */

/**
 * @description: will remove certain files from the tar file
 * @parameter: (files) the files name to be deleted
 * @parameter: (fileCount) the amount of files to be deleted
 * @parameter: (filename) the tar filename to be listed
 * @output: the exit code
 */
int delete(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to delete archives inside %s", filename);
  logVerbose(message);
  FILE *archive = fopen(filename, "rb");
  if (!archive) {
    logError("Failed to open tar archive file. Double check if the input file "
             "exists.");
    return 1;
  }

  struct posix_header *header = malloc(sizeof(struct posix_header));

  if (!header) {
    logError("Memory allocation for header failed.");
    fclose(archive);
    return 1;
  }

  if (fread(header, MAX_HEADER_SIZE, 1, archive) != 1) {
    logError("Failed to read header.");
    free(header);
    fclose(archive);
    return 1;
  }

  deleteFilesByTarFile(header, archive);

  free(header);
  fclose(archive);

  return 0;
}

  /**
 * @description: list all the files out of a tar file
 * @parameter: (header) the FAT header of the tar file
 * @parameter: (archive) the tar file to be read.
 * @output: n/a
 */
void deleteFilesByTarFile(struct posix_header *header, FILE *archive) {
  char message[100];

  for (int i = 0; i < MAX_FILES && strlen(header->files[i].filename) > 0; i++) {

    struct posix_file_info fileInfo = header->files[i];
    /*snprintf(message, 100, "this is a file present: %s", header->files[i].filename);
    logVerbose(message);*/
    //extractFileByTarFile(archive, &fileInfo);
  }
}



/**
 * ------------------------------------------
 *          UPDATE COMMAND
 * ------------------------------------------
 */

/**
 * @description: will update the content of n archives
 * @parameter: (files) the files name to be deleted
 * @parameter: (fileCount) the amount of files to be deleted
 * @parameter: (filename) the tar filename to be listed
 * @output: the exit code
 */
int update(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "Starting to update archives inside %s", filename);
  logVerbose(message);

  FILE *archive = fopen(filename, "r+b");
  if (!archive) {
    logError("Failed to open tar archive file. Double check if the input file "
             "exists.");
    return 1;
  }

  struct posix_header *header = malloc(sizeof(struct posix_header));

  if (!header) {
    logError("Memory allocation for header failed.");
    fclose(archive);
    return 1;
  }

  if (fread(header, MAX_HEADER_SIZE, 1, archive) != 1) {
    logError("Failed to read header.");
    free(header);
    fclose(archive);
    return 1;
  }

  updateBlocksInFile(files, fileCount, header, archive);

  // Rewrite the header if any changes
  fseek(archive, 0, SEEK_SET);
  fwrite(header, MAX_HEADER_SIZE, 1, archive);

  free(header);
  fclose(archive);
  return 0;
}

/**
 * @description: will update the blocks with the new files information
 * @parameter: (files) the files name to be updated
 * @parameter: (fileCount) the amount of files to be deleted
 * @parameter: (header) the FAT Header to be used
 * @parameter: (archive) the tar FILE
 * @output: n/a
 */
void updateBlocksInFile(char *files[], int fileCount,
                        struct posix_header *header, FILE *archive) {
  char message[100];

  for (int i = 0; i < fileCount; i++) {
    FILE *inputFile = fopen(files[i], "rb");

    if (!inputFile) {
      snprintf(message, 100,
               "Error reading file %s, continuing with other files.", files[i]);
      logError(message);
      continue;
    }

    fseek(inputFile, 0, SEEK_END);
    size_t newFileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    size_t newNumBlocks =
        (newFileSize + BLOCK_SIZE - 24 - 1) / (BLOCK_SIZE - 24);

    int fileIndex;
    bool isFileInArchive = isFileInFATTable(header, files[i], &fileIndex);

    if (!isFileInArchive) {
      logError("file not in archive... continuing...");

      fclose(inputFile);

      continue;
    }

    size_t existingBlocks =
        (octal_to_size_t(header->files[fileIndex].size) + BLOCK_SIZE - 24 - 1) /
        (BLOCK_SIZE - 24);

    snprintf(message, 100,
             "file %s has %d blocks and will require now %d blocks.",
             header->files[fileIndex].filename, existingBlocks, newNumBlocks);
    logVerbose(message);

    size_t currentBlockIndex =
        octal_to_size_t(header->files[fileIndex].blockAddress);

    if (existingBlocks >= newNumBlocks) {
      // Update existing blocks
      size_t blockCount = 0;

      overwriteExistingBlocks(header->files[fileIndex].filename,
                              &currentBlockIndex, &blockCount, &newNumBlocks,
                              archive, inputFile);

      // If the file is smaller, mark remaining blocks as free
      if (existingBlocks > newNumBlocks) {
        markRemainingBlocksAsFree(&currentBlockIndex, archive);
      }
    } else {
      updateWhenFileSizeIsGreater(header->files[fileIndex].filename,
                                  existingBlocks, currentBlockIndex,
                                  newNumBlocks, archive, inputFile);

      // Update file info in the header
      size_t_to_octal(header->files[fileIndex].size, newFileSize);
    }

    fclose(inputFile);
  }
}

/**
 * @description: will determine if a certain file is present in the FAT table
 * @parameter: (header) the FAT Header to be used
 * @parameter: (path) the filename with its path to look for
 * @parameter: (indexPosition) the position where the file is. This will be set
 * in the function.
 * @output: true if the file exists in the header
 */
bool isFileInFATTable(struct posix_header *header, char *path,
                      int *indexPosition) {
  char message[100];
  (*indexPosition) = -1;

  for (int j = 0; j < MAX_FILES && strlen(header->files[j].filename) > 0; j++) {
    if (strcmp(header->files[j].filename, get_filename(path)) == 0) {

      (*indexPosition) = j;

      snprintf(message, 100, "file %s exists in header will continue to update",
               get_filename(path));
      logVerbose(message);
      return true;
    }
  }

  return false;
}

/**
 * @description: will overwrite the exiting blocks of a file inside the tar file
 * @parameter: (filename) the filename of the file to overwrite
 * @parameter: (currentBlockIndex) the block address where the file starts
 * @parameter: (blockCount) the amount of blocks used. This will be set in the
 * function
 * @parameter: (newNumBlocks) the new amount of blocks required.
 * @parameter: (archive) the tar FILE
 * @parameter: (inputFile) the new file to be packaged
 * @output: n/a
 */
void overwriteExistingBlocks(char *filename, size_t *currentBlockIndex,
                             size_t *blockCount, size_t *newNumBlocks,
                             FILE *archive, FILE *inputFile) {
  char message[100];

  while ((*blockCount) < (*newNumBlocks)) {
    snprintf(message, 100, "reading block #%d for file ", *currentBlockIndex,
             filename);
    logVerbose(message);

    fseek(archive, MAX_HEADER_SIZE + (*currentBlockIndex) * BLOCK_SIZE,
          SEEK_SET);
    struct block_data block;
    fread(&block, sizeof(struct block_data), 1, archive);

    fread(block.data, 1, BLOCK_SIZE - 24, inputFile);
    fseek(archive, MAX_HEADER_SIZE + (*currentBlockIndex) * BLOCK_SIZE,
          SEEK_SET);
    fwrite(&block, sizeof(struct block_data), 1, archive);

    size_t nextBlockIndex = octal_to_size_t(block.next);

    if (nextBlockIndex == 0 || ++(*blockCount) >= (*newNumBlocks)) {
      break;
    }

    (*currentBlockIndex) = nextBlockIndex;
  }
}

/**
 * @description: will set the rest of the blocks as free
 * @parameter: (currentBlockIndex) the block address where the file starts
 * @parameter: (archive) the tar FILE
 * @output: n/a
 */
void markRemainingBlocksAsFree(size_t *currentBlockIndex, FILE *archive) {
  struct block_data block;

  while ((*currentBlockIndex) != 0) {
    fseek(archive, MAX_HEADER_SIZE + (*currentBlockIndex) * BLOCK_SIZE,
          SEEK_SET);
    fread(&block, sizeof(struct block_data), 1, archive);
    size_t_to_octal(block.isFree, 1); // Mark block as free
    fwrite(&block, sizeof(struct block_data), 1,
           archive); // Update block

    (*currentBlockIndex) =
        octal_to_size_t(block.next); // Move to the next block
  }
}

/**
 * @description: will update the blocks when file size is bigger. This requires
 * to find free blocks or allocate new blocks in the file.
 * @parameter: (filename) the name of the file updating
 * @parameter: (existingBlocks) the amount of current existing blocks
 * @parameter: (currentBlockIndex) the current block position of the file
 * @parameter: (newNumBlocks) the new amount of blocks required
 * @parameter: (archive) the tar FILE
 * @parameter: (inputFile) the new FILE
 * @output: n/a
 */
void updateWhenFileSizeIsGreater(char *filename, size_t existingBlocks,
                                 size_t currentBlockIndex, size_t newNumBlocks,
                                 FILE *archive, FILE *inputFile) {
  char message[100];

  size_t blockCount = 0;
  size_t lastBlockIndex = 0;

  overwriteExistingBlocks(filename, &currentBlockIndex, &blockCount,
                          &newNumBlocks, archive, inputFile);

  snprintf(message, 100, "starting to add new blocks for file %s", filename);
  logVerbose(message);

  fseek(archive, 0, SEEK_END);

  // This is where new blocks will start
  size_t appendPosition = ftell(archive);

  size_t firstPosition = (appendPosition - (MAX_HEADER_SIZE)) / (BLOCK_SIZE);

  updateAtNewBlocks(blockCount, newNumBlocks, &appendPosition, inputFile,
                    archive, filename);

  appendPosition = ftell(archive);

  if (existingBlocks > 0) {
    linkUpdatedBlocks(lastBlockIndex, firstPosition, archive, filename);
  }
}

/**
 * @description: will add new blocks for the updated file
 * @parameter: (blockCount) the counter for blocks
 * @parameter: (newNumBlocks) the new amount of blocks required
 * @parameter: (appendPosition) the current block position. This will be set in
 * the function.
 * @parameter: (inputFile) the new FILE
 * @parameter: (archive) the tar FILE
 * @parameter: (filename) the name of the updated file
 * @output: n/a
 */
void updateAtNewBlocks(size_t blockCount, size_t newNumBlocks,
                       size_t *appendPosition, FILE *inputFile, FILE *archive,
                       char *filename) {
  char message[100];
  struct block_data newBlock;

  for (; blockCount < newNumBlocks; blockCount++) {
    size_t pos = ((*appendPosition) - (MAX_HEADER_SIZE)) / (BLOCK_SIZE);

    memset(&newBlock, 0, sizeof(newBlock));

    // Read file content into block
    size_t readSize = fread(newBlock.data, 1, BLOCK_SIZE - 24, inputFile);

    if (blockCount < newNumBlocks - 1) {
      size_t_to_octal(newBlock.next, pos + +1);
    } else {
      size_t_to_octal(newBlock.next, 0);
    }

    snprintf(message, 100,
             "new block for %s is at block #%zu and its next will be #%d",
             filename, pos, octal_to_size_t(newBlock.next));
    logVerbose(message);

    fwrite(&newBlock, BLOCK_SIZE, 1, archive);
    (*appendPosition) += BLOCK_SIZE;
  }
}

/**
 * @description: will link the old blocks with the new ones
 * @parameter: (lastBlockIndex) the last block position
 * @parameter: (firstPosition) the first position of the new block added
 * @parameter: (archive) the tar FILE
 * @parameter: (filename) the name of the updated file
 * @output: n/a
 */
void linkUpdatedBlocks(size_t lastBlockIndex, size_t firstPosition,
                       FILE *archive, char *filename) {
  char message[100];

  fseek(archive, MAX_HEADER_SIZE + lastBlockIndex * BLOCK_SIZE, SEEK_SET);

  struct block_data lastBlock;

  fread(&lastBlock, BLOCK_SIZE, 1, archive);

  snprintf(message, 100, "new next in %s at block #%zu will be %d", filename,
           lastBlockIndex, firstPosition);
  logVerbose(message);

  size_t_to_octal(lastBlock.next, firstPosition);
  fseek(archive, MAX_HEADER_SIZE + lastBlockIndex * BLOCK_SIZE, SEEK_SET);
  fwrite(&lastBlock, BLOCK_SIZE, 1, archive);
}

/**
 * ------------------------------------------
 *          APPEND COMMAND
 * ------------------------------------------
 */

/**
 * @description: will add n new archives to the tar file
 * @parameter: (files) the files name to be deleted
 * @parameter: (fileCount) the amount of files to be deleted
 * @parameter: (filename) the tar filename to be listed
 * @output: the exit code
 */
int append(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to add new archives inside %s", filename);
  logVerbose(message);

  return 0;
}

/**
 * ------------------------------------------
 *          PACK COMMAND
 * ------------------------------------------
 */

/**
 * @description: will desfragment the tar file to free unused space
 * @parameter: (filename) the tar filename to be listed
 * @output: the exit code
 */
int pack(char *filename) {
  char message[100];
  snprintf(message, 100, "starting to desfragment the tar file %s", filename);
  logVerbose(message);

  FILE *archive = fopen(filename, "r+b");

  if (archive == NULL) {
    snprintf(message, 100, "error opening the tar file. %s", filename);
    logError(message);
    return 1;
  }

  struct posix_header *header = malloc(sizeof(struct posix_header));

  if (!header) {
    logError("memory allocation for header failed.");
    fclose(archive);
    return 1;
  }

  if (fread(header, MAX_HEADER_SIZE, 1, archive) != 1) {
    logError("failed to read header.");
    free(header);
    fclose(archive);
    return 1;
  }

  // Check and remove free blocks at the end of the file
  if (removeFreeBlocksAtEnd(archive, header) != 0) {
    free(header);
    fclose(archive);
    return 1;
  }

  size_t endPos = ftell(archive);

  for (int i = 0; i < MAX_FILES && strlen(header->files[i].filename) > 0; i++) {
    snprintf(message, 100, "reading info of file %s",
             header->files[i].filename);
    logVerbose(message);

    // Determine the first free block and save its position
    long firstFreeBlockPosition = -1;
    long currentPos = MAX_HEADER_SIZE;

    while (currentPos < endPos) {
      struct block_data block;

      fseek(archive, currentPos, SEEK_SET);
      fread(&block, BLOCK_SIZE, 1, archive);

      if (octal_to_size_t(block.isFree) == 1) {
        firstFreeBlockPosition = currentPos;
        break;
      }

      currentPos += BLOCK_SIZE;
    }

    if (firstFreeBlockPosition == -1) {
      logVerbose("no more free blocks available.");
      break;
    }

    snprintf(message, 100, "found a free block at %d", firstFreeBlockPosition);
    logVerbose(message);

    // Save the previous blockAddress
    size_t previousBlockAddress =
        octal_to_size_t(header->files[i].blockAddress);

    // Set the header's blockAddress to the first free block
    size_t_to_octal(header->files[i].blockAddress, firstFreeBlockPosition);

    snprintf(message, 100, "updated block address for %s from %s to %ld",
             header->files[i].filename, previousBlockAddress,
             firstFreeBlockPosition);
    logVerbose(message);

    size_t targetBlockAddress = firstFreeBlockPosition;

    while (previousBlockAddress != 0) {
      struct block_data block;

      fseek(archive, previousBlockAddress, SEEK_SET);
      fread(&block, BLOCK_SIZE, 1, archive);

      // Move block to the target address
      size_t nextBlockAddress = octal_to_size_t(block.next);

      size_t_to_octal(block.next, targetBlockAddress / (BLOCK_SIZE));

      fseek(archive, targetBlockAddress, SEEK_SET);
      fwrite(&block, BLOCK_SIZE, 1, archive);

      // Log the move
      snprintf(message, 100, "moved block from %ld to %ld",
               previousBlockAddress / (BLOCK_SIZE),
               targetBlockAddress / (BLOCK_SIZE));

      logVerbose(message);

      // Prepare for the next iteration
      previousBlockAddress = nextBlockAddress;
      targetBlockAddress += BLOCK_SIZE;
    }

    // Mark the last block of this file as free and update in the archive
    struct block_data block;

    memset(&block, 0, sizeof(block));
    size_t_to_octal(block.isFree, 1);

    fseek(archive, targetBlockAddress - BLOCK_SIZE, SEEK_SET);
    fwrite(&block, BLOCK_SIZE, 1, archive);

    // Log final block update
    snprintf(message, 100, "last block at %ld marked as free",
             targetBlockAddress - sizeof(struct block_data));
    logVerbose(message);

    // Update header's block address to new starting position
    size_t_to_octal(header->files[i].blockAddress, firstFreeBlockPosition);
  }

  // Check and remove free blocks at the end of the file
  if (removeFreeBlocksAtEnd(archive, header) != 0) {
    free(header);
    fclose(archive);
    return 1;
  }

  // rewrite the header with new directions
  fseek(archive, 0, SEEK_SET);
  fwrite(header, MAX_HEADER_SIZE, 1, archive);

  free(header);
  fclose(archive);

  logVerbose("file desfragmented successfully");

  return 0;
}

/**
 * @description: will remove unused blocks at the end of file
 * @parameter: (archive) the tar FILE
 * @parameter: (header) the tar FAT header
 * @output: the exit code
 */
int removeFreeBlocksAtEnd(FILE *archive, struct posix_header *header) {
  char message[100];

  // Seek to the end of the file to find the last block
  fseek(archive, 0, SEEK_END);
  long end_pos = ftell(archive);

  long current_pos = end_pos - BLOCK_SIZE;
  int counter = 0;

  while (current_pos >= MAX_HEADER_SIZE) {
    struct block_data block;

    fseek(archive, current_pos, SEEK_SET);
    fread(&block, BLOCK_SIZE, 1, archive);

    if (octal_to_size_t(block.isFree) == 0) {
      break;
    }

    snprintf(message, 100, "block #%d will be removed",
             current_pos / (BLOCK_SIZE));
    logVerbose(message);

    current_pos -= BLOCK_SIZE;
    counter++;
  }
  snprintf(message, 100, "found %d free blocks at the end", counter);
  logVerbose(message);

  if (counter > 0) {

    if (ftruncate(fileno(archive), current_pos) != 0) {
      return -1;
    }
  }

  return 0;
}

/**
 * ------------------------------------------
 *          HELP COMMAND
 * ------------------------------------------
 */

/**
 * @description: will print useful information for each command of this program
 * @output: the exit code
 */
int displayHelp() {
  // To prevent bad memory practices we need to free all these variables

  // Usage Text
  char *textUsageOption = applyColor("OPTION", ANSI_GREEN);
  char *textUsageFile = applyColor("FILE", ANSI_GREEN);

  // Flags Text
  char *textExampleCreateFilesFlags = applyColor("-cvf", ANSI_GREEN);
  char *textExampleExtractFilesFlags = applyColor("-xvf", ANSI_GREEN);
  char *textExampleDeleteFilesFlags = applyColor("--delete -vf", ANSI_GREEN);
  char *textExampleAppendFilesFlags = applyColor("-rvf", ANSI_GREEN);

  printf("%s_____________________________________________________________\n",
         AnsiColorStrings[ANSI_GREEN]);
  printf("   _____ _                 _        _______       _____  \n");
  printf("  / ____(_)               | |      |__   __|/\\   |  __ \\ \n");
  printf(" | (___  _ _ __ ___  _ __ | | ___     | |  /  \\  | |__) |\n");
  printf("  \\___ \\| | '_ ` _ \\| '_ \\| |/ _ \\    | | / /\\ \\ |  _  / \n");
  printf("  ____) | | | | | | | |_) | |  __/    | |/ ____ \\| | \\ \\ \n");
  printf(" |_____/|_|_| |_| |_| .__/|_|\\___|    |_/_/    \\_\\_|  \\_\n");
  printf("                    | |                                  \n");
  printf("                    |_|                                  \n");
  printf("_____________________________________________________________%s\n",
         AnsiColorStrings[ANSI_RESET]);
  printf("\nUsage: star [%s...] [%s...]\n", textUsageOption, textUsageFile);
  printf("\nExamples:\n");
  printf("\tstar %s html-paq.tar index.html\n", textExampleCreateFilesFlags);
  printf("\tstar %s xxx.tar\n", textExampleExtractFilesFlags);
  printf("\tstar %s foo.tar doc1.txt doc2.txt data.dat\n",
         textExampleCreateFilesFlags);
  printf("\tstar %s foo.tar data.dat\n", textExampleDeleteFilesFlags);
  printf("\tstar %s foo.tar test.doc\n", textExampleAppendFilesFlags);
  printf("\nMain operation mode:\n");
  printf("\t-h, --help: display this help menu\n");
  printf("\t-c, --create : create a new archive\n");
  printf("\t-x, --extract : extract from an archive\n");
  printf("\t-t, --list: list the contents of an archive\n");
  printf("\t--delete: delete from an archive\n");
  printf("\t-u, --update: update the contents of an archive\n");
  printf("\t-v, --verbose: display a verbose progress report\n");
  printf("\t-f, --file: archive contents from/to a file, if not present "
         "assumes standard input\n");
  printf("\t-r, --append: append contents to an archive\n");
  printf(
      "\t-p, --pack: pack the contents of an archive (not present in tar)\n");

  // free the memory
  free(textUsageOption);
  free(textUsageFile);

  free(textExampleCreateFilesFlags);
  free(textExampleExtractFilesFlags);
  free(textExampleDeleteFilesFlags);
  free(textExampleAppendFilesFlags);

  return 0;
}

/**
 * ------------------------------------------
 *          UTILITIES FUNCTIONS
 * ------------------------------------------
 */

/**
 * @description: out of a string octal it will return the number
 * @parameter: (octal) octal number in string format
 * @output: octal in number
 */
size_t octal_to_size_t(char *octal) {
  size_t size = 0;
  sscanf(octal, "%zo", &size);
  return size;
}

/**
 * @description: removes the path directory out of a path to get the filename
 * @parameter: (path) the full path
 * @output: the filename
 */
const char *get_filename(const char *path) {
  const char *last_slash = strrchr(path, '/');
  if (last_slash) {
    return last_slash + 1; // move past the '/' character
  }
  return path; // no '/' found, return the original path
}

/**
 * @description: creates an octal string out of a number
 * @parameter: (buffer) the string to be set
 * @parameter: (value) the numeric value
 * @output: n/a
 */
void size_t_to_octal(char *buffer, size_t value) {
  snprintf(buffer, 12, "%011lo", value);
}