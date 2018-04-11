#include "ZeroVHacdPrecompiled.hpp"

#include "Shapes.hpp"
#include "Zilch.hpp"


//-----------------------------------------------------------------------------Aabb
Aabb::Aabb()
{
  //set the aabb to an initial bad value (where the min is smaller than the max)
  mMin.Splat(Math::PositiveMax());
  mMax = -mMin;
}

Aabb::Aabb(const Real3& min, const Real3& max)
{
  mMin = min;
  mMax = max;
}

Aabb Aabb::BuildFromCenterAndHalfExtents(const Real3& center, const Real3& halfExtents)
{
  return Aabb(center - halfExtents, center + halfExtents);
}

Aabb Aabb::BuildFromMinMax(const Real3& min, const Real3& max)
{
  return Aabb(min, max);
}

float Aabb::GetVolume() const
{
  // Return the aabb's volume
  Real3 size = mMax - mMin;
  return size.x * size.y * size.z;
}

float Aabb::GetSurfaceArea() const
{
  // Return the aabb's surface area
  Real3 size = mMax - mMin;

  float surfaceArea = 0;
  surfaceArea += size.x * size.y * 2;
  surfaceArea += size.x * size.z * 2;
  surfaceArea += size.y * size.z * 2;
  return surfaceArea;
}

bool Aabb::Contains(const Aabb& aabb) const
{
  // If a min is less than our min or a max is greater than our
  // max on any axis then we don't contain the aabb
  for (size_t i = 0; i < 3; ++i)
  {
    if (aabb.mMin[i] < mMin[i])
      return false;
    if (aabb.mMax[i] > mMax[i])
      return false;
  }

  return true;
}

void Aabb::Expand(const Real3& point)
{
  for (size_t i = 0; i < 3; ++i)
  {
    mMin[i] = Math::Min(mMin[i], point[i]);
    mMax[i] = Math::Max(mMax[i], point[i]);
  }
}

Aabb Aabb::Combine(const Aabb& lhs, const Aabb& rhs)
{
  Aabb result;
  for (size_t i = 0; i < 3; ++i)
  {
    result.mMin[i] = Math::Min(lhs.mMin[i], rhs.mMin[i]);
    result.mMax[i] = Math::Max(lhs.mMax[i], rhs.mMax[i]);
  }
  return result;
}

bool Aabb::Compare(const Aabb& rhs, float epsilon) const
{
  float pos1Diff = Math::Length(mMin - rhs.mMin);
  float pos2Diff = Math::Length(mMax - rhs.mMax);

  return pos1Diff < epsilon && pos2Diff < epsilon;
}

Real3 Aabb::GetMin() const
{
  return mMin;
}

Real3 Aabb::GetMax() const
{
  return mMax;
}

Real3 Aabb::GetCenter() const
{
  return (mMin + mMax) * 0.5f;
}

Real3 Aabb::GetSize() const
{
  return mMax - mMin;
}

Real3 Aabb::GetHalfSize() const
{
  return (mMax - mMin) * 0.5f;
}

//-----------------------------------------------------------------------------AabbShape
Real3 AabbShape::Support(const Real3& searchDir)
{
  Real3 c = mAabb.GetCenter();
  Real3 e = mAabb.GetHalfSize();

  Real3 point = Real3::cZero;
  for (size_t i = 0; i < 3; ++i)
  {
    point[i] = c[i] + e[i] * Math::Sign(searchDir[i]);
  }

  return point;
}

void AabbShape::GetFaceNormals(Zero::Array<Real3>& normals)
{
  normals.PushBack(Real3::cXAxis);
  normals.PushBack(Real3::cYAxis);
  normals.PushBack(Real3::cZAxis);
}

void AabbShape::GetEdges(Zero::Array<Real3>& edges)
{
  edges.PushBack(Real3::cXAxis);
  edges.PushBack(Real3::cYAxis);
  edges.PushBack(Real3::cZAxis);
}

//-----------------------------------------------------------------------------TriangleShape
Real3 TriangleShape::Support(const Real3& searchDir)
{
  Real maxDistance = -Math::PositiveMax();

  Real3 points[3] = { mTri.mP0, mTri.mP1, mTri.mP2 };
  int bestIndex = 0;
  for(int i = 0; i < 3; ++i)
  {
    Real dist = Math::Dot(searchDir, points[i]);
    if(dist > maxDistance)
    {
      maxDistance = dist;
      bestIndex = i;
    }
  }

  return points[bestIndex];
}

void TriangleShape::GetFaceNormals(Zero::Array<Real3>& normals)
{
  normals.PushBack(mTri.GetNormal());
}

void TriangleShape::GetEdges(Zero::Array<Real3>& edges)
{
  Real3 edge01 = mTri.mP1 - mTri.mP0;
  Real3 edge12 = mTri.mP2 - mTri.mP1;
  Real3 edge20 = mTri.mP0 - mTri.mP2;
  edges.PushBack(edge01);
  edges.PushBack(edge12);
  edges.PushBack(edge20);
}
