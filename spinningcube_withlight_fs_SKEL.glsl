#version 130

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

out vec4 frag_col;

in vec3 frag_3Dpos;
in vec3 vs_normal;
in vec2 vs_tex_coord;
in vec3 vs_color;

uniform Material material;
uniform Light light;
uniform vec3 view_pos;

void main() {
  // Ambient
  vec3 ambient = light.ambient * material.ambient;

  // Diffuse
  vec3 light_dir = normalize(light.position - frag_3Dpos);
  float diff = max(dot(vs_normal, light_dir), 0.0);
  vec3 diffuse = light.diffuse * (diff * material.diffuse);
  // Specular

  vec3 result = vs_color * (diffuse + ambient);
  frag_col = vec4(result, 1.0);
}
