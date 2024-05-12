#ifndef COMMANDS_H
#define COMMANDS_H

#include "logs.h"

#include <stdbool.h>

typedef enum {
  CREATE = 0,
  EXTRACT,
  LIST,
  DELETE,
  UPDATE,
  VERBOSE,
  USE_FILE,
  APPEND,
  PACK,
  HELP,
  UNKNOWN
} Flags;

int handleCommands(int argumentCount, char *argumentList[]);

int callCommands(Flags command, char *files[], int fileCount, char *filename);

Flags getFromSimpleFlag(char *flag);
Flags determineFlag(char *flag);
void getFlags(int argumentCount, char *argumentList[], int *flagCount,
              char *flags[]);
void getFiles(int argumentCount, char *argumentList[], int *fileCount,
              char *files[]);
char *getOutFilename(int argumentCount, char *argumentList[]);
char *applyColor(const char *string, AnsiColor color);
bool isFlag(char *flag);
bool isLongFlag(char *flag);
bool endsWithTar(const char *filename);

#endif