#version 410 core // Minimal GL version support expected from the GPU

uniform sampler2D imageTexZ;
uniform sampler2D imageTexNormal;
uniform vec3 cameraPosition;
uniform int width;
uniform int height;
uniform float fov;
uniform float aspectRatio;

in vec2 fTexCoords;

out vec4 colorResponse; // Shader output: the color response attached to this fragment. here the simlple content of the bounded image texture

void main () {
   float depth = texture(imageTexZ, fTexCoords).r;
   float px = ((2 * fTexCoords.x / width) - 1.f) * tan(radians(fov) / 2.f) * aspectRatio;
   float py = ((2 * fTexCoords.y / height) - 1.f) * tan(radians(fov) / 2.f);
   vec3 in_cam_coords = vec3(px, py, depth);
   vec3 V = normalize(-in_cam_coords); // the camera is at (0,0,0) in camera space

   colorResponse = vec4 (vec3(texture(imageTexZ, fTexCoords).r, 0.0, 0.0), 1.0); // while calculations aren't working, just display Z-Buffer in red
}