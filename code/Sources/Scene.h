// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "Mesh.h"
#include "Lightsources.h"

class Scene
{
public:
	inline Scene() : m_backgroundColor(0.f, 0.f, 0.f) {}
	virtual ~Scene() {}

	inline const glm::vec3 &backgroundColor() const { return m_backgroundColor; }

	inline void setBackgroundColor(const glm::vec3 &color) { m_backgroundColor = color; }

	inline const std::shared_ptr<DirectionalLightsource> &directionallightsource() const { return m_directional_lightsource; }

	inline void setDirectionalLightsource(const DirectionalLightsource &lightsource) { m_directional_lightsource = std::make_shared<DirectionalLightsource>(lightsource); }

	inline const std::shared_ptr<PointLightsource> &pointlightsource() const { return m_point_lightsource; }

	inline void setPointLightsource(const PointLightsource &lightsource) { m_point_lightsource = std::make_shared<PointLightsource>(lightsource); }

	inline void set(std::shared_ptr<Camera> camera) { m_camera = camera; }

	inline const std::shared_ptr<Camera> camera() const { return m_camera; }

	inline std::shared_ptr<Camera> camera() { return m_camera; }

	inline void add(std::shared_ptr<Mesh> mesh) { m_meshes.push_back(mesh); }

	inline size_t numOfMeshes() const { return m_meshes.size(); }

	inline const std::shared_ptr<Mesh> mesh(size_t index) const { return m_meshes[index]; }

	inline std::shared_ptr<Mesh> mesh(size_t index) { return m_meshes[index]; }

	inline void clear()
	{
		m_camera.reset();
		m_meshes.clear();
	}

private:
	glm::vec3 m_backgroundColor;
	std::shared_ptr<DirectionalLightsource> m_directional_lightsource;
	std::shared_ptr<PointLightsource> m_point_lightsource;
	std::shared_ptr<Camera> m_camera;
	// the corresponding materials and meshes are stored in the same order (ensured by the add method)
	std::vector<std::shared_ptr<Mesh>> m_meshes;
};