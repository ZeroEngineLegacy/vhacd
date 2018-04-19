#include "ZeroVHacdPrecompiled.hpp"

#include "Voxelizer.hpp"
#include "Intersection.hpp"

void Voxelizer::CreateVoxels(Integer3 subDivision, const Aabb& aabb)
{
  mAabb = aabb;
  mSubDivisions = subDivision;
  mVoxels.Clear();
  mVoxels.Resize(mSubDivisions.x * mSubDivisions.y * mSubDivisions.z, VoxelState::Unknown);

  Real3 size = aabb.GetHalfSize() * 2;
  mVoxelSize = size / Real3((Real)mSubDivisions.x, (Real)mSubDivisions.y, (Real)mSubDivisions.z);
}

void Voxelizer::WriteTriangle(const Triangle& tri)
{
  Integer3 indexA = Discretize(tri.mP0);
  Integer3 indexB = Discretize(tri.mP1);
  Integer3 indexC = Discretize(tri.mP2);

  Integer3 start = Math::Min(Math::Min(indexA, indexB), indexC);
  Integer3 end = Math::Max(Math::Max(indexA, indexB), indexC);

  for(int z = start.z; z <= end.z; ++z)
  {
    for(int y = start.y; y <= end.y; ++y)
    {
      for(int x = start.x; x <= end.x; ++x)
      {
        Integer3 voxelCoord = Integer3(x, y, z);
        Aabb voxelAabb = GetVoxelAabb(voxelCoord);


        bool intersecting = Intersection::Test(voxelAabb, tri);

        if(!intersecting)
          continue;

        size_t voxelIndex = VoxelCoordToIndex(voxelCoord);
        SetVoxel(voxelCoord, VoxelState::Surface);
      }
    }
  }
}

Real Voxelizer::ComputeVolume()
{
  int count = 0;
  for (size_t i = 0; i < mVoxels.Size(); ++i)
  {
    VoxelState::Enum state = mVoxels[i];
    if (state == VoxelState::Inside || state == VoxelState::Surface)
      ++count;
  }

  Real voxelVolume = mVoxelSize.x * mVoxelSize.y * mVoxelSize.z;
  voxelVolume *= count;
  return voxelVolume;
}

Real Voxelizer::GetSurfaceVolume()
{
  int count = 0;
  for (size_t i = 0; i < mVoxels.Size(); ++i)
  {
    VoxelState::Enum state = mVoxels[i];
    if (state == VoxelState::Surface)
      ++count;
  }

  Real voxelVolume = mVoxelSize.x * mVoxelSize.y * mVoxelSize.z;
  voxelVolume *= count;
  return voxelVolume;
}

Integer3 Voxelizer::Discretize(const Real3& point)
{
  Real3 extents = mAabb.mMax - mAabb.mMin;
  Real3 subDivisions((Real)mSubDivisions.x, (Real)mSubDivisions.y, (Real)mSubDivisions.z);
  Real3 t = (point - mAabb.mMin) / (extents / subDivisions);
  t = Math::Floor(t);
  Integer3 coord = Integer3((int)t.x, (int)t.y, (int)t.z);
  return coord;
}

size_t Voxelizer::VoxelCoordToIndex(const Integer3& voxelCoord)
{
  int index = voxelCoord.x;
  index += voxelCoord.y * mSubDivisions.x;
  index += voxelCoord.z * (mSubDivisions.x * mSubDivisions.y);
  return index;
}

Aabb Voxelizer::GetVoxelAabb(const Integer3& voxelCoord)
{
  Real3 voxelSize = mVoxelSize;

  Real3 halfOffset = Real3(0.5) * voxelSize;
  Real3 start = mAabb.mMin + halfOffset;

  Real3 realCoord = Real3((Real)voxelCoord.x, (Real)voxelCoord.y, (Real)voxelCoord.z);
  Real3 center = start + realCoord * voxelSize;
  return Aabb::BuildFromCenterAndHalfExtents(center, voxelSize * 0.5f);
}

VoxelState::Enum Voxelizer::GetVoxel(const Integer3& voxelCoord)
{
  size_t index = VoxelCoordToIndex(voxelCoord);
  return mVoxels[index];
}

void Voxelizer::SetVoxel(const Integer3& voxelCoord, VoxelState::Enum state)
{
  size_t index = VoxelCoordToIndex(voxelCoord);
  mVoxels[index] = state;
}

void Voxelizer::FillOutside()
{
  Zero::HashSet<Integer3> visitedSet;
  Zero::Array<Integer3> queue;
  queue.PushBack(Integer3(0, 0, 0));

  FillOutside(queue, visitedSet);
}

void Voxelizer::FillOutside(Zero::Array<Integer3>& queue, Zero::HashSet<Integer3>& visitedSet)
{
  while (!queue.Empty())
  {
    Integer3 voxelCoord = queue.Back();
    queue.PopBack();

    if (GetVoxel(voxelCoord) != VoxelState::Unknown)
      continue;

    SetVoxel(voxelCoord, VoxelState::Outside);
    AddFillLocation(queue, voxelCoord + Integer3(-1, 0, 0));
    AddFillLocation(queue, voxelCoord + Integer3(0, -1, 0));
    AddFillLocation(queue, voxelCoord + Integer3(0, 0, -1));
    AddFillLocation(queue, voxelCoord + Integer3(1, 0, 0));
    AddFillLocation(queue, voxelCoord + Integer3(0, 1, 0));
    AddFillLocation(queue, voxelCoord + Integer3(0, 0, 1));
  }
}

void Voxelizer::AddFillLocation(Zero::Array<Integer3>& queue, const Integer3& voxelCoord)
{
  for (size_t i = 0; i < 3; ++i)
  {
    if (voxelCoord[i] < 0 || mSubDivisions[i] <= voxelCoord[i])
      return;
  }
  queue.PushBack(voxelCoord);
}

void Voxelizer::Fill()
{
  for(int z = 0; z < mSubDivisions.z; ++z)
  {
    for(int y = 0; y < mSubDivisions.y; ++y)
    {
      for(int x = 0; x < mSubDivisions.x; ++x)
      {
        Integer3 voxelCoord = Integer3(x, y, z);
        size_t voxelIndex = VoxelCoordToIndex(voxelCoord);
        if (mVoxels[voxelIndex] == VoxelState::Unknown)
          mVoxels[voxelIndex] = VoxelState::Inside;
      }
    }
  }
}

bool Voxelizer::Split(int axisIndex, Real axisValue, Voxelizer& front, Voxelizer& back)
{
  Integer3 index = Discretize(Real3(axisValue));
  int splitIndex = index[axisIndex];

  int axis1 = (axisIndex + 1) % 3;
  int axis2 = (axisIndex + 2) % 3;
  Integer3 subDivisions = Integer3(mSubDivisions[axisIndex], mSubDivisions[axis1], mSubDivisions[axis2]);
  int crossArea = subDivisions.y * subDivisions.z;

  int frontCount = splitIndex + 1;
  int backCount = subDivisions.x - frontCount;
  int voxelsFront = frontCount * crossArea;
  int voxelsBack = backCount * crossArea;

  if (frontCount == 0 || backCount == 0)
    return false;


  front.mSubDivisions[axisIndex] = frontCount;
  front.mSubDivisions[axis1] = mSubDivisions[axis1];
  front.mSubDivisions[axis2] = mSubDivisions[axis2];
  front.mVoxelSize = mVoxelSize;
  back.mSubDivisions[axisIndex] = backCount;
  back.mSubDivisions[axis1] = mSubDivisions[axis1];
  back.mSubDivisions[axis2] = mSubDivisions[axis2];
  back.mVoxelSize = mVoxelSize;

  Real3 frontMin = mAabb.mMin;
  Real3 frontMax = mAabb.mMax;
  frontMax[axisIndex] = frontMin[axisIndex] + frontCount * mVoxelSize[axisIndex];
  front.CreateVoxels(front.mSubDivisions, Aabb(frontMin, frontMax));

  Real3 backMin = mAabb.mMin;
  Real3 backMax = mAabb.mMax;
  backMin[axisIndex] = backMax[axisIndex] - backCount * mVoxelSize[axisIndex];
  back.CreateVoxels(back.mSubDivisions, Aabb(backMin, backMax));

  for (int z = 0; z < front.mSubDivisions.z; ++z)
  {
    for (int y = 0; y < front.mSubDivisions.y; ++y)
    {
      for (int x = 0; x < front.mSubDivisions.x; ++x)
      {
        Integer3 voxelCoord = Integer3(x, y, z);

        front.SetVoxel(voxelCoord, GetVoxel(voxelCoord));
      }
    }
  }
  for (int z = 0; z < back.mSubDivisions.z; ++z)
  {
    for (int y = 0; y < back.mSubDivisions.y; ++y)
    {
      for (int x = 0; x < back.mSubDivisions.x; ++x)
      {
        Integer3 voxelCoord = Integer3(x, y, z);
        Integer3 thisVoxelCoord = voxelCoord;
        thisVoxelCoord[axisIndex] += frontCount;
        back.SetVoxel(voxelCoord, GetVoxel(thisVoxelCoord));
      }
    }
  }

  for (int v1 = 0; v1 < mSubDivisions[axis1]; ++v1)
  {
    for (int v2 = 0; v2 < mSubDivisions[axis2]; ++v2)
    {
      Integer3 frontCoord = Integer3(0, 0, 0);
      frontCoord[axis1] = v1;
      frontCoord[axis2] = v2;
      Integer3 backCoord = frontCoord;
      frontCoord[axisIndex] = splitIndex;
      backCoord[axisIndex] = 0;

      if (front.GetVoxel(frontCoord) == VoxelState::Inside)
        front.SetVoxel(frontCoord, VoxelState::Surface);
      if (back.GetVoxel(backCoord) == VoxelState::Inside)
        back.SetVoxel(backCoord, VoxelState::Surface);
    }
  }
  return true;
}

void Voxelizer::ComputeEigenValuesAndVectors(Real3& eigenValues, Matrix3& eigenVectors)
{
  Array<Real3> points;
  for (int z = 0; z < mSubDivisions.z; ++z)
  {
    for (int y = 0; y < mSubDivisions.y; ++y)
    {
      for (int x = 0; x < mSubDivisions.x; ++x)
      {
        Integer3 voxelCoord = Integer3(x, y, z);
        VoxelState::Enum voxelState = GetVoxel(voxelCoord);
        if(voxelState == VoxelState::Inside || voxelState == VoxelState::Surface)
        {
          Aabb voxelAabb = GetVoxelAabb(voxelCoord);
          points.PushBack(voxelAabb.GetCenter());
        }
      }
    }
  }

  Matrix3 covarianceMatrix = ::ComputeCovarianceMatrix(points);
  ::ComputeEigenValuesAndVectors(covarianceMatrix, eigenValues, eigenVectors, 100, 0.0001f);
}

void Voxelizer::ComputeSymmetryComponents()
{
  ComputeEigenValuesAndVectors(mEigenValues, mEigenVectors);

  Real xy = Math::Abs(mEigenValues.x - mEigenValues.y);
  Real xz = Math::Abs(mEigenValues.x - mEigenValues.z);
  Real yz = Math::Abs(mEigenValues.y - mEigenValues.z);
  
  Real numer;
  Real denom;
  size_t primaryAxis = 0;

  if (yz < xy && yz < xz)
  {
    primaryAxis = 0;
  }
  else if (xz < xy && xz < yz)
  {
    primaryAxis = 1;
  }
  else
  {
    primaryAxis = 2;
  }

  mRevolutionAxis = Real3(0, 0, 0);
  mRevolutionAxis[primaryAxis] = 1;
  size_t axis1 = (primaryAxis + 1) % 3;
  size_t axis2 = (primaryAxis + 2) % 3;
  numer = Math::Sq(mEigenValues[axis1] - mEigenValues[axis2]);
  denom = Math::Sq(Math::Abs(mEigenValues[axis1]) + Math::Abs(mEigenValues[axis2]));
  mRevolutionWeight = 1 - numer / denom;
}