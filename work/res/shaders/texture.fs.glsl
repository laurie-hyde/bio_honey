#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform sampler2D textureSampler;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {
	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 color = texture(textureSampler,f_in.textureCoord).rgb;

	// output to the frambuffer
	fb_color = vec4(color, 1);
}
