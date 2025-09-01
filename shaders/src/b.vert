struct Tile {
  vec3 position;
  vec3 color;
};

layout(set = 0, binding = 0) buffer TileBuffer {
  Tile tiles[];
}

void main() {
  Tile t = tiles[gl_InstanceIndex];
  gl_Position = proj * view * t.position * vec4(inPosition, 1.0) 
}
