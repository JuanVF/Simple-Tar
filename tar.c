#include "tar.h"
#include "logs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEADER_SIZE 1000 * 1000 * 2 // Header Size of 2MB
#define BLOCK_SIZE 1000                 // 1 KB Block Size
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
  char data[BLOCK_SIZE - 12];
};

// create will add files to a new tar file
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

  fwrite(file_header, MAX_HEADER_SIZE, 1, output);

  // Now it will create the blocks for each file
  blocksCreated = 0;

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

      size_t read = fread(block->data, 1, BLOCK_SIZE - 12, inputFile);

      // Zero-fill the rest of the block
      if (read < BLOCK_SIZE - 12) {
        memset(block->data + read, 0, (BLOCK_SIZE - 12) - read);
      }

      int nextBlock = b + 1 < numBlocks ? blocksCreated + b + 1 : 0;

      snprintf(message, sizeof(message), "block #%d", blocksCreated + b);
      logVerbose(message);

      snprintf(block->next, sizeof(block->next), "%011lo",
               (unsigned long)nextBlock);

      // Write block to output file
      fwrite(block, sizeof(struct block_data), 1, output);

      free(block);
    }

    blocksCreated += numBlocks;

    fclose(inputFile);
  }

  free(file_header);
  fclose(output);

  return 0;
}

// extract will extract the files in the current directory
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

  for (int i = 0; i < MAX_FILES && strlen(header->files[i].filename) > 0; i++) {
    struct posix_file_info fileInfo = header->files[i];

    size_t fileSize = octal_to_size_t(fileInfo.size);
    size_t filePosition = octal_to_size_t(fileInfo.blockAddress);

    FILE *outputFile = fopen(fileInfo.filename, "wb");
    if (!outputFile) {
      snprintf(message, sizeof(message), "Failed to create file %s",
               fileInfo.filename);
      logError(message);
      continue; // Continue with next files
    }

    size_t currentBlockIndex = filePosition;
    size_t totalBytesWritten = 0;

    while (totalBytesWritten < fileSize) {
      // Seek to the current block in the archive file
      fseek(archive, MAX_HEADER_SIZE + currentBlockIndex * BLOCK_SIZE,
            SEEK_SET);

      struct block_data block;

      if (fread(&block, BLOCK_SIZE, 1, archive) != 1) {
        logError("Failed to read block data.");
        break;
      }

      // Calculate the size of data to write to the output file
      size_t writeSize = BLOCK_SIZE - 12;
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

  free(header);
  fclose(archive);
  return 0;
}

// list will list all the filenames
int list(char *filename) { return 0; }

// delete will remove certain files from the tar file
int delete(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to delete archives inside %s", filename);
  logVerbose(message);

  return 0;
}

// update will updates the content of an archive
int update(char *files[], int fileCount, char *filename) { return 0; }

// append will add new archives
int append(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to add new archives inside %s", filename);
  logVerbose(message);

  return 0;
}

// pack desfragment using FAT
int pack(char *filename) {
  char message[100];
  snprintf(message, 100, "starting to desfragment %s", filename);
  logVerbose(message);
  return 0;
}

// displayHelp will display all the required help
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

size_t octal_to_size_t(char *octal) {
  size_t size = 0;
  sscanf(octal, "%zo", &size);
  return size;
}

const char *get_filename(const char *path) {
  const char *last_slash = strrchr(path, '/');
  if (last_slash) {
    return last_slash + 1; // move past the '/' character
  }
  return path; // no '/' found, return the original path
}