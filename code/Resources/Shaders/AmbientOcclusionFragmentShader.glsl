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

int N_d = 8; // number of sampled angles
int N_s = 8; // number of samples to calculate h(theta)
float R = 1.0; // radius of influence

struct Horizon {
   float t_theta;
   float h_theta;
   vec3 horizon_point;
};

vec3 to_cam_coords(vec2 texCoords) {
   float px = ((2 * texCoords.x / width) - 1.f) * tan(radians(fov) / 2.f) * aspectRatio;
   float py = ((2 * texCoords.y / height) - 1.f) * tan(radians(fov) / 2.f);
   return vec3(px, py, texture(imageTexZ, texCoords).r);
}

float W(float r) {
   return max(0, 1 - r / R);
}

float compute_angle(vec3 D) {
   return atan(-D.z / length(D.xy));
   //return asin(dot(normalize(D), vec3(0, 0, -1)));
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

Horizon compute_h_theta(float theta, float R_projected, vec3 P, Horizon horizon) {
   float s_step = R_projected / N_s;
   for (int i = 0; i < N_s; i++) {
      float s = i * s_step;
      vec2 S_i_tex = fTexCoords + vec2(s * cos(theta), s * sin(theta));
      horizon = update_alpha_s_i(P, S_i_tex, horizon);
   }
   return horizon;
}

Horizon compute_for_theta(float theta, vec3 N, vec3 V, vec3 t0, vec3 t1, vec3 P) {
   vec3 t = normalize(t0 * cos(theta) + t1 * sin(theta));
   vec3 normal = normalize(cross(t, V)); // normal to the plane defined by t and V
   float R_in_image_coords = R / tan(radians(fov) / 2.f) * (height/2);
   float R_projected = R_in_image_coords * dot(normal, V);
   float t_theta = compute_angle(t);
   Horizon horizon = Horizon(t_theta, t_theta, P + 2*R*vec3(1.0)); // we initialize the horizon point to a point far away to ensure that the W(theta) will be 0 if h_theta=t_theta
   horizon = compute_h_theta(theta, R_projected, P, horizon);
   return horizon; // we return t_theta and h_theta
}

void main () {
   vec3 in_cam_coords = to_cam_coords(fTexCoords);
   //vec3 V = normalize(-in_cam_coords); // the camera is at (0,0,0) in camera space
   vec3 V = vec3(0, 0, -1); // not the actual view vector, but what seems to be done in the paper
   vec3 N = texture(imageTexNormal, fTexCoords).xyz;
   vec3 t0 = normalize(cross(N, V));
   vec3 t1 = normalize(cross(t0, N));

   float A = 0.0;

   float theta_min = 0.0; // will be used for jittering later
   float theta_max = 2 * 3.14159;
   float theta_step = (theta_max - theta_min) / N_d;
   for (int i = 0; i < N_d; i++) {
      float theta = theta_min + i * theta_step;
      Horizon horizon = compute_for_theta(theta, N, V, t0, t1, in_cam_coords);
      A += (sin(horizon.h_theta) - sin(horizon.t_theta)) * W(length(in_cam_coords - horizon.horizon_point));
   }

   A = 1 - A / N_d;

   colorResponse = vec4 (vec3(A), 1.0);
}