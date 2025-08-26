#include "peon.h"
#include <dirent.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
  SRC,
  INCLUDE,
  TEST,
  ROOT,
} DirType;

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

int isModule(char *path, DirType *dirType) {
  if (hasDir(path, "src")) {
    *dirType = SRC;
    return 1;
  }
  if (hasDir(path, "include")) {
    *dirType = INCLUDE;
    return 1;
  }
  return 0;
}

void walk(char *path, PeonTarget *targets, int *targetCount, DirType *dirType) {
  if (isModule(path, dirType)) {
    targets[*targetCount].path = strdup(path);
    *targetCount = *targetCount + 1;
    return;
  }
  DIR *dir = opendir(path);
  if (!dir) {
    return;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (isSkip(entry)) {
      continue;
    }
    char *filename = entry->d_name;
    int pathLen = strlen(path);
    strcat(path, "/");
    strcat(path, filename);
    walk(path, targets, targetCount, dirType);
    path[pathLen] = 0;
  }
  closedir(dir);
}

void parseRoot(char *root, PeonTarget *targets, int *targetCount) {
  char pathBuff[PATH_MAX];
  memset(pathBuff, 0, sizeof(char) * PATH_MAX);
  strcat(pathBuff, root);
  DirType dirType = ROOT;
  walk(pathBuff, targets, targetCount, &dirType);
}
