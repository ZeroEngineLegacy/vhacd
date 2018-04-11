#pragma once

#include "TriangleMesh.hpp"
#include "Voxelizer.hpp"
#include "QuickHull.hpp"

class VHacd
{
public:
  void Compute(const Integer3& subDivisions, int recursions, Mesh* mesh);
  void Clear();


  //--Internal

  void Initialize(Mesh* mesh);
  void ComputeApproximateConvexDecomposition();
  void Recurse(int depth);

  bool SplitVoxelizer(Voxelizer& voxelizer, Array<Voxelizer>& newVoxelizers, int depth);
  float TestSplit(Voxelizer& voxelizer, int axis, Real axisValue);

  void MergeHulls();
  void BuildHullTable(Zilch::Array<QuickHull>& combinedHulls, Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes);
  void FindHullsToMerge(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes, size_t& iX, size_t& iY);

  Zero::Array<Voxelizer> mVoxelizers;

  Zero::Array<Voxelizer> mFinalVoxelizers;
  Zero::Array<QuickHull> mHulls;

  Integer3 mSubDivisions;
  int mRecursions;
  int mMaxHulls;
  Real mConcavity;

  Real mInitialConvexHullVolume;

  TriangleMesh mMesh;
};
