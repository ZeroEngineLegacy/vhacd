#pragma once

struct Intersection
{
  static bool Test(const Aabb& aabb, const Triangle& tri);

  static bool Test(SupportShape* shapeA, SupportShape* shapeB);
  static bool TestAxes(SupportShape* shapeA, SupportShape* shapeB, const Zero::Array<Real3>& axes);
  static bool TestAxis(SupportShape* shapeA, SupportShape* shapeB, const Real3& axis);
  static Real2 GetProjectionInterval(SupportShape* shape, const Real3& axis);
};