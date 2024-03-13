#version 410 core // Minimal GL version support expected from the GPU

uniform sampler2D imageTexZ;

in vec2 fTexCoords;

out vec4 colorResponse; // Shader output: the color response attached to this fragment. here the simlple content of the bounded image texture

void main () {
	colorResponse = vec4 (vec3(texture(imageTexZ, fTexCoords).r), 1.0);	
	// I don't know why but the depth is only stored in the red channel of the texture
}