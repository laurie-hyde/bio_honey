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
out vec4 frag_color;

//const int num_point_lights = 1;


//const vec3 direction = vec3(0.25, 0.25, -1);
const vec3 light = vec3(0.5, 0.5, 0.5);
//const vec3 object = vec3(0, 0, 1);

float intensity = 5;
vec4 color = vec4(1, 0.5, 1,1);
vec3 position = vec3(0, 0, 0);
float constant = 0;
float linear = 0;
float exponent = 1;
vec3 norm = vec3(0,0,0);

vec4 calcPointLight(float intensity, vec4 color, vec3 position, float constant, float linear, float exponent, vec3 norm){
    vec3 lightDirection = frag_in.position;// - position;
    float distancePoint = length(lightDirection);
    lightDirection = normalize(lightDirection);
    
//    vec4 colorPoint = calcPointLight( intensity,  color,  position,  constant,  linear,  exponent,  norm);
    float attenuation = 1.0f / (0.1f + constant + linear * distancePoint + exponent * distancePoint * distancePoint);
//    vec3 diffuse = calcDirectLight(intensity, color, lightDir, normal);
//    float attenuation = pointLight.atten_in.constant + pointLight.atten_in.linear * distancePoint + pointLight.atten_in.exponent * distancePoint * distancePoint + 0.0001;
    //to do emmision use this equation (attenuation) and then add in emmision
    return color / attenuation;
}

void main() {
        float ambientStrength = 0.5;
        vec3 ambient = ambientStrength * light;
    
    vec4 totalLights = vec4(ambient, 1);
//    vec4 color = vec4(color, 1);
    
    totalLights += calcPointLight(intensity, color, position, constant, linear, exponent, norm);;
    
    frag_color = color * totalLights;
    
//    float ambientStrength = 0.5;
//    vec3 ambient = ambientStrength * light;
//
//    vec3 norm = normalize(frag_in.normal);
//    vec3 lightDir = normalize(-direction);
//
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * light;
//
//    float specStrength = 0.5;
//    vec3 reflectDir = reflect(-direction, norm);
//    vec3 viewDir = normalize(-frag_in.position);
//
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);
//    vec3 specular = specStrength * spec * light;
//
//    vec3 result = (ambient + diffuse + specular) * object;
//
//    frag_color = vec4(result, 1.0f);
    
    
}
