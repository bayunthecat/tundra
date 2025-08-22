#ifndef SLICE_H
#define SLICE_H

typedef struct Slice Slice;

Slice *sliceMake();

void sliceFree(Slice *s);

void sliceAppend(Slice *s, void *v);

void *sliceGet(Slice *s, unsigned int index);

unsigned int sliceLen(Slice *s);

#endif
