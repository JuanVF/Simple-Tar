#ifndef LOGS_H
#define LOGS_H

#include <stdbool.h>

typedef enum {
  ANSI_RESET = 0,
  ANSI_BLACK,
  ANSI_RED,
  ANSI_GREEN,
  ANSI_YELLOW,
  ANSI_BLUE,
  ANSI_MAGENTA,
  ANSI_CYAN,
  ANSI_WHITE
} AnsiColor;

static const char *AnsiColorStrings[] = {
    "\033[0m",  // ANSI_RESET
    "\033[30m", // ANSI_BLACK
    "\033[31m", // ANSI_RED
    "\033[32m", // ANSI_GREEN
    "\033[33m", // ANSI_YELLOW
    "\033[34m", // ANSI_BLUE
    "\033[35m", // ANSI_MAGENTA
    "\033[36m", // ANSI_CYAN
    "\033[37m"  // ANSI_WHITE
};

extern bool isGlobalVerbosed;

void logInfo(char *message);
void logError(char *message);
void logVerbose(char *message);
void logWarning(char *message);

char *applyColor(const char *string, AnsiColor color);

#endif