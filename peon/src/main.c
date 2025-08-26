#include "peon.h"
#include "slice.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int isSkip(struct dirent *e) {
  if (strcmp(e->d_name, ".") == 0) {
    return 1;
  }
  if (strcmp(e->d_name, "..") == 0) {
    return 1;
  }
  if (strcmp(e->d_name, ".git") == 0) {
    return 1;
  }
  if (strcmp(e->d_name, ".cache") == 0) {
    return 1;
  }
  return 0;
}

int hasDir(char *basePath, char *dir) {
  char check[256];
  strcat(check, basePath);
  strcat(check, "/");
  strcat(check, dir);
  if (access(check, F_OK) != -1) {
    return 1;
  }
  return 0;
}

int isModule(char *path) {
  if (hasDir(path, "src")) {
    return 1;
  }
  if (hasDir(path, "include")) {
    return 1;
  }
  return 0;
}

void walk(char *path, Slice *modules) {
  struct stat sb;
  if (stat(path, &sb) != 0) {
    printf("error: %s, %s\n", path, strerror(errno));
    return;
  }
  if (!S_ISDIR(sb.st_mode)) {
    return;
  }
  if (isModule(path)) {
    char *p = strdup(path);
    sliceAppend(modules, p);
    return;
  }
  DIR *dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "Unable to open dir: %s, %s\n", path, strerror(errno));
    return;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (isSkip(entry)) {
      continue;
    }
    char *filename = entry->d_name;
    int newPathLen = strlen(path) + strlen(filename) + 10;
    char *newPath = calloc(newPathLen, sizeof(char));
    strcat(newPath, path);
    strcat(newPath, "/");
    strcat(newPath, filename);
    walk(newPath, modules);
    free(newPath);
  }
  closedir(dir);
}

void printSlice(Slice *s) {
  printf("Detected %d modules:\n", sliceLen(s));
  for (int i = 0; i < sliceLen(s); i++) {
    printf("%s\n", (char *)sliceGet(s, i));
  }
}

void clearSlice(Slice *s) {
  for (int i = 0; i < sliceLen(s); i++) {
    free(sliceGet(s, i));
  }
}

int main(int argc, char **argv) {
  if (argc <= 1 || argc >= 3) {
    printf("Usage: %s dir\n", argv[0]);
  }
  char *path = argv[1];
  Slice *modules = sliceMake();
  walk(path, modules);
  printSlice(modules);
  clearSlice(modules);
  sliceFree(modules);
  return 0;
}
