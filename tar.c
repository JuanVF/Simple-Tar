#include "tar.h"
#include "logs.h"

#include <stdio.h>
#include <stdlib.h>

// create will add files to a new tar file
int create(char *files[], int fileCount, char *filename) {
  if (filename == NULL) {
    logError("filename must be specified to create a tar file");
    return 1;
  }

  char message[100];
  snprintf(message, 100, "starting to create %s", filename);
  logVerbose(message);

  return 0;
}

// extract will extract the files in the current directory
int extract(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to extract %s", filename);
  logVerbose(message);

  return 0;
}

// list will list all the contents from an archive
int list(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "list of files in %s", filename);
  logVerbose(message);

  return 0;
}

// delete will remove certain files from the tar file
int delete(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to delete archives inside %s", filename);
  logVerbose(message);

  return 0;
}

// update will updates the content of an archive
int update(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to update archives inside %s", filename);
  logVerbose(message);

  return 0;
}

// append will add new archives
int append(char *files[], int fileCount, char *filename) {
  char message[100];
  snprintf(message, 100, "starting to add new archives inside %s", filename);
  logVerbose(message);

  return 0;
}

// pack desfragment using FAT
int pack(char *files[], int fileCount, char *filename) {
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