#include "peon.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc <= 1 || argc >= 3) {
    printf("Usage: %s dir\n", argv[0]);
  }
  char *path = argv[1];
  PeonTarget targets[20];
  int targetCount = 0;
  parseRoot(path, targets, &targetCount);
  printf("Detected %d targets:\n", targetCount);
  for (int i = 0; i < targetCount; i++) {
    printf("%s\n", targets[i].path);
  }
  for (int i = 0; i < targetCount; i++) {
    free(targets[i].path);
  }
  return 0;
}
