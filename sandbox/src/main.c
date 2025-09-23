#include <stdio.h>

static inline int rotate(int v) {
  //
  return (v << 1 & 0b1111) | (v >> 0b0011);
}

int main() {
  int value = 0b1101;
  printf("original: %d, rotated: %d\n", value, rotate(value));
  return 0;
}
