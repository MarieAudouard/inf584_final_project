#version 410 core // Minimal GL version support expected from the GPU

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)

out vec3 colorResponse; // Shader output: the color response attached to this fragment. here the simlple content of the bounded image texture

void main () {
	colorResponse = normalize(fNormal);	// this output is not used (for now at least)
    // according to https://registry.khronos.org/OpenGL-Refpages/gl4/html/gl_FragDepth.xhtml the depth value is automatically set by the fragment shader
    //colorResponse = vec4(1.0); 
}