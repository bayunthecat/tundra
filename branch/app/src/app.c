#include "view.h"

int main() {
  View *v = makeView();
  run(v);
  freeView(v);
  return 0;
}
