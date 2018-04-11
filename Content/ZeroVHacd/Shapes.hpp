#pragma once

//-----------------------------------------------------------------------------Aabb
class Aabb
{
public:
  Aabb();
  Aabb(const Real3& min, const Real3& max);

  static Aabb BuildFromCenterAndHalfExtents(const Real3& center, const Real3& halfExtents);
  static Aabb BuildFromMinMax(const Real3& min, const Real3& max);

  // Computes the volume of this aabb.
  float GetVolume() const;
  // Computes the surface area of this aabb.
  float GetSurfaceArea() const;

  //void Compute(const std::vector<Real3>& points);

  // Does this aabb completely contain the given aabb (not an intersection test).
  bool Contains(const Aabb& aabb) const;
  // Expand the to include the given point.
  void Expand(const Real3& point);
  // Combine the two aabbs into a new one
  static Aabb Combine(const Aabb& lhs, const Aabb& rhs);
  // See if this aabb is equal to another (with epsilon). Used for unit testing.
  bool Compare(const Aabb& rhs, float epsilon) const;

  /// Compute aabb that would contain the obb of this aabb after transforming it.
  //void Transform(const Matrix4& transform);

  Real3 GetMin() const;
  Real3 GetMax() const;
  Real3 GetCenter() const;
  Real3 GetSize() const;
  Real3 GetHalfSize() const;

  Real3 mMin;
  Real3 mMax;
};

//-----------------------------------------------------------------------------Triangle
class Triangle
{
public:
  Real3 GetNormal() const
  {
    Real3 n = Math::Cross(mP1 - mP0, mP2 - mP0);
    return Math::Normalized(n);
  }

  Real3 mP0;
  Real3 mP1;
  Real3 mP2;
};

//-----------------------------------------------------------------------------SupportShape
class SupportShape
{
public:
  
  virtual Real3 Support(const Real3& searchDir) = 0;
  virtual void GetFaceNormals(Zero::Array<Real3>& normals) = 0;
  virtual void GetEdges(Zero::Array<Real3>& edges) = 0;
};

//-----------------------------------------------------------------------------AabbShape
class AabbShape : public SupportShape
{
public:
  Real3 Support(const Real3& searchDir) override;
  void GetFaceNormals(Zero::Array<Real3>& normals) override;
  void GetEdges(Zero::Array<Real3>& edges) override;

  Aabb mAabb;
};

//-----------------------------------------------------------------------------TriangleShape
class TriangleShape : public SupportShape
{
public:
  Real3 Support(const Real3& searchDir) override;
  void GetFaceNormals(Zero::Array<Real3>& normals) override;
  void GetEdges(Zero::Array<Real3>& edges) override;

  Triangle mTri;
};