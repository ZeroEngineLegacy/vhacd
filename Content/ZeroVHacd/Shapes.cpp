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

//void Aabb::Compute(const std::vector<Real3>& points)
//{
//  mMin.Splat(Math::PositiveMax());
//  mMax.Splat(Math::NegativeMin());
//  for (size_t i = 0; i < points.size(); ++i)
//  {
//    const Vector3& point = points[i];
//    mMin = Math::Min(mMin, point);
//    mMax = Math::Max(mMax, point);
//  }
//}

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

//void Aabb::Transform(const Matrix4& transform)
//{
//  /******Student:Assignment2******/
//  // Compute aabb of the this aabb after it is transformed.
//  // You should use the optimize method discussed in class (not transforming all 8 points).
//  //Matrix3 rotationScale = Math::ToMatrix3(transform);
//  //for (size_t y = 0; y < 3; ++y)
//  //  for (size_t x = 0; x < 3; ++x)
//  //    rotationScale[y][x] = Math::Abs(rotationScale[y][x]);
//  //
//  //Real3 center = GetCenter();
//  //Real3 halfExtent = GetHalfSize();
//  //
//  //halfExtent = Math::Transform(rotationScale, halfExtent);
//  //center = Math::TransformPoint(transform, center);
//  //
//  //mMin = center - halfExtent;
//  //mMax = center + halfExtent;
//
//}

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

 // Real d0 = Math::Dot(searchDir, mTri.mP0);
 // Real d1 = Math::Dot(searchDir, mTri.mP1);
 // Real d2 = Math::Dot(searchDir, mTri.mP2);
 //
 // if (d0 < d1)
 // {
 //   if (d0 < d2)
 //     return mTri.mP0;
 //   else
 //     return mTri.mP2;
 // }
 // else if (d1 < d2)
 // {
 //   return mTri.mP1;
 // }
 // return mTri.mP2;
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
