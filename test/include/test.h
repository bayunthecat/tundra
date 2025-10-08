#ifndef TEST_H
#define TEST_H

#include <stddef.h>
typedef struct TestSuite TestSuite;

typedef struct TestRun TestRun;

typedef void (*test)(TestRun *);

TestSuite *testSuiteMake(char *name);

void testSuiteFree(TestSuite *s);

void testRegisterFn(TestSuite *s, char *name, test);

void testFail(TestRun *r, char *reason);

void testVerify(TestRun *r, void *want, size_t wantSize, void *got,
                size_t gotSize);

void testRun(TestSuite *s);

#endif // !TEST_H
