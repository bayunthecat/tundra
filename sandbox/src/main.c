#include "engine.h"

int main() {
  Engine *e = makeEngine();
  run(e);
  freeEngine(e);
  return 0;
}
