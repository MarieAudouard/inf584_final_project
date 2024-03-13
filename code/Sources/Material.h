#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Material
{
public:
   inline virtual ~Material() {}

   inline Material(glm::vec3 albedo = glm::vec3(0.f, 0.f, 0.f), float roughness = 0.2f, float metalicness = 0.f) : m_albedo(albedo), m_roughness(roughness), m_metallicness(metalicness) {}

   inline const glm::vec3 &albedo() const { return m_albedo; }
   inline void setAlbedo(const glm::vec3 &albedo) { this->m_albedo = albedo; }

   inline float roughness() const { return m_roughness; }
   inline void setRoughness(float roughness) { this->m_roughness = roughness; }

   inline float metallicness() const { return m_metallicness; }
   inline void setMetallicness(float metallicness) { this->m_metallicness = metallicness; }

private:
   glm::vec3 m_albedo;
   float m_roughness;
   float m_metallicness;
};