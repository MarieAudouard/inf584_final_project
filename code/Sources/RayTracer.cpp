// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#define _USE_MATH_DEFINES

#include "RayTracer.h"

#include "Console.h"
#include "Camera.h"
#include "Lightsources.h"

RayTracer::RayTracer() : m_imagePtr(std::make_shared<Image>()) {}

RayTracer::~RayTracer() {}

void RayTracer::init(const std::shared_ptr<Scene> scenePtr)
{
}

RayHit RayTracer::rayTriangle(const Ray &ray, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2)
{
	float epsilon = 0.000001f;
	RayHit hit;
	hit.setIsHit(false);

	glm::vec3 e0 = p1 - p0;
	glm::vec3 e1 = p2 - p0;
	glm::vec3 n = normalize(glm::cross(e0, e1));
	glm::vec3 q = glm::cross(ray.direction(), e1);
	float a = glm::dot(e0, q);
	if (glm::dot(n, ray.direction()) >= 0 || abs(a) < epsilon)
	{
		return hit;
	}
	glm::vec3 s = (ray.origin() - p0) / a;
	glm::vec3 r = glm::cross(s, e0);
	float b1 = glm::dot(s, q);
	float b2 = glm::dot(r, ray.direction());
	float b0 = 1 - b2 - b1;
	if (b0 < 0 || b1 < 0 || b2 < 0)
		return hit;
	float t = glm::dot(e1, r);
	if (t >= 0)
	{
		hit.setIsHit(true);
		hit.setB0(b0);
		hit.setB1(b1);
		hit.setB2(b2);
		hit.setT(t);
	}
	return hit;
}

RayHit RayTracer::rayScene(const Ray &ray, const std::shared_ptr<Scene> scenePtr)
{
	RayHit best_hit;
	// we need to reset it because else it keeps the previous value but only for isHit (haven't figured out why)
	best_hit.setIsHit(false);
	//  For each mesh of the scene
	size_t numOfMeshes = scenePtr->numOfMeshes();
	for (size_t i = 0; i < numOfMeshes; i++)
	{
		// Compute the intersection between the ray and each triangle of the mesh
		std::shared_ptr<Mesh> mesh = scenePtr->mesh(i);
		size_t numOfTriangles = mesh->triangleIndices().size();
		for (size_t j = 0; j < numOfTriangles; j++)
		{
			RayHit hit;
			hit.setIsHit(false);
			glm::uvec3 triangle = mesh->triangleIndices()[j];
			glm::vec3 p0 = mesh->vertexPositions()[triangle[0]];
			glm::vec3 p1 = mesh->vertexPositions()[triangle[1]];
			glm::vec3 p2 = mesh->vertexPositions()[triangle[2]];
			hit = rayTriangle(ray, p0, p1, p2);
			// If the ray intersects the mesh
			if (hit.isHit() && (!best_hit.isHit() || hit.t() < best_hit.t()))
			{
				// If the intersection is closer than the previous one
				hit.setMeshIdx(i);
				hit.setTriangleIdx(j);
				best_hit = hit;
			}
		}
	}
	return best_hit;
}

void RayTracer::render(const std::shared_ptr<Scene> scenePtr)
{
	size_t width = m_imagePtr->width();
	size_t height = m_imagePtr->height();
	std::chrono::high_resolution_clock clock;
	Console::print("Start ray tracing at " + std::to_string(width) + "x" + std::to_string(height) + " resolution...");
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	m_imagePtr->clear(scenePtr->backgroundColor());

	// <---- Ray tracing code ---->
	// For each pixel of the image
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			// Compute the ray corresponding to the pixel
			Ray ray = scenePtr->camera()->rayAt((float)x, (float)y, (float)height, (float)width);
			// Compute the color of the pixel by tracing the ray
			RayHit hit = rayScene(ray, scenePtr);
			if (hit.isHit())
			{
				glm::vec3 color = shade(ray, hit, scenePtr);
				m_imagePtr->setPixel(x, y, color);
			}
			// glm::vec3 color = trace(ray, scenePtr);
			// Set the color of the pixel
			// m_imagePtr->setPixel(x, y, color);
		}
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Ray tracing executed in " + std::to_string(elapsedTime) + "ms");
}

glm::vec3 toneMapping(glm::vec3 radiance, float gamma, float exposure)
{
	glm::vec3 rgb = exposure * radiance;
	return glm::vec3(pow(rgb.r, gamma), pow(rgb.g, gamma), pow(rgb.b, gamma));
}

glm::vec3 attenuation(std::shared_ptr<PointLightsource> l, glm::vec3 lightPosition, glm::vec3 p)
{
	float d = distance(lightPosition, p);
	return l->intensity() * l->color() / (d * d);
}

glm::vec3 diffuseBRDF(std::shared_ptr<Material> m)
{
	return (float)((1.0 - m->metallicness()) / M_PI) * m->albedo();
}

float D_GGX(float NoH, float a)
{
	float a2 = a * a;
	float f = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (M_PI * f * f);
}

glm::vec3 F_Schlick(float u, glm::vec3 f0)
{
	return f0 + (float)pow(1.0 - u, 5.0) * (glm::vec3(1.0) - f0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
	float a2 = a * a;
	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
	float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
	return 0.5 / (GGXV + GGXL);
}

glm::vec3 microfacetBRDF(std::shared_ptr<Material> m, glm::vec3 n, glm::vec3 v, glm::vec3 l)
{
	glm::vec3 h = normalize(v + l);
	float NoV = abs(dot(n, v)) + 1e-5;
	float NoL = clamp(dot(n, l), 0.0f, 1.0f);
	float NoH = clamp(dot(n, h), 0.0f, 1.0f);
	float LoH = clamp(dot(l, h), 0.0f, 1.0f);

	// perceptually linear roughness to roughness (see parameterization)
	float roughness = m->roughness() * m->roughness();

	float D = D_GGX(NoH, roughness);
	float reflectance = 1.0;
	glm::vec3 f0 = 0.16f * reflectance * reflectance * (1.0f - m->metallicness()) + m->albedo() * m->metallicness();
	glm::vec3 F = F_Schlick(LoH, f0);
	float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
	return D * V * F;
}

glm::vec3 RayTracer::shade(const Ray &ray, const RayHit &hit, const std::shared_ptr<Scene> scenePtr)
{
	// get all the necessary information
	std::shared_ptr<PointLightsource> point_lightsource = scenePtr->pointlightsource();
	std::shared_ptr<DirectionalLightsource> directional_lightsource = scenePtr->directionallightsource();
	std::shared_ptr<Mesh> mesh = scenePtr->mesh(hit.getMeshIdx());
	std::shared_ptr<Material> material = mesh->material();
	glm::uvec3 triangle = mesh->triangleIndices()[hit.getTriangleIdx()];
	glm::vec3 p0 = mesh->vertexPositions()[triangle[0]];
	glm::vec3 p1 = mesh->vertexPositions()[triangle[1]];
	glm::vec3 p2 = mesh->vertexPositions()[triangle[2]];
	glm::vec3 fPosition = hit.getB0() * p0 + hit.getB1() * p1 + hit.getB2() * p2;
	glm::vec3 fNormal = normalize(hit.getB0() * mesh->vertexNormals()[triangle[0]] + hit.getB1() * mesh->vertexNormals()[triangle[1]] + hit.getB2() * mesh->vertexNormals()[triangle[2]]);

	// diffuse lambert

	// // point light source code
	// glm::vec3 wi1 = glm::normalize(point_lightsource->position() - fPosition);
	// float dist = glm::length(point_lightsource->position() - fPosition);
	// float cosTheta1 = std::max(0.f, glm::dot(fNormal, normalize(wi1)));
	// glm::vec3 lambert_coeff = (float)((1.0f - material->metallicness()) / M_PI) * material->albedo();
	// float denum = point_lightsource->ac() + point_lightsource->al() * dist + point_lightsource->aq() * dist * dist;
	// glm::vec4 colorResponse1 = glm::vec4(cosTheta1 * lambert_coeff * point_lightsource->color() * point_lightsource->intensity() / denum, 1.0);

	// // directional light source code
	// float cosTheta2 = std::max(0.f, glm::dot(fNormal, -directional_lightsource->direction()));
	// glm::vec4 colorResponse2 = glm::vec4(cosTheta2 * lambert_coeff * directional_lightsource->color() * directional_lightsource->intensity(), 1.0);

	// glm::vec4 colorResponse = colorResponse2;

	// Ray to_light = Ray(fPosition, point_lightsource->position() - fPosition);
	// RayHit light_hit = rayScene(to_light, scenePtr);
	// if (!light_hit.isHit())
	// {
	// 	colorResponse += colorResponse1;
	// }

	// BRDF microfacets
	glm::vec3 radiance = glm::vec3(0.0);
	glm::vec3 n = glm::normalize(fNormal);
	glm::vec3 wo = glm::normalize(-fPosition);
	glm::vec3 lightPosition = glm::vec3(glm::vec4(point_lightsource->position(), 1.0));
	glm::vec3 wi = glm::normalize(lightPosition - fPosition);
	glm::vec3 li = attenuation(point_lightsource, lightPosition, fPosition);
	glm::vec3 fd = diffuseBRDF(material);
	glm::vec3 fs = microfacetBRDF(material, n, wo, wi);
	glm::vec3 fr = fd + fs;
	float nDotL = max(0.0f, dot(n, wi));
	radiance += li * fr * nDotL;
	// directional light source code
	float cosTheta2 = max(0.0f, dot(fNormal, -directional_lightsource->direction()));
	glm::vec3 colorResponse2 = cosTheta2 * fd * directional_lightsource->color() * directional_lightsource->intensity();
	radiance += colorResponse2;
	radiance = toneMapping(radiance, 1.0, 1.0);
	glm::vec4 colorResponse = glm::vec4(radiance, 1.0);

	return colorResponse;
}
