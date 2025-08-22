#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int isSpecial(struct dirent *e) {
  if (strcmp(e->d_name, ".") == 0) {
    return 1;
  }
  if (strcmp(e->d_name, "..") == 0) {
    return 1;
  }
  return 0;
}

void walk(char *path, int depth) {
  printf("%s\n", path);
  struct stat sb;
  if (stat(path, &sb) != 0) {
    printf("error: %s, %s\n", path, strerror(errno));
    return;
  }
  if (!S_ISDIR(sb.st_mode)) {
    return;
  }
  DIR *dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "Unable to open dir: %s, %s\n", path, strerror(errno));
    return;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (isSpecial(entry)) {
      continue;
    }
    char *filename = entry->d_name;
    int pathLen = strlen(path);
    int filenameLen = strlen(filename);
    int newPathLen = filenameLen + pathLen + 2;
    char *newPath = malloc(sizeof(char) * newPathLen);
    memset(newPath, 0, newPathLen);
    memcpy(newPath, path, pathLen);
    newPath[pathLen] = '/';
    memcpy(&newPath[pathLen + 1], filename, filenameLen);
    walk(newPath, depth);
    free(newPath);
  }
  closedir(dir);
}

int main(int argc, char **argv) {
  if (argc <= 1 || argc >= 3) {
    printf("Usage: %s dir\n", argv[0]);
  }
  char *path = argv[1];
  walk(path, 100);
  return 0;
}
