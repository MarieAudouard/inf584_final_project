#version 410 core // Minimal GL version support expected from the GPU

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)

out vec4 colorResponse; // Shader output: the color response attached to this fragment. here the simlple content of the bounded image texture

// void main () {
// 	colorResponse = vec4 (vec3(fPosition.z), 1.0);	
// }


// for linearized depth
uniform float near; // Near clipping plane
uniform float far; // Far clipping plane

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    float depth = LinearizeDepth(fPosition.z) / far; // divide by far for demonstration
    colorResponse = vec4(vec3(depth), 1.0);
}