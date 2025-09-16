#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 outColor;

void main() {
  gl_Position = vec4(inPosition, 1.0); 
  outColor = vec4(color, 1.0);
}
