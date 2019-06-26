#version 330 core

out vec4 color;

in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

const vec3 lightDir = vec3(0.25,0.25,-1); //light direction
const vec3 ambient_Color = vec3(0,0,1);
const vec3 diffuse_Color = vec3(0.3,0.3,1);
const vec3 spec_color = vec3(1,1,1);



void main(){


  vec3 norm = normalize(f_in.normal);
  vec3 lightDir = normalize(-lightDir);
  vec3 viewDir = normalize(-f_in.position);
  vec3 halfAng = normalize(lightDir + viewDir);


  float reflectDir = max(dot(halfAng,norm),0);
  float lambertian = max(dot(lightDir,norm),0);
  float NdL = lambertian;
  float roughness = 0.1;

  float resulting = 1;

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
  float f = f0 + (1-f0)*pow(1-NdV,5); 

  //final caluction for spec
  resulting = (d*f*g) / (4* NdL * NdV);


  // put it all together
  vec3 result = ambient_Color + lambertian * diffuse_Color + resulting *spec_color;

  color = vec4(result, 1.0);


}
