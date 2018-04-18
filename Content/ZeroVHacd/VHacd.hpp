#pragma once

#include "TriangleMesh.hpp"
#include "Voxelizer.hpp"
#include "QuickHull.hpp"

class VHacd
{
public:
  VHacd();

  void Compute(const Integer3& subDivisions, int recursions, Mesh* mesh);
  void Clear();


  //--Internal
  struct SplitPlane
  {
    SplitPlane() {}
    SplitPlane(int axis, Real axisValue)
    {
      mAxis = axis;
      mAxisValue = axisValue;
    }
    Real mAxisValue;
    int mAxis;
  };

  void Initialize(Mesh* mesh);
  void ComputeApproximateConvexDecomposition();
  void Recurse(int depth);

  bool SplitVoxelizer(Voxelizer& voxelizer, Array<Voxelizer>& newVoxelizers, int depth);
  void ComputePossibleSplitPlanes(Voxelizer& voxelizer, Array<SplitPlane>& planes);
  SplitPlane FindBestSplitPlane(Voxelizer& voxelizer, Array<SplitPlane>& planes);
  float TestSplit(Voxelizer& voxelizer, int axis, Real axisValue);

  void MergeHulls();
  void BuildHullTable(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes);
  void FindHullsToMerge(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes, size_t& iX, size_t& iY);

  void Resample();
  void Resample(QuickHull& hull);

  Zero::Array<Voxelizer> mVoxelizers;

  Zero::Array<Voxelizer> mFinalVoxelizers;
  Zero::Array<QuickHull> mHulls;

  Integer3 mSubDivisions;
  int mMaxRecusionDepth;
  int mMaxHulls;
  bool mResampleMesh;
  Real mAllowedConcavityVolumeError;
  Real mAllowedVolumeSurfaceAreaRatio;

  Real mInitialConvexHullVolume;

  TriangleMesh mMesh;
};
