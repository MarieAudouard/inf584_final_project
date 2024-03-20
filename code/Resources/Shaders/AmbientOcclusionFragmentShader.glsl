#version 410 core // Minimal GL version support expected from the GPU
#define PI 3.1415926535897932384626433832795

uniform sampler2D imageTexZ;
uniform sampler2D imageTexNormal;
uniform vec3 cameraPosition;
uniform int width;
uniform int height;
uniform float fov;
uniform float aspectRatio;
uniform float time;

in vec2 fTexCoords;

out vec4 colorResponse; // Shader output: the color response attached to this fragment. here the simlple content of the bounded image texture

int N_d = 8; // number of sampled angles
int N_s = 8; // number of samples to calculate h(theta)
float R = 2.0; // radius of influence

struct Horizon {
   float t_theta;
   float h_theta;
   vec3 horizon_point;
};

// found at https://www.shadertoy.com/view/WljBDh
float random(vec3 pos){
    return fract(sin(dot(pos, vec3(64.25375463, 23.27536534, 86.29678483))) * 59482.7542);
}

// to have a different random number for the jitter step
float random2(vec3 pos){
    return fract(cos(dot(pos, vec3(43.25375463, 32.27536534, 78.29678483))) * 39872.1684);
}

// snaps a uv coord to the nearest texel centre
vec2 snap_to_texel_center(vec2 texCoords)
{
   float texX = (round(texCoords.x * width) + 0.5) / width;
   float texY = (round(texCoords.y * height) + 0.5) / height;
   return vec2(texX, texY);
}

vec3 to_cam_coords(vec2 texCoords) {
   float px = ((2 * texCoords.x) - 1.f) * tan(radians(fov) / 2.f) * aspectRatio;
   float py = ((2 * texCoords.y) - 1.f) * tan(radians(fov) / 2.f);
   return vec3(px, py, texture(imageTexZ, texCoords).r);
}

float W(float r) {
   return max(0, 1 - r / R);
}

float compute_angle(vec3 D) {
   return atan(-D.z / length(D.xy));
}

Horizon update_alpha_s_i(vec3 P, vec2 S_i_tex, Horizon current_value) {
   vec3 S_i = to_cam_coords(S_i_tex);
   vec3 D = S_i - P;
   if (length(D) > R) {
      return current_value;
   }
   else{
      float value = compute_angle(D);
      if (value > current_value.h_theta) {
         return Horizon(current_value.t_theta, value, S_i);
      }
   }
}

Horizon compute_h_theta(float theta, float R_projected, vec3 P, Horizon horizon, vec2 snapped_coords) {
   float real_s_step = R_projected / N_s;
   float s_step = real_s_step + real_s_step * (random2(vec3(snapped_coords, time)) - 0.5) / (4 * N_s); // Jittered step
   for (int i = 0; i < N_s; i++) {
      float s = (i+1) * s_step;
      vec2 S_i_tex = snapped_coords + vec2(s * cos(theta), s * sin(theta));
      vec2 S_i_tex_snapped = snap_to_texel_center(S_i_tex);
      horizon = update_alpha_s_i(P, S_i_tex_snapped, horizon);
   }
   return horizon;
}

Horizon compute_for_theta(float theta, vec3 N, vec3 V, vec3 P, vec2 snapped_coords) {
   vec3 t = normalize(cross(N, vec3(sin(theta), -cos(theta), 0))); // tangent vector "along" theta
   float R_in_image_coords = R / tan(radians(fov) / 2.f) * (1/2);
   float t_theta = compute_angle(t);
   float R_projected = R_in_image_coords * cos(t_theta);
   Horizon horizon = Horizon(t_theta, t_theta, P + 2*R*vec3(1.0)); // we initialize the horizon point to a point far away to ensure that the W(theta) will be 0 if h_theta=t_theta
   horizon = compute_h_theta(theta, R_projected, P, horizon, snapped_coords);
   return horizon; // we return t_theta and h_theta
}

void main () {
   vec2 snapped_coords = snap_to_texel_center(fTexCoords);
   vec3 in_cam_coords = to_cam_coords(snapped_coords);
   vec3 V = vec3(0, 0, -1); // not the actual view vector, but what is used in the paper
   vec3 N = texture(imageTexNormal, snapped_coords).xyz;

   float A = 0.0;

   float theta_step = 2 * PI / N_d;
   float theta_min = (random(vec3(fTexCoords, time)) - 0.5) * theta_step / 5; // Jitter
   for (int i = 0; i < N_d; i++) {
      float theta = theta_min + i * theta_step;
      Horizon horizon = compute_for_theta(theta, N, V, in_cam_coords, snapped_coords);
      A += (sin(horizon.h_theta) - sin(horizon.t_theta)) * W(length(in_cam_coords - horizon.horizon_point));
   }

   A = 1 - A / N_d;

   colorResponse = vec4 (vec3(A), 1.0);
}