#ifndef TEST_H
#define TEST_H

typedef struct TestSuite TestSuite;

typedef struct TestRun TestRun;

typedef void (*test)(TestRun *);

TestSuite *testSuiteMake(char *name);

void testSuiteFree(TestSuite *s);

void registerFn(TestSuite *s, char *name, test);

void fail(TestRun *s, char *reason);

void run(TestSuite *s);

#endif // !TEST_H
