#include "sandbox.h"

int main() {
  Sandbox *e = makeEngine();
  run(e);
  freeEngine(e);
  return 0;
}
