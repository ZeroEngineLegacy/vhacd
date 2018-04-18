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

Matrix3 ComputeCovarianceMatrix(const Array<Vector3>& points)
{
  size_t pointCount = points.Size();
  float invPointCount = 1.0f / pointCount;

  // Compute the centroid of the points
  Vector3 centroid = Vector3::cZero;
  for (size_t i = 0; i < pointCount; ++i)
    centroid += points[i];
  centroid *= invPointCount;

  // Compute covariance elements
  float e00, e11, e22, e01, e02, e12;
  e00 = e11 = e22 = e01 = e02 = e12 = 0.0f;
  for (size_t i = 0; i < pointCount; ++i)
  {
    // Translate points so center of mass is at origin
    Vector3 p = points[i] - centroid;
    // Compute covariance of translated points
    e00 += p.x * p.x;
    e11 += p.y * p.y;
    e22 += p.z * p.z;
    e01 += p.x * p.y;
    e02 += p.x * p.z;
    e12 += p.y * p.z;
  }

  // Fill in the covariance matrix elements
  Matrix3 covarianceMat;
  covarianceMat[0][0] = e00 * invPointCount;
  covarianceMat[1][1] = e11 * invPointCount;
  covarianceMat[2][2] = e22 * invPointCount;
  covarianceMat[0][1] = covarianceMat[1][0] = e01 * invPointCount;
  covarianceMat[0][2] = covarianceMat[2][0] = e02 * invPointCount;
  covarianceMat[1][2] = covarianceMat[2][1] = e12 * invPointCount;
  return covarianceMat;
}

Matrix3 ComputeJacobiRotation(const Matrix3& matrix)
{
  int p = 0;
  int q = 1;
  for (size_t i = 0; i < 3; ++i)
  {
    for (size_t j = 0; j < 3; ++j)
    {
      if (i == j)
        continue;
      if (Math::Abs(matrix[i][j]) > Math::Abs(matrix[p][q]))
      {
        p = i;
        q = j;
      }
    }
  }

  // Compute cosine and sine that will rotate the given matrix's [p][q] item to 0
  float c, s;
  if (Math::Abs(matrix[p][q]) > 0.0001f)
  {
    float r = (matrix[q][q] - matrix[p][p]) / (2.0f * matrix[p][q]);
    float tangent;
    if (r >= 0.0f)
      tangent = 1.0f / (r + Math::Sqrt(1.0f + r * r));
    else
      tangent = -1.0f / (-r + Math::Sqrt(1.0f + r * r));
    c = 1.0f / Math::Sqrt(1.0f + tangent * tangent);
    s = tangent * c;
  }
  else
  {
    c = 1.0f;
    s = 0.0f;
  }

  // Construct the jacobi rotation matrix with the compute cosine and sine values (in the right spots of course)
  Matrix3 rotation = Matrix3::cIdentity;
  rotation[p][p] = c;
  rotation[p][q] = s;
  rotation[q][p] = -s;
  rotation[q][q] = c;
  return rotation;
}

void ComputeEigenValuesAndVectors(const Matrix3& covariance, Vector3& eigenValues, Matrix3& eigenVectors, int maxIterations, float diagonalSqEpsilon)
{
  float prevoff = 0.0f;
  Matrix3 matrix = covariance;

  // Intializes the eigenvector matrix to identity
  eigenVectors.SetIdentity();

  // Repeat for some max number of iterations
  for (int n = 0; n < maxIterations; ++n)
  {
    Matrix3 jacobiRotation = ComputeJacobiRotation(matrix);

    // Cumulate rotations into what will contain the eigenvectors
    eigenVectors = eigenVectors * jacobiRotation;

    // Make 'a' more diagonal, until just eigenvalues remain on diagonal
    matrix = (jacobiRotation.Transposed() * matrix) * jacobiRotation;

    // Compute "norm" of off-diagonal elements
    float off = 0.0f;
    for (size_t i = 0; i < 3; ++i)
    {
      for (size_t j = 0; j < 3; ++j)
      {
        if (i == j)
          continue;
        off += matrix[i][j] * matrix[i][j];
      }
    }
    /* off = sqrt(off); not needed for norm comparison */
    if (off < diagonalSqEpsilon)
      break;

    // More propery method but harder to explain so leaving this as a reference
    // Stop when norm no longer decreasing
    //if(n > 2 && off >= prevoff)
    //  break;

    prevoff = off;
  }

  eigenValues[0] = matrix[0][0];
  eigenValues[1] = matrix[1][1];
  eigenValues[2] = matrix[2][2];
}
