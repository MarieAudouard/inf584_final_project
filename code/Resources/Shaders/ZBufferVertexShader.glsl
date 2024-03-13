#version 410 core // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition; // The 1st input attribute is the position (CPU side: glVertexAttrib 0)

uniform mat4 projectionMat, modelViewMat; // Uniform variables, set from the CPU-side main program

out vec3 fPosition;

void main() {
	vec4 p = modelViewMat * vec4 (vPosition, 1.0);
    gl_Position =  projectionMat * p; // mandatory to fire rasterization properly
    fPosition = p.xyz;
}