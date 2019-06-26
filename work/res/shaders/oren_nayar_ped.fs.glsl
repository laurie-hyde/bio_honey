
#version 330 core

out vec4 color;

in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

const vec3 lightDir = vec3(-0.5,-0.5,-1); //light direction
const vec3 ambient_Color = vec3(0.5,0.5,0.5);
const vec3 diffuse_Color = vec3(0.5,0.5,0.5);
const vec3 spec_color = vec3(0.5,0.5,0.5);


void main(){


  vec3 norm = normalize(f_in.normal);
  vec3 lightDir = normalize(-lightDir);
  vec3 viewDir = normalize(-f_in.position);
	vec3 halfAng = normalize(lightDir + viewDir);
	float reflectDir = max(dot(halfAng,norm),0);
	float roughness = 0.2; //change for oren
	float PI = 3.1415926535897932384626433832795028841971;

  float albedo = pow(roughness,2);
  float lambertian = max(dot(lightDir,norm),0.0);

  float LdV = dot(lightDir,viewDir);
  float NdL = dot(lightDir,norm);
  float NdV = dot(norm,viewDir);
  float rSqr = roughness * roughness;

  //s
  float s = LdV - NdL * NdV;

  //A
  float A = (1.0 + rSqr *(albedo/(rSqr+ 0.13)+0.5/(rSqr+0.33)));

  //b
  float B = 0.45 * rSqr / (rSqr +0.09);

  //t
  float t =mix(1.0, max(NdL,NdV),step(0.0,s));
  float rs = albedo * max(0.0,NdL) * (A * B *(s/t))/PI;


  // put it all together
  vec3 result = ambient_Color + lambertian * diffuse_Color + rs *spec_color;

  color = vec4(result, 1.0);


}
