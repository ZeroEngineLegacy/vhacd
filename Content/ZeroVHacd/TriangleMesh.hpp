#pragma once

class TriangleMesh
{
public:

  void Create(Mesh* mesh);
  void Clear();

  size_t GetCount() const;
  Triangle GetTriangle(size_t index);

  Array<Real3> mVertices;
  Array<size_t> mIndices;
  Aabb mAabb;
};