#version 410 core // Minimal GL version support expected from the GPU

#define PI 3.1415926535897932384626433832795

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)

layout (location = 1) out vec3 texNormal; // Shader output: the normal attached to this fragment. here the simlple content of the bounded image texture
layout (location = 2) out vec4 texColor; // Shader output: the color response attached to this fragment. here the simlple content of the bounded image texture

struct DirectionalLightSource {
	vec3 direction;
	vec3 color;
	float intensity;
};

struct PointLightSource {
	vec3 position;
	vec3 color;
	float intensity;
	float ac;
   float al;
   float aq;
};

struct Material {
	vec3 albedo;
	float roughness;
	float metallicness;
};

uniform DirectionalLightSource directional_lightsource;
uniform PointLightSource point_lightsource;
uniform Material material;

vec3 toneMapping(vec3 radiance, float gamma, float exposure) {
	vec3 rgb = exposure * radiance;
	return vec3(pow(rgb.r, gamma), pow(rgb.g, gamma), pow(rgb.b, gamma));
}


vec3 attenuation(PointLightSource l, vec3 lightPosition, vec3 p) {
	float d = distance (lightPosition, p);
	return l.intensity*l.color/(d*d);
}

vec3 diffuseBRDF(Material m) {
	return (1.0 - m.metallicness) * m.albedo/PI;
}

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

vec3 F_Schlick(float u, vec3 f0) {
	return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

vec3 microfacetBRDF(Material m, vec3 n, vec3 v, vec3 l)
{
	vec3 h = normalize(v + l);
    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    // perceptually linear roughness to roughness (see parameterization)
    float roughness = m.roughness * m.roughness;

    float D = D_GGX(NoH, roughness);
    float reflectance  = 1.0;
    vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - m.metallicness) + m.albedo * m.metallicness;
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return D*V*F;
}

void main () {
	vec3 radiance = vec3(0.0);
	vec3 n = normalize(fNormal);
	vec3 wo = normalize(-fPosition);
	vec3 wi = normalize(point_lightsource.position - fPosition);
	vec3 li = attenuation(point_lightsource, point_lightsource.position, fPosition);
	vec3 fd = diffuseBRDF(material);
	vec3 fs = microfacetBRDF(material, n, wo, wi);
	vec3 fr = fd+fs;
	float nDotL = max(0.0, dot(n, wi));
	radiance += li * fr * nDotL;
	//directional light source code
	float cosTheta2 = max(0, dot(fNormal, -directional_lightsource.direction));
 	vec3 colorResponse2 = cosTheta2 * fd * directional_lightsource.color * directional_lightsource.intensity;
	radiance += colorResponse2.rgb;
	radiance = toneMapping(radiance, 1.0, 1.0);
    // according to https://registry.khronos.org/OpenGL-Refpages/gl4/html/gl_FragDepth.xhtml the depth value is automatically set by the fragment shader
    texNormal = n;
	texColor = vec4(radiance, 1.0);
}