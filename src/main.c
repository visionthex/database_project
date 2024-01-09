#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "parse.h"
#include "common.h"
#include "file.h"

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <database file>\n", argv[0]);
  printf("\t -n - Create a new database file\n");
  printf("\t -f - [required] Path to database file\n");
  return;
}

int main(int argc, char *argv[]) {

  char *filepath = NULL;
  char *addstring = NULL;
  bool newfile = false;
  bool list = false;
  int c;

  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
    switch (c) {
      case 'n':
        newfile = true;
        break;
      case 'f':
        filepath = optarg;
        break;
      case 'a':
        addstring = optarg;
        break;
      case 'l':
        list = true;
        break;
      case '?' :
        printf("Unknown option: %c\n", c);
        break;
      default:
        return -1;
    }
  }

  if (filepath == NULL) {
    printf("Filepath is a required argument\n");
    print_usage(argv);
    return 0;
  }

  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file\n");
      return -1;
    }

    if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to create database header\n");
      return -1;
    }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return -1;
    }

    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return -1;
    }

  }

  if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
    printf("Failed to read employees\n");
    free(dbhdr);
    return 0;
  }

  if (addstring) {
    dbhdr->count++;
    struct employee_t *temp = realloc(employees, dbhdr->count*(sizeof(struct employee_t)));
    if (temp == NULL) {
      printf("Failed to allocate memory\n");
      free(employees);
      free(dbhdr);
      return -1;
    }
    employees = temp;
    add_employee(dbhdr, employees, addstring);
  }

  if (list) {
    read_employees(dbfd, dbhdr, &employees);
  }

  printf("newfile: %s\n", newfile ? "true" : "false");
  printf("filepath: %s\n", filepath);

  output_file(dbfd, dbhdr, employees);

  free(employees);
  free(dbhdr);

  return 0;
}