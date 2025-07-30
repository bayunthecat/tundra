#include "engine.h"
#include <stdio.h>

int main() {
  Engine *e = makeEngine();
  run(e);
  freeEngine(e);
  printf("Success\n");
  return 0;
}
