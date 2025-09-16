#version 450

layout(binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
  vec2 resolution;
} ubo;

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 outColor;

void main() {
  vec3 a = vec2(3, 2, 0);
  vec3 b = vec2(1, 0, 0);
  
  vec2 uv = (gl_FragCoord.xy / ubo.resolution) * 2.0 - 1.0;
  outColor = vec4(uv.x, uv.y, 0.0, 1.0);
}
