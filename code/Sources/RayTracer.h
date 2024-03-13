// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Image.h"
#include "Scene.h"
#include "RayHit.h"

using namespace std;

class RayTracer
{
public:
	RayTracer();
	virtual ~RayTracer();

	inline void setResolution(int width, int height) { m_imagePtr = make_shared<Image>(width, height); }
	inline std::shared_ptr<Image> image() { return m_imagePtr; }
	void init(const std::shared_ptr<Scene> scenePtr);
	void render(const std::shared_ptr<Scene> scenePtr);
	RayHit rayTriangle(const Ray &ray, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2);
	RayHit rayScene(const Ray &ray, const std::shared_ptr<Scene> scenePtr);
	glm::vec3 shade(const Ray &ray, const RayHit &hit, const std::shared_ptr<Scene> scenePtr);

private:
	std::shared_ptr<Image> m_imagePtr;
};