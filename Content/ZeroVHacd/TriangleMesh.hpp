#pragma once

class TriangleMesh
{
public:

  void Create(Mesh* mesh);
  void Clear();

  size_t GetCount() const;
  Triangle GetTriangle(size_t index);

  // Find if this ray hits the mesh and if so where.
  // If there's multiple points of contact, keep the first one.
  bool CastRay(const Ray& ray, Real& t);
  // Find if this ray hits the mesh and if so where.
  // If there's multiple points of contact, keep the one that is closest to the target point.
  bool CastRay(const Ray& ray, Real& t, Real3& targetPoint);

  Array<Real3> mVertices;
  Array<size_t> mIndices;
  Aabb mAabb;
};