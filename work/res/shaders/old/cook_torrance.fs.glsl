#version 330 core
#define PI 3.141592653589

out vec4 color;

in VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
    } f_in;

const vec3 lightDirection = vec3(0.25, 0.25, -1);

const vec3 ambientColor = vec3(1.0, 0.5, 0.5);
const vec3 diffuseColor = vec3(0.4, 0.4, 1.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

const float shine = 12.0;

void main(){
    vec3 normal = normalize(f_in.normal);
    vec3 lightDirection = normalize(-lightDirection);
    vec3 viewDirection = normalize(-f_in.position);
    vec3 halfDirection = normalize(viewDirection + lightDirection);
    
    float results = 1;
    float specular = 0.0;
    float roughness = 0.1;
    float k = 0.2;
    float F0 = 0.5;
    
    float specAngle = max(dot(halfDirection, normal), 0.0);
    float lambertian = max(dot(lightDirection, normal), 0.0);
    float normDotLight = max(dot(lightDirection, normal), 0.0);
    
    float albedo = pow(roughness, 2);
    
    float normDotView = max(0, dot(normal, viewDirection));
    vec3 H = (viewDirection + lightDirection) / (length(viewDirection + lightDirection));
    float viewDotH = max(0, dot(viewDirection, H));
    float normDotH = max(0, dot(normal, H));
    
    float fresnel = F0 + (1 - F0) * pow (1 - normDotView, 5);
    float distribution = albedo / pow (PI * (1 + pow (normDotH, 2) * (albedo - 1)), 2);
    float geometry = min(1, min(2 * (normDotView * normDotH / viewDotH), 2 * (normDotLight * normDotH / viewDotH)));
    
    results = (distribution * fresnel * geometry) / (4 * normDotLight * normDotView);
    
    vec3 fragColor = ambientColor + lambertian * diffuseColor + results * specColor;
    color = vec4(fragColor, 1.0);
}
