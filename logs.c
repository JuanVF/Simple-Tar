#include "logs.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool isGlobalVerbosed = false;

// logInfo prints information
void logInfo(char *message) {
  char *infoText = applyColor("info:", ANSI_BLUE);

  printf("%s %s\n", infoText, message);

  free(infoText);
}

// logError prints errors
void logError(char *message) {
  char *errorText = applyColor("error:", ANSI_RED);

  printf("%s %s\n", errorText, message);

  free(errorText);
}

// logVerbose prints the verbose information
void logVerbose(char *message) {
  if (isGlobalVerbosed) {
    char *verboseText = applyColor("verbose:", ANSI_GREEN);

    printf("%s %s\n", verboseText, message);

    free(verboseText);
  }
}

// logVerbose prints the verbose information
void logWarning(char *message) {
  char *warningText = applyColor("warning:", ANSI_YELLOW);

  printf("%s %s\n", warningText, message);

  free(warningText);
}

// applyColor is a wrapper to add colors for a certain string
// this uses malloc, make sure to free the memory!
char *applyColor(const char *string, AnsiColor color) {
  size_t len = strlen(string);
  size_t escape_len = strlen(AnsiColorStrings[color]);
  size_t reset_len = strlen(AnsiColorStrings[ANSI_RESET]);

  char *coloredString = (char *)malloc(len + escape_len + reset_len +
                                       1); // +1 for null terminator
  if (coloredString == NULL) {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    exit(EXIT_FAILURE);
  }

  sprintf(coloredString, "%s%s%s", AnsiColorStrings[color], string,
          AnsiColorStrings[ANSI_RESET]);
  return coloredString;
}