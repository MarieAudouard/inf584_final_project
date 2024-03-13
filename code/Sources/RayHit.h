#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class RayHit
{
public:
   inline RayHit(bool m_is_hit = false, const float b0 = 0.f, const float b1 = 0.f, const float b2 = 0.f, float t = 0.f, size_t mesh_idx = 0, size_t triangle_idx = 0) : b0(b0), b1(b1), b2(b2), m_t(t), mesh_idx(mesh_idx), triangle_idx(triangle_idx) {}
   inline virtual ~RayHit() {}

   inline const bool isHit() const { return m_is_hit; }
   inline void setIsHit(bool is_hit) { m_is_hit = is_hit; }

   inline const float getB0() const { return b0; }
   inline void setB0(const float b0) { this->b0 = b0; }

   inline const float getB1() const { return b1; }
   inline void setB1(const float b1) { this->b1 = b1; }

   inline const float getB2() const { return b2; }
   inline void setB2(const float b2) { this->b2 = b2; }

   inline float t() const { return m_t; }
   inline void setT(float t) { m_t = t; }

   inline const size_t getMeshIdx() const { return mesh_idx; }
   inline void setMeshIdx(const size_t mesh_idx) { this->mesh_idx = mesh_idx; }

   inline const size_t getTriangleIdx() const { return triangle_idx; }
   inline void setTriangleIdx(const size_t triangle_idx) { this->triangle_idx = triangle_idx; }

private:
   bool m_is_hit;
   float b0;
   float b1;
   float b2;
   float m_t;
   size_t mesh_idx;
   size_t triangle_idx;
};