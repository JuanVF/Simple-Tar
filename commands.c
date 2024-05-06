#include "commands.h"
#include "tar.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// handleCommands is the entry point to handle all commands
int handleCommands(int argumentCount, char *argumentList[]) {
  if (argumentCount <= 1) {
    displayHelp();

    return 0;
  }

  int flagCount;
  char *flags[argumentCount];

  getFlags(argumentCount, argumentList, &flagCount, flags);

  char *filename = NULL;

  Flags selectedMode = UNKNOWN;

  // run the check for long flags like --create, --update, etc.
  for (int i = 0; i < flagCount; i++) {
    Flags currentMode = UNKNOWN;

    if (isFlag(flags[i])) {
      continue;
    }

    currentMode = determineFlag(flags[i]);

    if (currentMode == VERBOSE) {
      isGlobalVerbosed = true;
    }

    if (currentMode == USE_FILE) {
      filename = getOutFilename(argumentCount, argumentList);

      if (filename == NULL)
        return 1;
    }

    if (currentMode == UNKNOWN) {
      char errorMessage[100];

      snprintf(errorMessage, 100, "unknown flag %s", flags[i]);

      logError(errorMessage);
      logInfo("run \"star --help\" to see the available flags");

      return 1;
    }

    if (currentMode != VERBOSE && currentMode != USE_FILE) {
      selectedMode = currentMode;
    }
  }

  // run the check for short flags, like -cvz
  for (int i = 0; i < flagCount; i++) {
    if (isLongFlag(flags[i])) {
      continue;
    }

    for (int k = 0; k < strlen(flags[i]); k++) {
      if (flags[i][k] == '-')
        continue;

      char buffer[2];

      buffer[0] = flags[i][k];

      Flags currentMode = determineFlag(buffer);

      if (currentMode == VERBOSE) {
        isGlobalVerbosed = true;
      }

      if (currentMode == USE_FILE) {
        filename = getOutFilename(argumentCount, argumentList);

        if (filename == NULL)
          return 1;
      }

      if (currentMode == UNKNOWN) {
        char errorMessage[100];

        snprintf(errorMessage, 100, "unknown flag %s", flags[i]);

        logError(errorMessage);
        logInfo("run \"star --help\" to see the available flags");

        return 1;
      }

      if (currentMode != VERBOSE && currentMode != USE_FILE) {
        selectedMode = currentMode;
      }
    }
  }

  int filesCount;
  char *files[argumentCount];

  getFiles(argumentCount, argumentList, &filesCount, files);

  return callCommands(selectedMode, files, filesCount, filename);
}

// callCommands decide which command to send
int callCommands(Flags command, char *files[], int fileCount, char *filename) {
  if (command == HELP) {
    return displayHelp();
  }
  if (command == EXTRACT) {
    return extract(files, fileCount, filename);
  }
  if (command == CREATE) {
    return create(files, fileCount, filename);
  }
  if (command == LIST) {
    return list(files, fileCount, filename);
  }
  if (command == DELETE) {
    return delete (files, fileCount, filename);
  }
  if (command == UPDATE) {
    return update(files, fileCount, filename);
  }
  if (command == APPEND) {
    return append(files, fileCount, filename);
  }
  if (command == PACK) {
    return pack(files, fileCount, filename);
  }

  return 0;
}

// determineFlag determines which long flag is using
Flags determineFlag(char *flag) {
  if (strcmp(flag, "--create") == 0 || flag[0] == 'c') {
    return CREATE;
  }

  if (strcmp(flag, "--extract") == 0 || flag[0] == 'x') {
    return EXTRACT;
  }

  if (strcmp(flag, "--list") == 0 || flag[0] == 't') {
    return LIST;
  }

  if (strcmp(flag, "--delete") == 0) {
    return DELETE;
  }

  if (strcmp(flag, "--update") == 0 || flag[0] == 'u') {
    return UPDATE;
  }

  if (strcmp(flag, "--verbose") == 0 || flag[0] == 'v') {
    return VERBOSE;
  }

  if (strcmp(flag, "--file") == 0 || flag[0] == 'f') {
    return USE_FILE;
  }

  if (strcmp(flag, "--append") == 0 || flag[0] == 'r') {
    return APPEND;
  }

  if (strcmp(flag, "--help") == 0 || flag[0] == 'h') {
    return HELP;
  }

  if (strcmp(flag, "--pack") == 0 || flag[0] == 'p') {
    return PACK;
  }

  return UNKNOWN;
}

// getOutFilename retrives the filename to be used
char *getOutFilename(int argumentCount, char *argumentList[]) {
  // Iterate over all arguments
  for (int i = 1; i < argumentCount; i++) {
    // If the argument ends with ".tar", return it
    if (endsWithTar(argumentList[i])) {
      return argumentList[i];
    }
  }

  // Log an error if no .tar file is found
  logError("No .tar file specified in arguments");
  return NULL;
}

// getFlags return all the flags that the user passed as parameter
void getFlags(int argumentCount, char *argumentList[], int *flagCount,
              char *flags[]) {
  *flagCount = 0;

  for (int i = 1; i < argumentCount; i++) {
    bool isValidFlag = isFlag(argumentList[i]) || isLongFlag(argumentList[i]);

    if (isValidFlag) {
      flags[*flagCount] = argumentList[i];
      (*flagCount)++;
    }
  }
}

// getFiles return all the files that are necessary for the command
void getFiles(int argumentCount, char *argumentList[], int *fileCount,
              char *files[]) {
  *fileCount = 0;

  int startIndex = 2;

  // just received "star" "<flag>" "<flag>"  "<files>..."
  if (argumentCount >= 4) {
    startIndex = 3;
  }

  for (int i = startIndex; i < argumentCount; i++) {
    bool isValidFlag = isFlag(argumentList[i]) || isLongFlag(argumentList[i]);

    if (!isValidFlag) {
      files[*fileCount] = argumentList[i];
      (*fileCount)++;
    }
  }
}

// isFlag determines if it is a single flag
bool isFlag(char *flag) { return flag[0] == '-' && flag[1] != '-'; }

// isLongFlag determines if it is a long flag
bool isLongFlag(char *flag) {
  return strlen(flag) > 1 && flag[0] == '-' && flag[1] == '-';
}

// Function to check if the argument ends with ".tar"
bool endsWithTar(const char *filename) {
  const char *tarSuffix = ".tar";
  size_t lenSuffix = strlen(tarSuffix);
  size_t lenFilename = strlen(filename);

  // Check if the filename is longer than the suffix and ends with ".tar"
  return lenFilename >= lenSuffix &&
         strcmp(filename + lenFilename - lenSuffix, tarSuffix) == 0;
}