#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Ray
{
public:
   inline Ray(const glm::vec3 &origin, const glm::vec3 &direction) : m_origin(origin), m_direction(direction) {}
   inline virtual ~Ray() {}

   inline const glm::vec3 &origin() const { return m_origin; }
   inline void setOrigin(const glm::vec3 &origin) { m_origin = origin; }

   inline const glm::vec3 &direction() const { return m_direction; }
   inline void setDirection(const glm::vec3 &direction) { m_direction = direction; }

private:
   glm::vec3 m_origin;
   glm::vec3 m_direction;
};