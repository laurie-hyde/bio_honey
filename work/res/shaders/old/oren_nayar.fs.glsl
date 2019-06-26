#version 330 core

out vec4 color;

in VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
} f_in;

const vec3 lightDirection = vec3(0, 0, -1);

const vec3 ambientColor = vec3(1.0, 0.5, 0.5);
const vec3 diffuseColor = vec3(0.3, 0.3, 1.0);
const vec3 specColor = vec3(0.1, 0.1, 0.1);

void main(){
    float roughness = 0.1;
    vec3 viewDirection = normalize(-f_in.position);
    vec3 lightDirection = normalize(-lightDirection);
    vec3 norm = normalize(f_in.normal);
    float lambertian = max(dot(lightDirection, norm), 0.0);

    float viewDotNorm = dot(viewDirection, norm);
    float lightDotNorm = dot(lightDirection, norm);
    float difference = dot(normalize(viewDirection - norm * viewDotNorm), normalize(lightDirection - norm * lightDotNorm));
    
    float angleX = acos(viewDotNorm);
    float angleY = acos(lightDotNorm);
    float maxAngle = max(angleX, angleY);
    float minAngle = min(angleX, angleY);
    
    float rough = roughness * roughness;
    float x = 1.0 - 0.5 * rough / (rough + 0.33);
    float y = 0.45 * rough / (rough + 0.09);
    
    if(difference >= 0) {
         y *= sin(maxAngle) * tan(minAngle);
    }
    else {
        y *= 0;
    }
   
    float result = lightDotNorm * (x + y);
    
    vec3 fragColor = ambientColor + lambertian * diffuseColor + result * specColor;
    
    color = vec4(fragColor, 1.0);
    
}


