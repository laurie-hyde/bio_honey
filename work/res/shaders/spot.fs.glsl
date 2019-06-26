//
#version 330 core
//
//
//    vec3 position;
//    vec3 direction = vec3(0.25,0.25,-1);
//    float cutOff = 20;
//    float outerCutOff = 20;
//
//    float constant;
//    float linear;
//    float quadratic;
//
//    vec3 ambient = vec3(0,1,1);
//    vec3 diffuse = vec3(0.3,0.3,1);
//    vec3 specular = vec3(1,1,1);
//
//in VertexData {
//    vec3 position;
//    vec3 Normal;
//    vec2 textureCoord;
//} f_in;
//
//out vec4 color;
//
//uniform vec3 viewPos;
//float spec = 0;
//const vec3 lightDir = vec3(0.25,0.25,-1);
////uniform Material material;
////uniform Light light;
//
//void main()
//{
//    // Ambient
//    vec3 ambient = ambient * vec3(texture(matDiffuse, f_in.textureCoord));
//
//    // Diffuse
//    vec3 norm = normalize(f_in.Normal);
//    vec3 lightDir = normalize(-lightDir);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diffuse * diff * vec3(texture(matDiffuse, f_in.textureCoord));
//
//    // Specular
//    vec3 viewDir = normalize(-f_in.position);
//    vec3 reflectDir = reflect(-lightDir, norm);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//    vec3 specular = specular * spec * vec3(texture(matSpecular, f_in.textureCoord));
//
//    // Spotlight (soft edges)
//    float theta = dot(lightDir, normalize(-direction));
//    float epsilon = (cutOff - outerCutOff);
//    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
//    diffuse  *= intensity;
//    specular *= intensity;
//
//    // Attenuation
//    float distance = length(position - f_in.position);
//    float att = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
//    ambient  *= att;
//    diffuse  *= att;
//    specular *= att;
//
//    color = vec4(ambient + diffuse + specular, 1.0f);
//}















out vec4 colorOut;

layout (std140) uniform Materials {
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    float shininess;
};

layout (std140) uniform Lights {
    vec4 l_pos, l_spotDir;
    float l_spotCutOff;
};

in Data {
    vec3 normal;
    vec3 eye;
    vec3 lightDir;
} DataIn;

void main() {
    
    float intensity = 0.0;
    vec4 spec = vec4(0.0);
    
    vec3 ld = normalize(DataIn.lightDir);
    vec3 sd = normalize(vec3(-l_spotDir));
    
    // inside the cone?
    if (dot(sd,ld) > l_spotCutOff) {
        
        vec3 n = normalize(DataIn.normal);
        intensity = max(dot(n,ld), 0.0);
        
        if (intensity > 0.0) {
            vec3 eye = normalize(DataIn.eye);
            vec3 h = normalize(ld + eye);
            float intSpec = max(dot(h,n), 0.0);
            spec = specular * pow(intSpec, shininess);
        }
    }
    
    colorOut = max(intensity * diffuse + spec, ambient);
}
