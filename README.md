# Star Utility

## Introduction

Star (Simple Star) is a command-line utility designed to perform various operations on archives. It provides functionality to create, extract, list, delete, update, and pack archive files. Additionally, it supports verbose progress reporting and appending contents to an archive.

## Usage

To use Star, follow the instructions below:

### Building

Before using Star, you need to build it. Use the provided Makefile to compile the source code. Run the following command:

```bash
make build
```

This command will compile the source code and generate an executable named `star` in the `bin` directory.

### Command Syntax

The general syntax for running Star is:

```bash
star [OPTIONS] [FILES...]
```

Here, `OPTIONS` represent the various flags that control the operation of Star, and `FILES` are the files to be processed.

### Examples

Here are some examples of using Star:

- Create a new archive:

  ```bash
  star -cvf archive.tar file1.txt file2.txt
  ```

- Extract files from an archive:

  ```bash
  star -xvf archive.tar
  ```

- List the contents of an archive:

  ```bash
  star -tvf archive.tar
  ```

- Delete files from an archive:

  ```bash
  star --delete -vf archive.tar file1.txt
  ```

- Update the contents of an archive:

  ```bash
  star -uvf archive.tar file1.txt file2.txt
  ```

- Display a verbose progress report:

  ```bash
  star -v -cf archive.tar file1.txt file2.txt
  ```

- Append contents to an archive:

  ```bash
  star -rvf archive.tar file3.txt file4.txt
  ```

- Pack the contents of an archive (not present in tar):
  ```bash
  star -pvf archive.tar
  ```

For more information on available options, you can use the `-h` or `--help` flag:

```bash
star --help
```
