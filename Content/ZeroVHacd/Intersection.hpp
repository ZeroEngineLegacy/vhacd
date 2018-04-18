#pragma once

struct Intersection
{
  static bool BarycentricCoordinates(const Real3& point, const Real3& a, const Real3& b, const Real3& c, float& u, float& v, float& w, float expansionEpsilon = 0);
  static bool RayPlane(const Ray& ray, const Real4& plane, float& t, float parallelCheckEpsilon = 0.0001f);
  static bool RayTriangle(const Ray& ray, const Triangle& tri, float& t, float triExpansionEpsilon = 0, float parallelCheckEpsilon = 0.0001f);

  static bool Test(const Aabb& aabb, const Triangle& tri);

  static bool Test(SupportShape* shapeA, SupportShape* shapeB);
  static bool TestAxes(SupportShape* shapeA, SupportShape* shapeB, const Zero::Array<Real3>& axes);
  static bool TestAxis(SupportShape* shapeA, SupportShape* shapeB, const Real3& axis);
  static Real2 GetProjectionInterval(SupportShape* shape, const Real3& axis);
};