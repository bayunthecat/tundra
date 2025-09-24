#include <stdio.h>

static inline int shiftLeft(int v, int times) {
  return (v << times & 0b1111) | (v >> (4 - times));
}

static inline int shiftRight(int v, int times) {
  return (v >> times) | (v << (4 - times) & 0b1111);
}

int main() {
  int value = 0b1101;
  printf("%d, rotated %d times: %d\n", value, 1, shiftLeft(value, 1));
  printf("%d, rotated %d times: %d\n", value, 2, shiftLeft(value, 2));
  printf("%d, rotated %d times: %d\n", value, 3, shiftLeft(value, 3));
  printf("%d, rotated %d times: %d\n", value, 4, shiftLeft(value, 4));

  printf("%d, rotated %d times: %d\n", value, 1, shiftRight(value, 1));
  printf("%d, rotated %d times: %d\n", value, 2, shiftRight(value, 2));
  printf("%d, rotated %d times: %d\n", value, 3, shiftRight(value, 3));
  printf("%d, rotated %d times: %d\n", value, 4, shiftRight(value, 4));

  return 0;
}
