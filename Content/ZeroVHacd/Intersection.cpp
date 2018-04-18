#include "ZeroVHacdPrecompiled.hpp"

#include "Intersection.hpp"

bool Intersection::BarycentricCoordinates(const Real3& point, const Real3& a, const Real3& b, const Real3& c, float& u, float& v, float& w, float expansionEpsilon)
{
  Real3 v0 = point - c;
  Real3 v1 = a - c;
  Real3 v2 = b - c;

  u = v = w = 0;

  // Compute the dot products
  float dot01 = Math::Dot(v0, v1);
  float dot02 = Math::Dot(v0, v2);
  float dot12 = Math::Dot(v1, v2);
  float dot11 = Math::Dot(v1, v1);
  float dot22 = Math::Dot(v2, v2);

  // Compute barycentric coordinates
  float denom = dot11 * dot22 - dot12 * dot12;
  // check for a zero division
  if (denom == float(0.0))
    return false;

  denom = float(1.0) / denom;
  u = (dot01 * dot22 - dot02 * dot12) * denom;
  v = (dot11 * dot02 - dot12 * dot01) * denom;
  w = 1 - u - v;

  if (u >= -expansionEpsilon && v >= -expansionEpsilon && (u + v <= float(1.0) + expansionEpsilon))
    return true;

  return false;
}

bool Intersection::RayPlane(const Ray& ray, const Real4& plane, float& t, float parallelCheckEpsilon)
{
  float d = plane.w;
  Real3 normal = Real3(plane.x, plane.y, plane.z);

  float denominator = Math::Dot(normal, ray.mDirection);

  if (Math::Abs(denominator) <= parallelCheckEpsilon)
    return false;

  float numerator = d - Math::Dot(normal, ray.mStart);
  t = numerator / denominator;
  if (t < 0)
    return false;

  return true;
}

bool Intersection::RayTriangle(const Ray& ray, const Triangle& tri, float& t, float triExpansionEpsilon, float parallelCheckEpsilon)
{
  Real3 normal = -Math::Cross(tri.mP1 - tri.mP0, tri.mP2 - tri.mP0);
  normal.AttemptNormalize();

  Real4 plane(normal.x, normal.y, normal.z, Math::Dot(normal, tri.mP0));
  if (RayPlane(ray, plane, t, parallelCheckEpsilon) == false)
    return false;

  Real3 point = ray.mStart + ray.mDirection * t;

  Real3 coords;
  return BarycentricCoordinates(point, tri.mP0, tri.mP1, tri.mP2, coords.x, coords.y, coords.z, triExpansionEpsilon);
}

bool Intersection::Test(const Aabb& aabb, const Triangle& tri)
{
  AabbShape aabbShape;
  aabbShape.mAabb = aabb;
  TriangleShape triShape;
  triShape.mTri = tri;

  return Test(&aabbShape, &triShape);
}

bool Intersection::Test(SupportShape* shapeA, SupportShape* shapeB)
{
  Zero::Array<Real3> normalsA;
  shapeA->GetFaceNormals(normalsA);
  bool normalsAResult = TestAxes(shapeA, shapeB, normalsA);
  if (normalsAResult == false)
    return false;

  Zero::Array<Real3> normalsB;
  shapeB->GetFaceNormals(normalsB);
  bool normalsBResult = TestAxes(shapeA, shapeB, normalsB);
  if (normalsBResult == false)
    return false;

  Zero::Array<Real3> edgesA;
  Zero::Array<Real3> edgesB;
  shapeA->GetEdges(edgesA);
  shapeB->GetEdges(edgesB);

  for(size_t a = 0; a < edgesA.Size(); ++a)
  {
    Real3 edgeA = edgesA[a];
    for (size_t b = 0; b < edgesB.Size(); ++b)
    {
      Real3 edgeB = edgesB[b];

      Real3 axis = Math::Cross(edgeA, edgeB);
      if (Math::Length(axis) < 0.001f)
        continue;

      bool axisResult = TestAxis(shapeA, shapeB, Math::Normalized(axis));

      if (axisResult == false)
        return false;
    }
  }

  return true;
}

bool Intersection::TestAxes(SupportShape* shapeA, SupportShape* shapeB, const Zero::Array<Real3>& axes)
{
  for(size_t i = 0; i < axes.Size(); ++i)
  {
    Real3 axis = axes[i];
    bool axisResult = TestAxis(shapeA, shapeB, Math::Normalized(axis));

    if (axisResult == false)
      return false;
  }
  return true;
}

bool Intersection::TestAxis(SupportShape* shapeA, SupportShape* shapeB, const Real3& axis)
{
  Real2 projA = GetProjectionInterval(shapeA, axis);
  Real2 projB = GetProjectionInterval(shapeB, axis);

  if (projA.x > projB.y || projB.x > projA.y)
    return false;
  return true;
}

Real2 Intersection::GetProjectionInterval(SupportShape* shape, const Real3& axis)
{
  Real3 min = shape->Support(-axis);
  Real3 max = shape->Support(axis);

  Real2 result = Real2(Math::Dot(min, axis), Math::Dot(max, axis));
  return result;
}