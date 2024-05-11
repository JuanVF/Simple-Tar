#ifndef SO_TAR_H
#define SO_TAR_H

#include <stdio.h>

size_t octal_to_size_t(char *octal);
const char *get_filename(const char *path);

int displayHelp();
int create(char *files[], int fileCount, char *filename);
int extract(char *filename);
int list(char *filename);
int delete(char *files[], int fileCount, char *filename);
int update(char *files[], int fileCount, char *filename);
int append(char *files[], int fileCount, char *filename);
int pack(char *filename);

#endif