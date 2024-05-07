#include "tar.h"
#include "logs.h"

#include <stdio.h>
#include <stdlib.h>


#define BLOCK_SIZE (256 * 1024) // Tamaño de bloque en bytes (256 KB)

struct posix_header {
    char name[100];          /* Nombre del archivo */
    char mode[8];            /* Modo de archivo */
    char uid[8];             /* ID de usuario */
    char gid[8];             /* ID de grupo */
    char size[12];           /* Tamaño del archivo en bytes (octal) */
    char mtime[12];          /* Tiempo de modificación en formato numérico Unix (octal) */
    char checksum[8];        /* Suma de verificación para el encabezado */
    char typeflag[1];        /* Tipo de archivo */
    char linkname[100];      /* Nombre del enlace */
    /* Agregar más campos si es necesario */
    char pad[255];           /* Relleno para hacer el encabezado de 512 bytes */
};

// create will add files to a new tar file
int create(char *input_files[], int num_files, char *output_file) {
  if (filename == NULL) {
    logError("filename must be specified to create a tar file");
    return 1;
  }

  char message[100];
  snprintf(message, 100, "starting to create %s", filename);
  logVerbose(message);
  FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error al abrir el archivo de salida.\n");
        return;
    }

    // Iterar sobre los archivos de entrada y escribirlos en el archivo .tar
    for (int i = 0; i < num_files; i++) {
        FILE *input = fopen(input_files[i], "rb");
        if (!input) {
            printf("No se pudo abrir el archivo %s\n", input_files[i]);
            continue;
        }

        // Obtener el tamaño del archivo de entrada
        long file_size = 0;
        fseek(input, 0, SEEK_END);
        file_size = ftell(input);
        fseek(input, 0, SEEK_SET);

        printf("Archivo: %s, Tamaño: %ld bytes\n", input_files[i], file_size);

        // Crear el encabezado para el archivo
        struct posix_header header;
        memset(&header, 0, sizeof(header));
        strncpy(header.name, input_files[i], 100);
        snprintf(header.mode, 8, "%07o", 0644); // Modo de archivo: rw-r--r--
        snprintf(header.size, 12, "%011lo", (unsigned long)file_size); // Tamaño del archivo en octal
        snprintf(header.typeflag, 1, "%c", '0'); // Tipo de archivo: archivo normal

        // Escribir el encabezado en el archivo .tar
        fwrite(&header, sizeof(header), 1, output);

        // Escribir el contenido del archivo en bloques de datos
        char buffer[BLOCK_SIZE];
        size_t bytes_read;
        int num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // Calcular el número de bloques
        printf("Número de bloques: %d\n", num_blocks);
        for (int j = 0; j < num_blocks; j++) {
            bytes_read = fread(buffer, 1, BLOCK_SIZE, input);
            fwrite(buffer, 1, bytes_read, output);
        }

        fclose(input);
    }

    fclose(output);


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
// el files y fileCount deberia de quitarse
int list(char *files[], int fileCount, char *tar_file) {
  char message[100];
  snprintf(message, 100, "list of files in %s", filename);
  logVerbose(message);
  FILE *tar = fopen(tar_file, "rb");
    if (!tar) {
        printf("Error al abrir el archivo .tar\n");
        return;
    }

    // Leer el archivo .tar y mostrar los nombres de los archivos dentro de él
    struct posix_header header;
    while (fread(&header, sizeof(header), 1, tar) > 0) {
        if (header.name[0] == '\0') // Fin del archivo tar
            break;
        
        printf("%s\n", header.name);

        // Calcular el tamaño del archivo y saltar al siguiente encabezado
        long file_size = strtol(header.size, NULL, 8);
        int remaining_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        fseek(tar, remaining_blocks * BLOCK_SIZE, SEEK_CUR); // Saltar al siguiente encabezado
    }

    fclose(tar);
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