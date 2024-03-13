#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"

class PointLightsource : Transform
{
public:
   inline virtual ~PointLightsource() {}

   inline PointLightsource(glm::vec3 color = glm::vec3(1.f, 1.f, 1.f), float intensity = 1.f, float ac = 0.f, float al = 0.f, float aq = 1.0f) : m_color(color),
                                                                                                                                                 m_intensity(intensity),
                                                                                                                                                 m_ac(ac),
                                                                                                                                                 m_al(al),
                                                                                                                                                 m_aq(aq) {}

   inline const glm::vec3 &position() const { return getTranslation(); }

   inline void setPosition(const glm::vec3 &position) { setTranslation(position); }

   inline const glm::vec3 &color() const { return m_color; }

   inline void setColor(const glm::vec3 &color) { this->m_color = color; }

   inline float intensity() const { return m_intensity; }

   inline void setIntensity(float intensity) { this->m_intensity = intensity; }

   inline float ac() const { return m_ac; }

   inline void ac(float ac) { this->m_ac = ac; }

   inline float al() const { return m_al; }

   inline void setAl(float al) { this->m_al = al; }

   inline float aq() const { return m_aq; }

   inline void setAq(float aq) { this->m_aq = aq; }

private:
   glm::vec3 m_color;
   float m_intensity;
   float m_ac;
   float m_al;
   float m_aq;
};

class DirectionalLightsource
{
public:
   inline virtual ~DirectionalLightsource() {}

   inline DirectionalLightsource(glm::vec3 direction = glm::vec3(1.f, 0.f, 0.f), glm::vec3 color = glm::vec3(1.f, 1.f, 1.f), float intensity = 1.f) : m_direction(normalize(direction)),
                                                                                                                                                      m_color(color),
                                                                                                                                                      m_intensity(intensity) {}

   inline const glm::vec3 &direction() const { return m_direction; }

   inline void setDirection(const glm::vec3 &direction) { this->m_direction = direction; }

   inline const glm::vec3 &color() const { return m_color; }

   inline void setColor(const glm::vec3 &color) { this->m_color = color; }

   inline float intensity() const { return m_intensity; }

   inline void setIntensity(float intensity) { this->m_intensity = intensity; }

private:
   glm::vec3 m_direction;
   glm::vec3 m_color;
   float m_intensity;
};