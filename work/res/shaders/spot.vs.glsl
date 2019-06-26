#version 330 core

//// uniform data
//uniform mat4 uProjectionMatrix;
//uniform mat4 uModelViewMatrix;
//uniform vec3 uColor;
//
//// mesh data
//layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
//layout(location = 2) in vec2 aTexCoord;
//
//layout (std140) uniform Lights {
//    vec4 l_pos, l_spotDir;
//    float l_spotCutOff;
//};
//
//// model data (this must match the input of the vertex shader)
//out VertexData {
//    vec3 position;
//    vec3 normal;
//    vec2 textureCoord;
//} v_out;
//
//void main() {
//    // transform vertex data to viewspace
//    v_out.position = (uModelViewMatrix * vec4(aPosition, 1)).xyz;
//    v_out.normal = normalize((uModelViewMatrix * vec4(aNormal, 0)).xyz);
//    v_out.textureCoord = aTexCoord;
//
//    // set the screenspace position (needed for converting to fragment data)
//    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1);
//}


layout (std140) uniform Matrices {
    mat4 uProjectionMatrix;
    mat4 uModelViewMatrix;
    vec3 uColor;
};

layout (std140) uniform Lights {
    vec4 l_pos, l_spotDir;
    float l_spotCutOff;
};

in vec4 position;
in vec3 normal;

out Data {
    vec3 normal;
    vec3 eye;
    vec3 lightDir;
} DataOut;

void main () {
    
    vec4 pos = uModelViewMatrix * position;
    
    DataOut.normal = normalize(uColor * normal);
    DataOut.lightDir = vec3(l_pos - pos);
    DataOut.eye = vec3(-pos);
    
    gl_Position = uProjectionMatrix * position;
}
