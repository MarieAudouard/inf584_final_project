// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Camera.h"

Ray Camera::rayAt(float x, float y, float height, float width)
{
   glm::vec3 origin = glm::vec3(glm::inverse(computeViewMatrix()) * glm::vec4(0, 0, 0, 1));
   float px = ((2 * x / width) - 1.f) * tan(glm::radians(m_fov) / 2.f) * m_aspectRatio;
   float py = ((2 * y / height) - 1.f) * tan(glm::radians(m_fov) / 2.f);
   glm::vec4 in_world_coords = glm::inverse(computeViewMatrix()) * glm::vec4(px, py, -1, 1);
   glm::vec3 direction = glm::normalize(glm::vec3(in_world_coords) - origin);
   return Ray(origin, direction);
}