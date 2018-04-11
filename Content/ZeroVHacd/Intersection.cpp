#include "ZeroVHacdPrecompiled.hpp"

#include "Intersection.hpp"

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