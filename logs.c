#include "logs.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool isGlobalVerbosed = false;

/**
 * @description: logs information
 * @parameter: (message) the information to be logged
 * @output: n/a
 */
void logInfo(char *message) {
  char *infoText = applyColor("info:", ANSI_BLUE);

  printf("%s %s\n", infoText, message);

  free(infoText);
}

/**
 * @description: logs errors
 * @parameter: (message) the error to be logged
 * @output: n/a
 */
void logError(char *message) {
  char *errorText = applyColor("error:", ANSI_RED);

  printf("%s %s\n", errorText, message);

  free(errorText);
}

/**
 * @description: logs verbose information. Only logs if the verbose mode is set.
 * This can be set by using the -v flag in the program
 * @parameter: (message) the verbose message to be logged
 * @output: n/a
 */
void logVerbose(char *message) {
  if (isGlobalVerbosed) {
    char *verboseText = applyColor("verbose:", ANSI_GREEN);

    printf("%s %s\n", verboseText, message);

    free(verboseText);
  }
}

/**
 * @description: logs warning information.
 * @parameter: (message) the warning message to be logged
 * @output: n/a
 */
void logWarning(char *message) {
  char *warningText = applyColor("warning:", ANSI_YELLOW);

  printf("%s %s\n", warningText, message);

  free(warningText);
}

/**
 * @description: is a wrapper to add colors for a certain string. This uses
 * malloc, make sure to free the memory!
 * @parameter: (string) the string to be wrapped
 * @parameter: (color) the ansi color to be used to color the string
 * @output: the wrapped string with color. If there is an error in malloc it
 * will exit the program.
 */
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