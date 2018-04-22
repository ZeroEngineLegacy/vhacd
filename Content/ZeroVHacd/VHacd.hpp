#pragma once

#include "TriangleMesh.hpp"
#include "Voxelizer.hpp"
#include "QuickHull.hpp"

class VHacd
{
public:
  VHacd();

  typedef void(*CallbackFn)(const String& message, float percentage, void* clientData);
  void SetProgressCallback(CallbackFn callbackFn, void* clientData);

  void Compute(Real fidelity, int recursions, TriangleMesh& mesh);
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

  void ComputeSubDivisions(Aabb& aabb);
  void Initialize(TriangleMesh& mesh);
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

  void UpdateProgress(const String& message, float percentage);

  Zero::Array<Voxelizer> mVoxelizers;

  Zero::Array<Voxelizer> mFinalVoxelizers;
  Zero::Array<QuickHull> mHulls;

  Real mFidelity;
  Integer3 mSubDivisions;
  int mMaxRecusionDepth;
  int mMaxHulls;
  bool mResampleMesh;
  Real mAllowedConcavityVolumeError;
  Real mAllowedVolumeSurfaceAreaRatio;

  Real mInitialConvexHullVolume;
  Real mBalanceWeight;
  Real mSymmetryWeight;

  TriangleMesh mMesh;
  CallbackFn mCallbackFn;
  void* mClientData;
  float mProgress;
  bool mForceStop;
};
