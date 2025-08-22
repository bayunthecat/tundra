#ifndef TEST_H
#define TEST_H

typedef struct TestSuite TestSuite;

typedef void (*test)(TestSuite *);

TestSuite *testSuiteMake(char *name);

void testSuiteFree(TestSuite *s);

void registerFn(TestSuite *s, char *name, test);

void fail(TestSuite *s, char *reason);

void run(TestSuite *s);

#endif // !TEST_H
