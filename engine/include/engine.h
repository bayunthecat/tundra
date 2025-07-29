#ifndef ENGINE_H
#define ENGINE_H

typedef struct Engine Engine;

Engine *makeEngine();

void freeEngine(Engine *engine);

#endif // !ENGINE_H
