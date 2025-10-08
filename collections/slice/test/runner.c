#include "slice.h"
#include "test.h"
#include <string.h>

void testSliceNominal(TestRun *r) {
  Slice *s = sliceMake();
  sliceAppend(s, "hello");
  char *val = sliceGet(s, 0);
  if (strcmp(val, "hello") != 0) {
    testFail(r, "Error getting the value from the slice");
  }
  sliceFree(s);
}

void testSliceGrow(TestRun *r) {
  int len = 100;
  Slice *s = sliceMake();
  for (int i = 0; i < len; i++) {
    sliceAppend(s, "hello");
  }
  int newLen = sliceLen(s);
  if (newLen != len) {
    testFail(r, "want one, got another");
  }
  sliceFree(s);
}

int main() {
  TestSuite *s = testSuiteMake("slice");
  testRegisterFn(s, "testSliceNominal", testSliceNominal);
  testRegisterFn(s, "testSliceGrow", testSliceGrow);
  testRun(s);
  testSuiteFree(s);
  return 0;
}
