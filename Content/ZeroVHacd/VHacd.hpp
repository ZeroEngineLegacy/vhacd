#pragma once
#include "Voxelizer.hpp"
#include "QuickHull.hpp"

class VHacd
{
public:
  void Compute(const Integer3& subDivisions, int recursions, Mesh* mesh);
  void Clear();

  void Recurse(int depth);
  void MergeHulls();
  void Build(Zilch::Array<QuickHull>& combinedHulls, Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes);
  void FindHullsToMerge(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes, size_t& iX, size_t& iY);

  Zero::Array<Voxelizer> mVoxelizers;

  Zero::Array<Voxelizer> mFinalVoxelizers;
  Zero::Array<QuickHull> mHulls;

  Integer3 mSubDivisions;
  int mRecursions;
  int mMaxHulls;
};
