#include "slice.h"
#include "test.h"
#include <string.h>

void testSliceNominal(TestSuite *su) {
  Slice *s = sliceMake();
  sliceAppend(s, "hello");
  char *val = sliceGet(s, 0);
  if (strcmp(val, "hello") != 0) {
    fail(su, "Error getting the value from the slice");
  }
  sliceFree(s);
}

void testSliceGrow(TestSuite *su) {
  int len = 100;
  Slice *s = sliceMake();
  for (int i = 0; i < len; i++) {
    sliceAppend(s, "hello");
  }
  int newLen = sliceLen(s);
  if (newLen != len) {
    fail(su, "want one, got another");
  }
  sliceFree(s);
}

int main() {
  TestSuite *s = testSuiteMake("slice");
  registerFn(s, "testSliceNominal", testSliceNominal);
  registerFn(s, "testSliceGrow", testSliceGrow);
  run(s);
  testSuiteFree(s);
  return 0;
}
