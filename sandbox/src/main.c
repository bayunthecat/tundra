#include <stdio.h>
#include <time.h>

int main() {
  clock_t start = clock();
  clock_t end;
  int printed = 0;
  double curr = 0;
  int currInt = 0;
  for (int i = 0;; i++) {
    end = clock();
    curr = (double)(end - start) / CLOCKS_PER_SEC;
    currInt = (int)curr;
    if (currInt != printed) {
      printf("elapsed: %f\n", curr);
      printed = currInt;
    }
  }
}
