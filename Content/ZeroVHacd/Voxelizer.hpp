#pragma once

namespace VoxelState
{
  enum Enum
  {
    Unknown, Inside, Outside, Surface
  };
}

class Voxelizer
{
public:

  void CreateVoxels(Integer3 subDivision, const Aabb& aabb);
  void CreateVoxels(Integer3 subDivision, const Aabb& aabb, VoxelState::Enum defaultValue);
  void WriteTriangle(const Triangle& tri);

  Real ComputeVolume();
  Real GetSurfaceVolume();

  Integer3 Discretize(const Real3& point);
  size_t VoxelCoordToIndex(const Integer3& voxelCoord);
  Aabb GetVoxelAabb(const Integer3& voxelCoord);

  VoxelState::Enum GetVoxel(const Integer3& voxelCoord);
  void SetVoxel(const Integer3& voxelCoord, VoxelState::Enum state);

  void FillOutside();
  void FillOutside(Zero::Array<Integer3>& queue, Zero::HashSet<Integer3>& visitedSet);
  void AddFillLocation(Zero::Array<Integer3>& queue, const Integer3& voxelCoord);
  void Fill();

  bool Split(int axisIndex, Real axisValue, Voxelizer& front, Voxelizer& back);
  bool Split(int axisIndex, int frontSplitIndexInclusive, Voxelizer& front, Voxelizer& back);
  bool TestSplit(int axisIndex, int splitIndex, Voxelizer& front, Voxelizer& back);
  bool GetSplitTest(int axisIndex, int axisStartInclusive, int axisEndExclusive, Array<Real3>& surfaceVoxelCenters, Real& volume);

  void AabbCentersToPoints(Array<Vector3>& voxelCenters, Array<Real3>& points);
  void AabbCenterToPoints(Vector3& voxelCenter, Array<Real3>& points);

  void ComputeEigenValuesAndVectors(Real3& eigenValues, Matrix3& eigenVectors);
  void ComputeSymmetryComponents();

  Integer3 mSubDivisions;
  Real3 mVoxelSize;
  Aabb mAabb;
  Zero::Array<VoxelState::Enum> mVoxels;

  Real3 mEigenValues;
  Matrix3 mEigenVectors;
  Real3 mRevolutionAxis;
  Real mRevolutionWeight;
};
