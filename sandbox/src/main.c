#include <stdio.h>

static inline int shiftRight(int v, int times) {
  return (v >> times) | (v << (4 - times) & 0b1111);
}

int main() {
  int value = 0b1101;

  printf("%d, rotated %d times: %d\n", value, 1, shiftRight(value, 1));
  printf("%d, rotated %d times: %d\n", value, 2, shiftRight(value, 2));
  printf("%d, rotated %d times: %d\n", value, 3, shiftRight(value, 3));
  printf("%d, rotated %d times: %d\n", value, 4, shiftRight(value, 4));

  return 0;
}
