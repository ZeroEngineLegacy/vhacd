#include "ZeroVHacdPrecompiled.hpp"

void TriangleMesh::Create(Mesh* mesh)
{
  Clear();

  Zilch::HandleOf<ZeroEngine::VertexBuffer> verticesHandle = mesh->GetVertices();
  ZeroEngine::VertexBuffer* VertexBuffer = verticesHandle;
  int vertexCount = VertexBuffer->GetVertexCount();

  for (int i = 0; i < vertexCount; ++i)
  {
    Real4 pos = VertexBuffer->GetVertexData(i, ZeroEngine::VertexSemantic::GetPosition());
    Real3 point = Real3(pos.x, pos.y, pos.z);
    mVertices.PushBack(point);

    mAabb.Expand(point);
  }

  Zilch::HandleOf<ZeroEngine::IndexBuffer> indicesHandle = mesh->GetIndices();
  ZeroEngine::IndexBuffer* indexBuffer = indicesHandle;
  int indexCount = indexBuffer->GetCount();
  for (int i = 0; i < indexCount; ++i)
  {
    int index = indexBuffer->Get(i);
    mIndices.PushBack(index);
  }
}

void TriangleMesh::Clear()
{
  mVertices.Clear();
  mIndices.Clear();
  mAabb = Aabb();
}

size_t TriangleMesh::GetCount() const
{
  return  mIndices.Size() / 3;
}

Triangle TriangleMesh::GetTriangle(size_t index)
{
  size_t i = index * 3;
  Triangle tri;
  tri.mP0 = mVertices[mIndices[i + 0]];
  tri.mP1 = mVertices[mIndices[i + 1]];
  tri.mP2 = mVertices[mIndices[i + 2]];
  return tri;
}

bool TriangleMesh::CastRay(const Ray& ray, Real& t)
{
  t = Math::PositiveMax();
  bool hit = false;

  for (size_t i = 0; i < GetCount(); ++i)
  {
    Triangle tri = GetTriangle(i);
    Real triT;
    if (Intersection::RayTriangle(ray, tri, triT))
    {
      hit = true;
      t = Math::Min(triT, t);
    }
  }
  return hit;
}

bool TriangleMesh::CastRay(const Ray& ray, Real& t, Real3& targetPoint)
{
  Real minDistance = Math::PositiveMax();
  t = Math::PositiveMax();
  bool hit = false;

  for (size_t i = 0; i < GetCount(); ++i)
  {
    Triangle tri = GetTriangle(i);
    Real triT;
    if (Intersection::RayTriangle(ray, tri, triT))
    {
      Real3 point = ray.mStart + ray.mDirection * triT;
      Real distance = Math::Distance(point, targetPoint);
      if (distance < minDistance)
      {
        minDistance = distance;
        t = triT;
        hit = true;
      }
    }
  }
  return hit;
}
