#ifndef ENGINE_H
#define ENGINE_H

#include <cglm/cglm.h>

typedef struct Sandbox Sandbox;

Sandbox *makeEngine();

void run(Sandbox *engine);

void freeEngine(Sandbox *engine);

#endif // !ENGINE_H
