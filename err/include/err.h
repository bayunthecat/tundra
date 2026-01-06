#ifndef ERR_H
#define ERR_H

typedef enum { SUCCESS, FAIL } ErrStatus;

typedef struct {
  ErrStatus status;
  char *desc;
  struct Err *cause;
} Err;

#endif // !ERR_H
