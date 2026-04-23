#version 450

layout(location = 0) in vec3 inPosition;

void main() {
  float z = inPosition.z;
  gl_Position = vec4(inPosition.xy, z, z);
  gl_PointSize = 5.0f;
}
