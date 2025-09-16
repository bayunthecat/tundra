#ifndef ENGINE_H
#define ENGINE_H

#include <cglm/cglm.h>

// TODO does no belong here
typedef struct {
  mat4 model;
  mat4 view;
  mat4 proj;
  vec2 resolution;
} UniformBufferObject;

typedef struct {
  vec3 vertex;
  vec3 color;
  vec2 texture;
} Vertex;
// TODO does no belong here end

typedef struct Engine Engine;

Engine *makeEngine();

void run(Engine *engine);

void freeEngine(Engine *engine);

#endif // !ENGINE_H
