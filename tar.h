#ifndef SO_TAR_H
#define SO_TAR_H

int displayHelp();
int create(char *files[], int fileCount, char *filename);
int extract(char *files[], int fileCount, char *filename);
int list(char *files[], int fileCount, char *filename);
int delete(char *files[], int fileCount, char *filename);
int update(char *files[], int fileCount, char *filename);
int append(char *files[], int fileCount, char *filename);
int pack(char *files[], int fileCount, char *filename);

#endif