#version 330 core



// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;


in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} frag_in;

out vec4 color;

const vec3 direction = vec3(0,0,-1); //light direction
const vec3 ambient_Color = vec3(0.7,0.7,0.7);
const vec3 diffuse_Color = vec3(0,0,0);
const vec3 spec_color = vec3(0.7,0.7,0.7);

//const vec3 direction = vec3(0.25, 0.25, -1);
const vec3 object = vec3(1, 1, 1);

void main(){

    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * ambient_Color;

    vec3 norm = normalize(frag_in.normal);
    vec3 lightDir = normalize(-direction);

    float diff = max(dot(norm, lightDir), 1.0);
    vec3 diffuse = diff * diffuse_Color;

    float specStrength = 4;
    vec3 reflectD = reflect(-direction, norm);
    vec3 viewDir = normalize(-frag_in.position);

    float spec = pow(max(dot(viewDir, reflectD), 0.0), 35);
    vec3 specular = specStrength * spec * spec_color;
//
//    vec3 result1 = (ambient + diffuse + specular) * object;
    
//      vec3 norm = normalize(frag_in.normal);
//      vec3 lightDir = normalize(-direction);
//      vec3 viewDir = normalize(-frag_in.position);
      vec3 halfAng = normalize(lightDir + viewDir);

      float reflectDir = max(dot(halfAng,norm),0);
      float lambertian = max(dot(lightDir,norm),0);
      float NdL = lambertian;
      float roughness = 0.05;

      float resulting = 0.5;

      float f0 = 0.8;

      vec3 H = (viewDir +lightDir)/(length(viewDir+lightDir));
      float NdH = max(0,dot(norm,H));
      float NdV = max(0,dot(norm,viewDir));
      float VdH = max(0,dot(viewDir, H));


      float albedo = pow(roughness,2);
      float PI = 3.1415926535897932384626433832795028841971;

      //beckman
      float d  = albedo/pow(PI*(1+pow(NdH,2)*(albedo-1)),2);

      //geometry/microfacet func
      float g = min(1,min(2*(NdV*NdH/VdH), 2*(NdL*NdH/VdH)));

      //fresnel func
      float f = f0 + (1-f0)*pow(1-NdV,10);

      //final caluction for spec
      resulting = (d*f*g) / (4* NdL * NdV);


      // put it all together
    
      vec3 result = ambient + lambertian * diffuse + resulting * specular;
    
      color = vec4(result, 1.0f);



}
