#include "engine.h"
#include <stdio.h>

int main() {
  Engine *e = makeEngine();
  freeEngine(e);
  printf("Success\n");
  return 0;
}
