#ifndef PEON_H
#define PEON_H

typedef struct PeonOpts {
  char **skipDirs;
} PeonOpts;

typedef struct PeonTarget {
  char *path;

  char **srcs;
  int srcCount;

  char **includes;
  int includeCount;

  char **flags;
  int flagCount;

  char **tests;
  int testCount;

} PeonTarget;

void parseRoot(char *root, PeonTarget *targets, int *targetCount);

#endif // !PEON_H
