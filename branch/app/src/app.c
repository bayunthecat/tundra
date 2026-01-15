#include "view.h"

int main() {
  View *v = makeEngine();
  run(v);
  freeView(v);
  return 0;
}
