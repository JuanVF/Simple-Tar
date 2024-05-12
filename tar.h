#ifndef SO_TAR_H
#define SO_TAR_H

#include <stdbool.h>
#include <stdio.h>

struct posix_header;
struct posix_file_info;

// Command Functions
int displayHelp();
int create(char *files[], int fileCount, char *filename);
int extract(char *filename);
int list(char *filename);
int delete(char *files[], int fileCount, char *filename);
int update(char *files[], int fileCount, char *filename);
int append(char *files[], int fileCount, char *filename);
int pack(char *filename);

// Utility functions

// octal string to number
size_t octal_to_size_t(char *octal);

// removes the path out of a string to get the filename
const char *get_filename(const char *path);

// number to octal string
void size_t_to_octal(char *buffer, size_t value);

// creates the header using FAT standard
void createHeader(struct posix_header *file_header, int num_files,
                  char *input_files[]);

// create FAT Cluster blocks in a file
int createFATBlocks(struct posix_header *file_header, FILE *output,
                    int num_files, char *input_files[]);

// extract files out of a tar file
void extractFilesByTarFile(struct posix_header *header, FILE *archive);

// extract a single file out of a tar file
void extractFileByTarFile(FILE *archive, struct posix_file_info *fileInfo);

// updates the block in the tar file
void updateBlocksInFile(char *files[], int fileCount,
                        struct posix_header *header, FILE *archive);

// will determine if a certain file is present in the FAT table
bool isFileInFATTable(struct posix_header *header, char *path,
                      int *indexPosition);

// will overwrite the existing blocks
void overwriteExistingBlocks(char *filename, size_t *currentBlockIndex,
                             size_t *blockCount, size_t *newNumBlocks,
                             FILE *archive, FILE *inputFile);

// will set the rest of the blocks as free
void markRemainingBlocksAsFree(size_t *currentBlockIndex, FILE *archive);

// will update the blocks when the size of the file is bigger
void updateWhenFileSizeIsGreater(char *filename, size_t existingBlocks,
                                 size_t currentBlockIndex, size_t newNumBlocks,
                                 FILE *archive, FILE *inputFile);

// will add new blocks for the updated file
void updateAtNewBlocks(size_t blockCount, size_t newNumBlocks,
                       size_t *appendPosition, FILE *inputFile, FILE *archive,
                       char *filename);

// will link the old blocks with the new ones
void linkUpdatedBlocks(size_t lastBlockIndex, size_t firstPosition,
                       FILE *archive, char *filename);

// will go to the end of file and remove last unused blocks
int removeFreeBlocksAtEnd(FILE *archive, struct posix_header *header);
#endif