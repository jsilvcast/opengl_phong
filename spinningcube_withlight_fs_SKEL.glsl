#version 130

struct Material {
    vec3 ambient;
    sampler2D diffuse;
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
#define NR_POINT_LIGHTS 4
uniform Light lights[NR_POINT_LIGHTS];
uniform vec3 view_pos;

vec3 CalcPointLight(Light light, vec3 vs_normal, vec3 frag_3Dpos, vec3 view_pos)
{
//    vec3 ambient = light.ambient * material.ambient;
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vs_tex_coord));

    // Diffuse
    vec3 light_dir = normalize(light.position - frag_3Dpos);
    float diff = max(dot(vs_normal, light_dir), 0.0);
//    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, vs_tex_coord)));
    // Specular
    vec3 view_dir = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir = reflect(-light_dir, vs_normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = vs_color * (diffuse + ambient + specular);
    return result;
}

void main() {

    vec3 result = vec3(0.0);
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(lights[i], vs_normal, frag_3Dpos, view_pos);
    }
    frag_col = vec4(result, 1.0);
}


