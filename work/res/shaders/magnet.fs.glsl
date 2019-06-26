#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
    vec2 textureCoord;
} frag_in;

// framebuffer output
out vec4 color;

const vec3 direction = vec3(0.25, 0.25, -1);
const vec3 light = vec3(1, 1, 1);
const vec3 object = vec3(0, 0.9, 0);


void main() {
    
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * light;
    
    vec3 norm = normalize(frag_in.normal);
    vec3 lightDir = normalize(-direction);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light;
    
    float specStrength = 0.5;
    vec3 reflectDir = reflect(-direction, norm);
    vec3 viewDir = normalize(-frag_in.position);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);
    vec3 specular = specStrength * spec * light;
    
    vec3 result = (ambient + diffuse + specular) * object;
    
    color = vec4(result, 0.7f);
    
    
}
